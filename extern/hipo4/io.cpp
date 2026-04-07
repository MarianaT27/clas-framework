#include "io.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace hipo {

// ============================================================================
// file_source (POSIX fd + pread)
// ============================================================================

file_source::~file_source() {
    close();
}

file_source::file_source(file_source&& other) noexcept
    : m_fd(other.m_fd),
      m_path(std::move(other.m_path)),
      m_size(other.m_size) {
    other.m_fd = -1;
    other.m_size = 0;
}

file_source& file_source::operator=(file_source&& other) noexcept {
    if (this != &other) {
        close();
        m_fd = other.m_fd;
        m_path = std::move(other.m_path);
        m_size = other.m_size;
        other.m_fd = -1;
        other.m_size = 0;
    }
    return *this;
}

result<file_source> file_source::open(std::string_view path) {
    file_source src;
    src.m_path = std::string(path);

    src.m_fd = ::open(src.m_path.c_str(), O_RDONLY);
    if (src.m_fd < 0)
        return unexpected(error_code::file_open_failed);

    struct stat st{};
    if (::fstat(src.m_fd, &st) != 0) {
        ::close(src.m_fd);
        src.m_fd = -1;
        return unexpected(error_code::file_not_found);
    }

    src.m_size = st.st_size;

    // Enable kernel read-ahead for sequential access patterns
#ifdef __APPLE__
    (void)::fcntl(src.m_fd, F_RDAHEAD, 1);
#elif defined(__linux__)
    (void)::posix_fadvise(src.m_fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

    return src;
}

result<int> file_source::read_at(int64_t pos, char* dest, int size) {
    auto n = ::pread(m_fd, dest, static_cast<size_t>(size), static_cast<off_t>(pos));
    if (n < 0)
        return unexpected(error_code::file_read_failed);
    return static_cast<int>(n);
}

void file_source::prefetch(int64_t pos, int64_t size) const noexcept {
#ifdef __linux__
    if (m_fd >= 0) {
        (void)::posix_fadvise(m_fd, static_cast<off_t>(pos),
                              static_cast<off_t>(size), POSIX_FADV_WILLNEED);
    }
#else
    (void)pos;
    (void)size;
#endif
}

void file_source::close() {
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}

} // namespace hipo
