/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Thread pool and parallel for job scheduling
*/

#ifndef JOBS_H_
#define JOBS_H_

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace racer::engine {

class ParallelForRunner;

class JobSystem {
public:
    explicit JobSystem(unsigned int workerCount = DefaultWorkerCount());
    ~JobSystem();

    JobSystem(const JobSystem &) = delete;
    JobSystem &operator=(const JobSystem &) = delete;

    static unsigned int DefaultWorkerCount();
    std::size_t WorkerCount() const { return workers_.size(); }

    std::future<void> Submit(std::function<void()> job);
    void ParallelFor(
        std::size_t begin,
        std::size_t end,
        std::size_t grainSize,
        const std::function<void(std::size_t)> &fn);

private:
    friend class ParallelForRunner;

    using Task = std::function<void()>;

    void WorkerLoop();
    void Enqueue(Task task);
    bool TryPop(Task &task);

    std::vector<std::thread> workers_;
    std::deque<Task> queue_;
    std::mutex queueMutex_;
    std::condition_variable queueCv_;
    bool stopping_ = false;
};

} // namespace racer::engine

#endif /* !JOBS_H_ */
