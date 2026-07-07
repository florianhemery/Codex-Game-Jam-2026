#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

namespace racer::engine {

// Pool de threads minimaliste : file de jobs protegee par mutex/condvar.
// Toutes les methodes publiques sont thread-safe. Ne pas soumettre de job
// pendant la destruction (l'appelant possede le JobSystem) ; les jobs deja
// en file sont draines avant l'arret.
class JobSystem {
public:
    // workerCount == 0 est rehausse a 1. Par defaut : coeurs materiels - 1.
    explicit JobSystem(unsigned int workerCount = DefaultWorkerCount());
    ~JobSystem();

    JobSystem(const JobSystem&) = delete;
    JobSystem& operator=(const JobSystem&) = delete;

    static unsigned int DefaultWorkerCount();
    std::size_t WorkerCount() const { return workers_.size(); }

    // Planifie un job ; le future permet d'attendre sa fin (et propage
    // l'exception eventuelle levee par le job).
    std::future<void> Submit(std::function<void()> job);

    // Applique fn(i) pour i dans [begin, end), par tranches de grainSize.
    // Bloquant : le thread appelant execute lui aussi des jobs en attendant
    // (pas d'interblocage meme avec un seul worker). Relance la premiere
    // exception levee par fn.
    void ParallelFor(std::size_t begin, std::size_t end, std::size_t grainSize,
                     const std::function<void(std::size_t)>& fn);

private:
    using Task = std::function<void()>;

    void WorkerLoop();
    void Enqueue(Task task);
    bool TryPop(Task& task);

    std::vector<std::thread> workers_;
    std::deque<Task> queue_; // protegee par queueMutex_
    std::mutex queueMutex_;
    std::condition_variable queueCv_;
    bool stopping_ = false; // protege par queueMutex_
};

} // namespace racer::engine
