#include "file_sink.h"

#include <fcntl.h>
#include <unistd.h>

namespace hipo {

file_sink::~file_sink() {
    close();
}

file_sink::file_sink(file_sink&& other) noexcept
    : m_fd(other.m_fd),
      m_path(std::move(other.m_path)),
      m_position(other.m_position) {
    other.m_fd = -1;
    other.m_position = 0;
}

file_sink& file_sink::operator=(file_sink&& other) noexcept {
    if (this != &other) {
        close();
        m_fd = other.m_fd;
        m_path = std::move(other.m_path);
        m_position = other.m_position;
        other.m_fd = -1;
        other.m_position = 0;
    }
    return *this;
}

result<file_sink> file_sink::open(std::string_view path) {
    file_sink sink;
    sink.m_path = std::string(path);

    sink.m_fd = ::open(sink.m_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (sink.m_fd < 0)
        return unexpected(error_code::file_open_failed);

    sink.m_position = 0;
    return sink;
}

result<void> file_sink::write(const char* data, int size) {
    int written = 0;
    while (written < size) {
        auto n = ::write(m_fd, data + written, static_cast<size_t>(size - written));
        if (n < 0)
            return unexpected(error_code::write_failed);
        written += static_cast<int>(n);
    }
    m_position += written;
    return ok();
}

result<void> file_sink::write_at(int64_t pos, const char* data, int size) {
    int written = 0;
    while (written < size) {
        auto n = ::pwrite(m_fd, data + written, static_cast<size_t>(size - written),
                          static_cast<off_t>(pos + written));
        if (n < 0)
            return unexpected(error_code::write_failed);
        written += static_cast<int>(n);
    }
    return ok();
}

void file_sink::close() {
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}

} // namespace hipo
