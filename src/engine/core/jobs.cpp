#include "engine/core/jobs.h"

#include <algorithm>
#include <exception>
#include <memory>
#include <utility>

namespace racer::engine {

unsigned int JobSystem::DefaultWorkerCount() {
    const unsigned int hardware = std::thread::hardware_concurrency();
    return hardware > 1u ? hardware - 1u : 1u;
}

JobSystem::JobSystem(unsigned int workerCount) {
    if (workerCount == 0u) {
        workerCount = 1u;
    }
    workers_.reserve(workerCount);
    for (unsigned int i = 0; i < workerCount; ++i) {
        workers_.emplace_back([this] { WorkerLoop(); });
    }
}

JobSystem::~JobSystem() {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        stopping_ = true;
    }
    queueCv_.notify_all();
    for (std::thread& worker : workers_) {
        worker.join();
    }
}

void JobSystem::Enqueue(Task task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        queue_.push_back(std::move(task));
    }
    queueCv_.notify_one();
}

bool JobSystem::TryPop(Task& task) {
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (queue_.empty()) {
        return false;
    }
    task = std::move(queue_.front());
    queue_.pop_front();
    return true;
}

void JobSystem::WorkerLoop() {
    for (;;) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCv_.wait(lock, [this] { return stopping_ || !queue_.empty(); });
            if (queue_.empty()) {
                return; // stopping_ et file vide : arret propre
            }
            task = std::move(queue_.front());
            queue_.pop_front();
        }
        // Les exceptions sont capturees en amont (packaged_task / tranche
        // ParallelFor), task() ne doit donc jamais lever.
        task();
    }
}

std::future<void> JobSystem::Submit(std::function<void()> job) {
    // shared_ptr car std::function exige un foncteur copiable alors que
    // packaged_task est move-only.
    auto task = std::make_shared<std::packaged_task<void()>>(std::move(job));
    std::future<void> future = task->get_future();
    Enqueue([task] { (*task)(); });
    return future;
}

void JobSystem::ParallelFor(std::size_t begin, std::size_t end, std::size_t grainSize,
                            const std::function<void(std::size_t)>& fn) {
    if (begin >= end) {
        return;
    }
    if (grainSize == 0) {
        grainSize = 1;
    }

    const std::size_t count = end - begin;
    const std::size_t chunkCount = (count + grainSize - 1) / grainSize;

    // Etat partage de la rafale, protege par doneMutex. Le notify_all est
    // emis sous le verrou : aucun worker ne touche cet etat apres que le
    // thread appelant a observe remaining == 0.
    std::size_t remaining = chunkCount;
    std::exception_ptr firstError;
    std::mutex doneMutex;
    std::condition_variable doneCv;

    for (std::size_t c = 0; c < chunkCount; ++c) {
        const std::size_t chunkBegin = begin + c * grainSize;
        const std::size_t chunkEnd = std::min(end, chunkBegin + grainSize);
        Enqueue([&, chunkBegin, chunkEnd] {
            std::exception_ptr error;
            try {
                for (std::size_t i = chunkBegin; i < chunkEnd; ++i) {
                    fn(i);
                }
            } catch (...) {
                error = std::current_exception();
            }
            std::lock_guard<std::mutex> lock(doneMutex);
            if (error && !firstError) {
                firstError = error;
            }
            if (--remaining == 0) {
                doneCv.notify_all();
            }
        });
    }

    // Le thread appelant participe au lieu d'attendre passivement : il
    // depile des jobs tant que ses tranches ne sont pas toutes terminees.
    for (;;) {
        {
            std::lock_guard<std::mutex> lock(doneMutex);
            if (remaining == 0) {
                break;
            }
        }
        Task task;
        if (TryPop(task)) {
            task();
        } else {
            // Plus rien a depiler : les dernieres tranches tournent sur les
            // workers, on attend leur signal.
            std::unique_lock<std::mutex> lock(doneMutex);
            doneCv.wait(lock, [&remaining] { return remaining == 0; });
            break;
        }
    }

    if (firstError) {
        std::rethrow_exception(firstError);
    }
}

} // namespace racer::engine
