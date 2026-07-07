/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Thread pool worker loop and parallel for implementation
*/

#include "Engine/Core/JobSystem.hpp"

#include <algorithm>
#include <exception>
#include <memory>
#include <utility>

namespace racer::engine {

class ParallelBatch {
public:
    explicit ParallelBatch(std::size_t chunkCount)
        : remaining(chunkCount)
    {
    }

    void recordChunkDone(std::exception_ptr error)
    {
        std::lock_guard<std::mutex> lock(doneMutex);

        if (error && !firstError)
            firstError = error;
        if (--remaining == 0)
            doneCv.notify_all();
    }

    std::size_t remaining;
    std::exception_ptr firstError;
    std::mutex doneMutex;
    std::condition_variable doneCv;
};

struct ParallelForParams {
    std::size_t begin;
    std::size_t end;
    std::size_t grainSize;
    const std::function<void(std::size_t)> &fn;
};

class ParallelForRunner {
public:
    static void run(
        JobSystem &jobs,
        std::size_t begin,
        std::size_t end,
        std::size_t grainSize,
        const std::function<void(std::size_t)> &fn);

private:
    static void enqueueChunks(
        JobSystem &jobs,
        const ParallelForParams &params,
        ParallelBatch &batch);
    static void participateUntilDone(
        JobSystem &jobs,
        ParallelBatch &batch);
};

unsigned int JobSystem::DefaultWorkerCount()
{
    const unsigned int hardware = std::thread::hardware_concurrency();

    return hardware > 1u ? hardware - 1u : 1u;
}

JobSystem::JobSystem(unsigned int workerCount)
{
    if (workerCount == 0u)
        workerCount = 1u;
    workers_.reserve(workerCount);
    for (unsigned int i = 0; i < workerCount; ++i)
        workers_.emplace_back([this] { WorkerLoop(); });
}

JobSystem::~JobSystem()
{
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        stopping_ = true;
    }
    queueCv_.notify_all();
    for (std::thread &worker : workers_)
        worker.join();
}

void JobSystem::Enqueue(Task task)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        queue_.push_back(std::move(task));
    }
    queueCv_.notify_one();
}

bool JobSystem::TryPop(Task &task)
{
    std::lock_guard<std::mutex> lock(queueMutex_);

    if (queue_.empty())
        return false;
    task = std::move(queue_.front());
    queue_.pop_front();
    return true;
}

void JobSystem::WorkerLoop()
{
    for (;;) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCv_.wait(lock, [this] {
                return stopping_ || !queue_.empty();
            });
            if (queue_.empty())
                return;
            task = std::move(queue_.front());
            queue_.pop_front();
        }
        task();
    }
}

std::future<void> JobSystem::Submit(std::function<void()> job)
{
    auto task = std::make_shared<std::packaged_task<void()>>(
        std::move(job));
    std::future<void> future = task->get_future();

    Enqueue([task] { (*task)(); });
    return future;
}

void ParallelForRunner::enqueueChunks(
    JobSystem &jobs,
    const ParallelForParams &params,
    ParallelBatch &batch)
{
    const std::size_t count = params.end - params.begin;
    const std::size_t chunkCount =
        (count + params.grainSize - 1) / params.grainSize;

    for (std::size_t c = 0; c < chunkCount; ++c) {
        const std::size_t chunkBegin = params.begin + c * params.grainSize;
        const std::size_t chunkEnd = std::min(
            params.end, chunkBegin + params.grainSize);

        jobs.Enqueue([&, chunkBegin, chunkEnd] {
            std::exception_ptr error;
            try {
                for (std::size_t i = chunkBegin; i < chunkEnd; ++i)
                    params.fn(i);
            } catch (...) {
                error = std::current_exception();
            }
            batch.recordChunkDone(error);
        });
    }
}

void ParallelForRunner::participateUntilDone(
    JobSystem &jobs,
    ParallelBatch &batch)
{
    for (;;) {
        {
            std::lock_guard<std::mutex> lock(batch.doneMutex);
            if (batch.remaining == 0)
                break;
        }
        JobSystem::Task task;
        if (jobs.TryPop(task)) {
            task();
        } else {
            std::unique_lock<std::mutex> lock(batch.doneMutex);
            batch.doneCv.wait(lock, [&batch] {
                return batch.remaining == 0;
            });
            break;
        }
    }
}

void ParallelForRunner::run(
    JobSystem &jobs,
    std::size_t begin,
    std::size_t end,
    std::size_t grainSize,
    const std::function<void(std::size_t)> &fn)
{
    if (begin >= end)
        return;
    if (grainSize == 0)
        grainSize = 1;
    const std::size_t count = end - begin;
    const std::size_t chunkCount = (count + grainSize - 1) / grainSize;
    ParallelBatch batch(chunkCount);
    const ParallelForParams params{begin, end, grainSize, fn};

    enqueueChunks(jobs, params, batch);
    participateUntilDone(jobs, batch);
    if (batch.firstError)
        std::rethrow_exception(batch.firstError);
}

void JobSystem::ParallelFor(
    std::size_t begin,
    std::size_t end,
    std::size_t grainSize,
    const std::function<void(std::size_t)> &fn)
{
    ParallelForRunner::run(*this, begin, end, grainSize, fn);
}

} // namespace racer::engine
