#include "parallel/parallel_for.h"

#include <algorithm>
#include <atomic>
#include <exception>
#include <mutex>
#include <thread>
#include <vector>

int resolveThreadCount(int requested) {
    if (requested == 0) {
        const unsigned hw = std::thread::hardware_concurrency();
        return hw == 0 ? 1 : static_cast<int>(hw);
    }
    return requested < 0 ? 1 : requested;
}

void parallelFor(std::size_t count, int threads, const std::function<void(std::size_t)>& body) {
    const std::size_t workers = std::min<std::size_t>(static_cast<std::size_t>(std::max(1, threads)), count);
    if (workers <= 1) {
        for (std::size_t i = 0; i < count; ++i) body(i);
        return;
    }

    std::atomic<std::size_t> next{0};
    std::exception_ptr error;
    std::mutex errorMutex;

    auto worker = [&]() {
        for (;;) {
            const std::size_t i = next.fetch_add(1);
            if (i >= count) return;
            try {
                body(i);
            } catch (...) {
                std::lock_guard<std::mutex> lock(errorMutex);
                if (!error) error = std::current_exception();
                next.store(count);  // zatrzymaj pozostale watki
                return;
            }
        }
    };

    std::vector<std::thread> pool;
    pool.reserve(workers - 1);
    for (std::size_t t = 1; t < workers; ++t) pool.emplace_back(worker);
    worker();  // watek wywolujacy tez pracuje
    for (auto& t : pool) t.join();
    if (error) std::rethrow_exception(error);
}
