#include "prefetch.h"

namespace hipo {

record_prefetcher::record_prefetcher(int capacity, int num_producers, producer_fn fill)
    : m_fill(std::move(fill))
    , m_num_producers(std::max(1, num_producers))
{
    if (capacity < 1) capacity = 1;
    m_slots.reserve(static_cast<std::size_t>(capacity));
    for (int i = 0; i < capacity; ++i) {
        m_slots.push_back(std::make_unique<slot>());
        m_free.push(m_slots.back().get());
    }
}

record_prefetcher::~record_prefetcher() {
    stop();
}

void record_prefetcher::start() {
    {
        std::lock_guard lk(m_mu);
        m_active_producers = m_num_producers;
    }
    m_threads.reserve(static_cast<std::size_t>(m_num_producers));
    for (int i = 0; i < m_num_producers; ++i) {
        m_threads.emplace_back([this] { producer_loop(); });
    }
}

void record_prefetcher::stop() {
    {
        std::lock_guard lk(m_mu);
        if (m_stop) return;       // already stopped
        m_stop = true;
    }
    // Wake any blocked waiters so they observe the stop flag.
    m_cv_free.notify_all();
    m_cv_ready.notify_all();
    for (auto& t : m_threads) {
        if (t.joinable()) t.join();
    }
    m_threads.clear();
}

void record_prefetcher::producer_loop() {
    while (true) {
        slot* s = nullptr;
        {
            std::unique_lock lk(m_mu);
            m_cv_free.wait(lk, [this] { return !m_free.empty() || m_stop; });
            if (m_stop) return;
            s = m_free.front();
            m_free.pop();
        }

        // Reset bookkeeping (rec.m_decompressed_buf keeps capacity from
        // the previous iteration — the whole point of pooling).
        s->ok       = false;
        s->work_idx = -1;
        s->rec_idx  = -1;
        s->file_idx = -1;

        bool produced = false;
        try {
            produced = m_fill(*s);
        } catch (...) {
            // If the producer callback throws we still want to drain
            // cleanly: mark done and let workers exit. Re-throwing
            // would abandon other workers mid-flight.
            produced = false;
        }

        if (!produced) {
            // No more work for THIS producer. Return slot to the free
            // pool, decrement the active-producer count, and only
            // flip the global m_done flag once every producer has
            // exited (otherwise a fast producer could mark done
            // while a slow one still has records in flight).
            {
                std::lock_guard lk(m_mu);
                m_free.push(s);
                if (--m_active_producers == 0) m_done = true;
            }
            m_cv_ready.notify_all();
            return;
        }

        {
            std::lock_guard lk(m_mu);
            m_ready.push(s);
        }
        m_cv_ready.notify_one();
    }
}

auto record_prefetcher::pop() -> slot* {
    std::unique_lock lk(m_mu);
    m_cv_ready.wait(lk, [this] {
        // Wake when there's something to consume, when the producer
        // has signaled completion (and we therefore know no more will
        // arrive), or when the world is shutting down.
        return !m_ready.empty() || m_done || m_stop;
    });
    if (m_stop) return nullptr;
    if (m_ready.empty()) return nullptr;       // implies m_done
    slot* s = m_ready.front();
    m_ready.pop();
    return s;
}

auto record_prefetcher::release(slot* s) -> void {
    if (!s) return;
    {
        std::lock_guard lk(m_mu);
        m_free.push(s);
    }
    m_cv_free.notify_one();
}

}  // namespace hipo
