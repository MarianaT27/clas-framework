#include "chain.h"

#include <algorithm>
#include <filesystem>

namespace hipo {

chain::chain(chain_options opts)
    : m_opts(std::move(opts)) {
    m_num_threads = m_opts.threads > 0
        ? m_opts.threads
        : static_cast<int>(std::thread::hardware_concurrency());
    if (m_num_threads < 1) m_num_threads = 1;
    m_thread_pool = std::make_unique<ThreadPool>(m_num_threads, "hipo-chain");
}

chain& chain::add(std::string_view path) {
    chain_file_info info;
    info.path = std::string(path);
    info.index = static_cast<int>(m_files.size());

    std::error_code ec;
    auto sz = std::filesystem::file_size(info.path, ec);
    if (!ec) info.file_size = sz;

    m_files.push_back(std::move(info));
    m_metadata_loaded = false;
    return *this;
}

result<void> chain::open() {
    if (m_files.empty())
        return unexpected(error_code::chain_no_files);

    load_metadata();

    // Load dictionary from first valid file
    for (auto const& f : m_files) {
        if (!f.valid) continue;
        auto rdr = reader::open(f.path, m_opts.reader_opts);
        if (rdr) {
            m_dict = rdr->dict();
            break;
        }
    }

    return ok();
}

void chain::load_metadata() {
    if (m_metadata_loaded) return;

    std::unique_ptr<ProgressTracker> progress;
    if (m_opts.show_progress && m_files.size() > 1) {
        ProgressTracker::Config config;
        config.label = "Scanning files";
        config.show_eta = true;
        config.show_rate = true;
        progress = std::make_unique<ProgressTracker>(m_files.size(), config);
        progress->start();
    }

    std::vector<std::future<void>> futures;
    for (auto& f : m_files) {
        futures.push_back(m_thread_pool->submit([file = &f, &progress]() {
            auto meta = reader::count_events(file->path);
            if (!meta) {
                file->valid = false;
                file->error = fmt::format("{}", meta.error());
            } else {
                file->total_events = meta->total_events;
                file->num_records  = meta->num_records;
            }
            if (progress) progress->increment();
        }));
    }
    for (auto& fut : futures) fut.get();

    if (progress) progress->finish();

    int64_t total = 0;
    for (auto const& f : m_files)
        if (f.valid) total += f.total_events;

    m_stats.total_events = total;
    m_metadata_loaded = true;
}

int64_t chain::total_events() {
    load_metadata();
    return m_stats.total_events.load();
}

void chain::scan() {
    load_metadata();
    fmt::print("\nChain: {} files\n", m_files.size());
    fmt::print("{:>4s}  {:>12s}  {:>10s}  {:>8s}  {}\n", "#", "Events", "Records", "Size", "Path");
    fmt::print("{}\n", std::string(80, '-'));

    for (auto const& f : m_files) {
        std::string size_str;
        if (f.file_size > 1024ULL * 1024 * 1024)
            size_str = fmt::format("{:.1f} GB", f.file_size / (1024.0 * 1024 * 1024));
        else if (f.file_size > 1024 * 1024)
            size_str = fmt::format("{:.1f} MB", f.file_size / (1024.0 * 1024));
        else
            size_str = fmt::format("{:.1f} KB", f.file_size / 1024.0);

        if (f.valid) {
            fmt::print("{:>4d}  {:>12d}  {:>10d}  {:>8s}  {}\n",
                       f.index, f.total_events, f.num_records, size_str, f.path);
        } else {
            fmt::print("{:>4d}  {:>12s}  {:>10s}  {:>8s}  {} [{}]\n",
                       f.index, "ERROR", "-", size_str, f.path, f.error);
        }
    }

    fmt::print("{}\n", std::string(80, '-'));
    fmt::print("Total events: {}\n\n", m_stats.total_events.load());
}

void chain::print_statistics() const {
    fmt::print("\nChain Statistics\n");
    fmt::print("  Files:            {}\n", m_files.size());
    fmt::print("  Total events:     {}\n", m_stats.total_events.load());
    fmt::print("  Events processed: {}\n", m_stats.events_processed.load());
    fmt::print("  Processing time:  {:.2f} seconds\n", m_stats.elapsed_seconds());
    if (m_stats.elapsed_seconds() > 0)
        fmt::print("  Throughput:       {:.1f} events/sec\n", m_stats.throughput());
}

} // namespace hipo
