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
        : remaining_(chunkCount)
    {
    }

    void recordChunkDone(std::exception_ptr error)
    {
        std::lock_guard<std::mutex> lock(doneMutex_);

        if (error && !firstError_)
            firstError_ = error;
        if (--remaining_ == 0)
            doneCv_.notify_all();
    }

    std::size_t remaining_;
    std::exception_ptr firstError_;
    std::mutex doneMutex_;
    std::condition_variable doneCv_;
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
    static void enqueueOneChunk(
        JobSystem &jobs,
        const ParallelForParams &params,
        ParallelBatch &batch,
        std::size_t chunkBegin,
        std::size_t chunkEnd);
    static bool isBatchDone(ParallelBatch &batch);
    static void waitForBatch(ParallelBatch &batch);
    static void rethrowBatchError(const ParallelBatch &batch);
    static void participateUntilDone(
        JobSystem &jobs,
        ParallelBatch &batch);
};

unsigned int JobSystem::defaultWorkerCount()
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
        workers_.emplace_back([this] { workerLoop(); });
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

void JobSystem::enqueue(Task task)
{
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        queue_.push_back(std::move(task));
    }
    queueCv_.notify_one();
}

bool JobSystem::tryPop(Task &task)
{
    std::lock_guard<std::mutex> lock(queueMutex_);

    if (queue_.empty())
        return false;
    task = std::move(queue_.front());
    queue_.pop_front();
    return true;
}

void JobSystem::workerLoop()
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

std::future<void> JobSystem::submit(std::function<void()> job)
{
    auto task = std::make_shared<std::packaged_task<void()>>(
        std::move(job));
    std::future<void> future = task->get_future();

    enqueue([task] { (*task)(); });
    return future;
}

void ParallelForRunner::enqueueOneChunk(
    JobSystem &jobs,
    const ParallelForParams &params,
    ParallelBatch &batch,
    std::size_t chunkBegin,
    std::size_t chunkEnd)
{
    jobs.enqueue([&, chunkBegin, chunkEnd] {
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

        enqueueOneChunk(jobs, params, batch, chunkBegin, chunkEnd);
    }
}

bool ParallelForRunner::isBatchDone(ParallelBatch &batch)
{
    std::lock_guard<std::mutex> lock(batch.doneMutex_);

    return batch.remaining_ == 0;
}

void ParallelForRunner::waitForBatch(ParallelBatch &batch)
{
    std::unique_lock<std::mutex> lock(batch.doneMutex_);
    batch.doneCv_.wait(lock, [&batch] {
        return batch.remaining_ == 0;
    });
}

void ParallelForRunner::rethrowBatchError(const ParallelBatch &batch)
{
    if (batch.firstError_)
        std::rethrow_exception(batch.firstError_);
}

void ParallelForRunner::participateUntilDone(
    JobSystem &jobs,
    ParallelBatch &batch)
{
    for (;;) {
        if (isBatchDone(batch))
            break;
        JobSystem::Task task;
        if (jobs.tryPop(task)) {
            task();
        } else {
            waitForBatch(batch);
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
    rethrowBatchError(batch);
}

void JobSystem::parallelFor(
    std::size_t begin,
    std::size_t end,
    std::size_t grainSize,
    const std::function<void(std::size_t)> &fn)
{
    ParallelForRunner::run(*this, begin, end, grainSize, fn);
}

} // namespace racer::engine
