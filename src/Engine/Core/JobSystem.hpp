/*
** EPITECH PROJECT, 2026
** racer
** File description:
** Thread pool and parallel for job scheduling
*/

#ifndef JOB_SYSTEM_HPP_
#define JOB_SYSTEM_HPP_

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
    explicit JobSystem(unsigned int workerCount = defaultWorkerCount());
    ~JobSystem();

    JobSystem(const JobSystem &) = delete;
    JobSystem &operator=(const JobSystem &) = delete;

    static unsigned int defaultWorkerCount();
    std::size_t workerCount() const { return workers_.size(); }

    std::future<void> submit(std::function<void()> job);
    void parallelFor(
        std::size_t begin,
        std::size_t end,
        std::size_t grainSize,
        const std::function<void(std::size_t)> &fn);

private:
    friend class ParallelForRunner;

    using Task = std::function<void()>;

    void workerLoop();
    void enqueue(Task task);
    bool tryPop(Task &task);

    std::vector<std::thread> workers_;
    std::deque<Task> queue_;
    std::mutex queueMutex_;
    std::condition_variable queueCv_;
    bool stopping_ = false;
};

} // namespace racer::engine

#endif /* !JOB_SYSTEM_HPP_ */
