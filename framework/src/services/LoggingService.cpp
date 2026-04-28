// Out-of-line definitions for LoggingService.
//
// Previously every non-template member lived in the header, which pulled
// ~1700 LOC (plus a grand tour of box-drawing UTF-8 sequences) into every
// translation unit that logged anything. That meant the whole project got
// recompiled whenever someone tweaked a print_table() row. This .cpp moves
// the printing and formatting machinery out of the header; the header now
// only carries templates (fmt::format callers) and trivial inline setters.

#include <clas12/framework/services/LoggingService.hpp>

#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <fmt/format.h>

namespace framework {

namespace {

// Box-drawing UTF-8 sequences used by the various table printers.
// Kept at file scope so each definition isn't re-creating its own locals.
constexpr const char* BOX_TL = "\342\224\214";  // ┌
constexpr const char* BOX_TR = "\342\224\220";  // ┐
constexpr const char* BOX_BL = "\342\224\224";  // └
constexpr const char* BOX_BR = "\342\224\230";  // ┘
constexpr const char* BOX_H  = "\342\224\200";  // ─
constexpr const char* BOX_V  = "\342\224\202";  // │
constexpr const char* BOX_TM = "\342\224\254";  // ┬
constexpr const char* BOX_BM = "\342\224\264";  // ┴
constexpr const char* BOX_LM = "\342\224\234";  // ├
constexpr const char* BOX_RM = "\342\224\244";  // ┤
constexpr const char* BOX_X  = "\342\224\274";  // ┼

auto repeat(const char* ch, int n) -> std::string {
    std::string result;
    for (int i = 0; i < n; ++i) result += ch;
    return result;
}

}  // namespace

auto LoggingService::initialize() -> StatusCode {
    info("LoggingService", "Initialized");
    return StatusCode::SUCCESS();
}

auto LoggingService::finalize() -> StatusCode {
    info("LoggingService", "Finalized");
    if (m_file_stream.is_open()) {
        m_file_stream.close();
    }
    return StatusCode::SUCCESS();
}

auto LoggingService::set_source_level(const std::string& source, LogLevel level) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_source_levels[source] = level;
}

auto LoggingService::set_log_file(const std::string& filename) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file_stream.is_open()) {
        m_file_stream.close();
    }
    m_log_file = filename;
    m_file_stream.open(filename, std::ios::out | std::ios::trunc);
    if (!m_file_stream.is_open()) {
        fmt::print(stderr, "WARNING: Failed to open log file: {}\n", filename);
    }
}

auto LoggingService::section(const std::string& title) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_console_output) {
        if (m_colored_output) {
            fmt::print("\n\033[1;36m{}\033[0m\n", std::string(60, '\342'));
            fmt::print("\033[1;36m  {}\033[0m\n", title);
            fmt::print("\033[1;36m{}\033[0m\n\n", std::string(60, '\342'));
        } else {
            fmt::print("\n{}\n", std::string(60, '='));
            fmt::print("  {}\n", title);
            fmt::print("{}\n\n", std::string(60, '='));
        }
    }
    write_to_file_raw("\n" + std::string(60, '=') + "\n  " + title + "\n" + std::string(60, '=') + "\n");
}

auto LoggingService::subsection(const std::string& title) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_console_output) {
        if (m_colored_output) {
            fmt::print("\n\033[1;37m  {}\033[0m\n", title);
            fmt::print("\033[90m  {}\033[0m\n", std::string(title.length() + 4, '-'));
        } else {
            fmt::print("\n  {}\n", title);
            fmt::print("  {}\n", std::string(title.length(), '-'));
        }
    }
    write_to_file_raw("\n  " + title + "\n  " + std::string(title.length(), '-') + "\n");
}

auto LoggingService::divider(char c, int width) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_console_output) {
        if (m_colored_output) {
            fmt::print("\033[90m{}\033[0m\n", std::string(width, c));
        } else {
            fmt::print("{}\n", std::string(width, c));
        }
    }
    write_to_file_raw(std::string(width, c) + "\n");
}

auto LoggingService::start(const std::string& source, const std::string& operation) -> void {
    if (LogLevel::INFO < m_min_level) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_console_output) {
        if (m_colored_output) {
            std::string icon = m_show_icons ? "\033[33m\342\226\266\033[0m " : "";
            fmt::print("{}{}\033[33m{:<16}\033[0m Starting {}...\n",
                       format_timestamp(), icon, source, operation);
        } else {
            fmt::print("{}[>>] {:<16} Starting {}...\n", format_timestamp(), source, operation);
        }
    }
    write_to_file("START", source, "Starting " + operation);
}

auto LoggingService::done(const std::string& source, const std::string& operation, double duration_ms) -> void {
    if (LogLevel::INFO < m_min_level) return;
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string duration_str = format_duration(duration_ms);
    if (m_console_output) {
        if (m_colored_output) {
            std::string icon = m_show_icons ? "\033[32m\342\234\223\033[0m " : "";
            fmt::print("{}{}\033[32m{:<16}\033[0m {} completed \033[90m({})\033[0m\n",
                       format_timestamp(), icon, source, operation, duration_str);
        } else {
            fmt::print("{}[OK] {:<16} {} completed ({})\n",
                       format_timestamp(), source, operation, duration_str);
        }
    }
    write_to_file("DONE", source, fmt::format("{} completed ({})", operation, duration_str));
}

auto LoggingService::progress(const std::string& source, size_t current, size_t total,
                              const std::string& item) -> void {
    if (LogLevel::INFO < m_min_level) return;
    if (total == 0) return;
    std::lock_guard<std::mutex> lock(m_mutex);

    double percent = (static_cast<double>(current) / total) * 100.0;
    int bar_width = 30;
    int filled = static_cast<int>((percent / 100.0) * bar_width);

    if (m_console_output) {
        std::string bar(filled, '\342');
        std::string empty(bar_width - filled, ' ');

        if (m_colored_output) {
            fmt::print("\r{}\033[90m{:<16}\033[0m [\033[32m{}\033[90m{}\033[0m] "
                       "\033[1m{:5.1f}%\033[0m ({}/{} {})",
                       format_timestamp(), source, bar, empty, percent, current, total, item);
        } else {
            std::string simple_bar(filled, '#');
            std::string simple_empty(bar_width - filled, '-');
            fmt::print("\r{}{:<16} [{}{}] {:5.1f}% ({}/{} {})",
                       format_timestamp(), source, simple_bar, simple_empty, percent, current, total, item);
        }

        if (current == total) {
            fmt::print("\n");
        } else {
            std::fflush(stdout);
        }
    }
}

auto LoggingService::blank() -> void {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_console_output) {
        fmt::print("\n");
    }
    write_to_file_raw("\n");
}

auto LoggingService::print_table(const TableBuilder& tb) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);

    int total_width = 1;
    for (size_t i = 0; i < tb.m_widths.size(); ++i) {
        total_width += tb.m_widths[i] + 3;
    }
    total_width -= 1;

    auto make_horizontal = [&](const char* left, const char* mid, const char* right) {
        std::string line = left;
        for (size_t i = 0; i < tb.m_widths.size(); ++i) {
            for (int j = 0; j < tb.m_widths[i] + 2; ++j) {
                line += BOX_H;
            }
            if (i < tb.m_widths.size() - 1) {
                line += mid;
            }
        }
        line += right;
        return line;
    };

    auto make_row = [&](const std::vector<std::string>& cells, [[maybe_unused]] bool header = false) {
        std::string line = BOX_V;
        for (size_t i = 0; i < tb.m_widths.size() && i < cells.size(); ++i) {
            std::string cell = cells[i];
            int width = tb.m_widths[i];

            if (static_cast<int>(cell.length()) > width) {
                cell = cell.substr(0, width - 2) + "..";
            }

            bool is_number = !cell.empty() && (std::isdigit(cell[0]) || cell[0] == '-' || cell[0] == '+');
            if (is_number) {
                line += fmt::format(" {:>{}} ", cell, width);
            } else {
                line += fmt::format(" {:<{}} ", cell, width);
            }
            line += BOX_V;
        }
        return line;
    };

    if (m_console_output) {
        std::string top_line = make_horizontal(BOX_TL, BOX_TM, BOX_TR);
        int content_width = total_width - 2;

        if (m_colored_output) {
            fmt::print("\n\033[36m{}\033[0m\n", top_line);
            fmt::print("\033[36m{}\033[0m\033[1;37m{:^{}}\033[0m\033[36m{}\033[0m\n",
                       BOX_V, tb.m_title, content_width, BOX_V);
        } else {
            fmt::print("\n{}\n", top_line);
            fmt::print("{}{:^{}}{}\n", BOX_V, tb.m_title, content_width, BOX_V);
        }

        std::string header_sep = make_horizontal(BOX_LM, BOX_X, BOX_RM);
        if (m_colored_output) {
            fmt::print("\033[36m{}\033[0m\n", header_sep);
        } else {
            fmt::print("{}\n", header_sep);
        }

        if (!tb.m_columns.empty()) {
            if (m_colored_output) {
                fmt::print("\033[36m{}\033[0m\n", BOX_V);
                fmt::print("\033[36m{}\033[0m", BOX_V);
                for (size_t i = 0; i < tb.m_widths.size() && i < tb.m_columns.size(); ++i) {
                    std::string cell = tb.m_columns[i];
                    int width = tb.m_widths[i];
                    if (static_cast<int>(cell.length()) > width) {
                        cell = cell.substr(0, width - 2) + "..";
                    }
                    fmt::print("\033[1;37m {:<{}} \033[0m\033[36m{}\033[0m", cell, width, BOX_V);
                }
                fmt::print("\n");
            } else {
                fmt::print("{}\n", make_row(tb.m_columns));
            }

            if (m_colored_output) {
                fmt::print("\033[36m{}\033[0m\n", header_sep);
            } else {
                fmt::print("{}\n", header_sep);
            }
        }

        for (const auto& row : tb.m_rows) {
            if (m_colored_output) {
                fmt::print("\033[36m{}\033[0m", BOX_V);
                for (size_t i = 0; i < tb.m_widths.size() && i < row.size(); ++i) {
                    std::string cell = row[i];
                    int width = tb.m_widths[i];
                    if (static_cast<int>(cell.length()) > width) {
                        cell = cell.substr(0, width - 2) + "..";
                    }
                    bool is_number = !cell.empty() && (std::isdigit(cell[0]) || cell[0] == '-' || cell[0] == '+');
                    if (is_number) {
                        fmt::print("\033[33m {:>{}} \033[0m\033[36m{}\033[0m", cell, width, BOX_V);
                    } else {
                        fmt::print("\033[37m {:<{}} \033[0m\033[36m{}\033[0m", cell, width, BOX_V);
                    }
                }
                fmt::print("\n");
            } else {
                fmt::print("{}\n", make_row(row));
            }
        }

        if (!tb.m_footer.empty()) {
            std::string footer_sep = make_horizontal(BOX_LM, BOX_BM, BOX_RM);
            if (m_colored_output) {
                fmt::print("\033[36m{}\033[0m\n", footer_sep);
                fmt::print("\033[36m{}\033[0m\033[1;32m {:<{}} \033[0m\033[36m{}\033[0m\n",
                           BOX_V, tb.m_footer, content_width - 1, BOX_V);
            } else {
                fmt::print("{}\n", footer_sep);
                fmt::print("{} {:<{}} {}\n", BOX_V, tb.m_footer, content_width - 1, BOX_V);
            }
        }

        std::string bottom_line = make_horizontal(BOX_BL, BOX_BM, BOX_BR);
        if (m_colored_output) {
            fmt::print("\033[36m{}\033[0m\n", bottom_line);
        } else {
            fmt::print("{}\n", bottom_line);
        }
    }

    if (m_file_stream.is_open()) {
        write_to_file_raw("\n");
        write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");
        int content_width = total_width - 2;
        write_to_file_raw("|" + fmt::format("{:^{}}", tb.m_title, content_width) + "|\n");
        write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");

        if (!tb.m_columns.empty()) {
            std::string header_line = "|";
            for (size_t i = 0; i < tb.m_widths.size() && i < tb.m_columns.size(); ++i) {
                header_line += fmt::format(" {:<{}} |", tb.m_columns[i], tb.m_widths[i]);
            }
            write_to_file_raw(header_line + "\n");
            write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");
        }

        for (const auto& row : tb.m_rows) {
            std::string line = "|";
            for (size_t i = 0; i < tb.m_widths.size() && i < row.size(); ++i) {
                bool is_number = !row[i].empty() && (std::isdigit(row[i][0]) || row[i][0] == '-');
                if (is_number) {
                    line += fmt::format(" {:>{}} |", row[i], tb.m_widths[i]);
                } else {
                    line += fmt::format(" {:<{}} |", row[i], tb.m_widths[i]);
                }
            }
            write_to_file_raw(line + "\n");
        }

        if (!tb.m_footer.empty()) {
            write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");
            write_to_file_raw("| " + fmt::format("{:<{}}", tb.m_footer, content_width - 2) + " |\n");
        }
        write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");
    }
}

auto LoggingService::print_cutflow_table(
    const std::string& title,
    const std::vector<std::tuple<std::string, size_t, double, double>>& cuts,
    double efficiency
) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);

    constexpr int CUT_WIDTH = 35;
    constexpr int COUNT_WIDTH = 12;
    constexpr int EFF_WIDTH = 10;
    constexpr int TOTAL_WIDTH = CUT_WIDTH + COUNT_WIDTH + EFF_WIDTH + EFF_WIDTH + 13;

    std::string top = std::string(BOX_TL) + repeat(BOX_H, TOTAL_WIDTH - 2) + BOX_TR;
    std::string title_sep = std::string(BOX_LM) + repeat(BOX_H, CUT_WIDTH + 2) + BOX_TM +
                            repeat(BOX_H, COUNT_WIDTH + 2) + BOX_TM +
                            repeat(BOX_H, EFF_WIDTH + 2) + BOX_TM +
                            repeat(BOX_H, EFF_WIDTH + 2) + BOX_RM;
    std::string header_sep = std::string(BOX_LM) + repeat(BOX_H, CUT_WIDTH + 2) + BOX_X +
                             repeat(BOX_H, COUNT_WIDTH + 2) + BOX_X +
                             repeat(BOX_H, EFF_WIDTH + 2) + BOX_X +
                             repeat(BOX_H, EFF_WIDTH + 2) + BOX_RM;
    std::string footer_sep = std::string(BOX_LM) + repeat(BOX_H, CUT_WIDTH + 2) + BOX_BM +
                             repeat(BOX_H, COUNT_WIDTH + 2) + BOX_BM +
                             repeat(BOX_H, EFF_WIDTH + 2) + BOX_BM +
                             repeat(BOX_H, EFF_WIDTH + 2) + BOX_RM;
    std::string bottom = std::string(BOX_BL) + repeat(BOX_H, TOTAL_WIDTH - 2) + BOX_BR;

    if (m_console_output) {
        if (m_colored_output) {
            fmt::print("\n\033[36m{}\033[0m\n", top);
            fmt::print("\033[36m{}\033[0m\033[1;37m{:^{}}\033[0m\033[36m{}\033[0m\n",
                       BOX_V, "Cutflow: " + title, TOTAL_WIDTH - 2, BOX_V);
            fmt::print("\033[36m{}\033[0m\n", title_sep);
        } else {
            fmt::print("\n{}\n", top);
            fmt::print("{}{:^{}}{}\n", BOX_V, "Cutflow: " + title, TOTAL_WIDTH - 2, BOX_V);
            fmt::print("{}\n", title_sep);
        }

        if (m_colored_output) {
            fmt::print("\033[36m{}\033[0m\033[1;37m {:<{}} \033[0m\033[36m{}\033[0m"
                       "\033[1;37m {:>{}} \033[0m\033[36m{}\033[0m"
                       "\033[1;37m {:>{}} \033[0m\033[36m{}\033[0m"
                       "\033[1;37m {:>{}} \033[0m\033[36m{}\033[0m\n",
                       BOX_V, "Cut", CUT_WIDTH, BOX_V,
                       "Events", COUNT_WIDTH, BOX_V,
                       "Abs Eff", EFF_WIDTH, BOX_V,
                       "Rel Eff", EFF_WIDTH, BOX_V);
            fmt::print("\033[36m{}\033[0m\n", header_sep);
        } else {
            fmt::print("{} {:<{}} {} {:>{}} {} {:>{}} {} {:>{}} {}\n",
                       BOX_V, "Cut", CUT_WIDTH, BOX_V,
                       "Events", COUNT_WIDTH, BOX_V,
                       "Abs Eff", EFF_WIDTH, BOX_V,
                       "Rel Eff", EFF_WIDTH, BOX_V);
            fmt::print("{}\n", header_sep);
        }

        for (const auto& [cut_name, count, abs_eff, rel_eff] : cuts) {
            std::string count_str = fmt::format("{}", count);
            std::string abs_str = fmt::format("{:>6.1f}%", abs_eff);
            std::string rel_str = fmt::format("{:>6.1f}%", rel_eff);

            if (m_colored_output) {
                std::string abs_color = abs_eff >= 90 ? "\033[32m" : (abs_eff >= 50 ? "\033[33m" : "\033[31m");
                std::string rel_color = rel_eff >= 95 ? "\033[32m" : (rel_eff >= 80 ? "\033[33m" : "\033[31m");

                fmt::print("\033[36m{}\033[0m\033[37m {:<{}} \033[0m\033[36m{}\033[0m"
                           "\033[1;33m {:>{}} \033[0m\033[36m{}\033[0m"
                           "{} {:>{}} \033[0m\033[36m{}\033[0m"
                           "{} {:>{}} \033[0m\033[36m{}\033[0m\n",
                           BOX_V, cut_name, CUT_WIDTH, BOX_V,
                           count_str, COUNT_WIDTH, BOX_V,
                           abs_color, abs_str, EFF_WIDTH, BOX_V,
                           rel_color, rel_str, EFF_WIDTH, BOX_V);
            } else {
                fmt::print("{} {:<{}} {} {:>{}} {} {:>{}} {} {:>{}} {}\n",
                           BOX_V, cut_name, CUT_WIDTH, BOX_V,
                           count_str, COUNT_WIDTH, BOX_V,
                           abs_str, EFF_WIDTH, BOX_V,
                           rel_str, EFF_WIDTH, BOX_V);
            }
        }

        std::string eff_str = fmt::format("Overall efficiency: {:.2f}%", efficiency);
        if (m_colored_output) {
            fmt::print("\033[36m{}\033[0m\n", footer_sep);
            std::string eff_color = efficiency >= 90 ? "\033[1;32m" : (efficiency >= 50 ? "\033[1;33m" : "\033[1;31m");
            fmt::print("\033[36m{}\033[0m{} {:<{}} \033[0m\033[36m{}\033[0m\n",
                       BOX_V, eff_color, eff_str, TOTAL_WIDTH - 4, BOX_V);
            fmt::print("\033[36m{}\033[0m\n", bottom);
        } else {
            fmt::print("{}\n", footer_sep);
            fmt::print("{} {:<{}} {}\n", BOX_V, eff_str, TOTAL_WIDTH - 4, BOX_V);
            fmt::print("{}\n", bottom);
        }
    }

    if (m_file_stream.is_open()) {
        std::string eff_str_file = fmt::format("Overall efficiency: {:.2f}%", efficiency);
        write_to_file_raw("\n+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
        write_to_file_raw(fmt::format("|{:^{}}|\n", "Cutflow: " + title, TOTAL_WIDTH - 2));
        write_to_file_raw("+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
        write_to_file_raw(fmt::format("| {:<{}} | {:>{}} | {:>{}} | {:>{}} |\n",
                                     "Cut", CUT_WIDTH, "Events", COUNT_WIDTH - 1,
                                     "Abs Eff", EFF_WIDTH - 1, "Rel Eff", EFF_WIDTH - 1));
        write_to_file_raw("+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");

        for (const auto& [cut_name, count, abs_eff, rel_eff] : cuts) {
            write_to_file_raw(fmt::format("| {:<{}} | {:>{}} | {:>6.1f}%   | {:>6.1f}%   |\n",
                                         cut_name, CUT_WIDTH, count, COUNT_WIDTH - 1,
                                         abs_eff, rel_eff));
        }

        write_to_file_raw("+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
        write_to_file_raw(fmt::format("| {:<{}} |\n", eff_str_file, TOTAL_WIDTH - 4));
        write_to_file_raw("+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
    }
}

auto LoggingService::print_fit_results_table(
    const std::string& title,
    const std::vector<std::tuple<std::string, std::string, std::string>>& rows
) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);

    constexpr int LABEL_WIDTH = 15;
    constexpr int VALUE_WIDTH = 25;
    constexpr int UNIT_WIDTH = 12;
    constexpr int TOTAL_WIDTH = LABEL_WIDTH + VALUE_WIDTH + UNIT_WIDTH + 10;

    std::string top = std::string(BOX_TL) + repeat(BOX_H, TOTAL_WIDTH - 2) + BOX_TR;
    std::string sep = std::string(BOX_LM) + repeat(BOX_H, TOTAL_WIDTH - 2) + BOX_RM;
    std::string bottom = std::string(BOX_BL) + repeat(BOX_H, TOTAL_WIDTH - 2) + BOX_BR;

    if (m_console_output) {
        if (m_colored_output) {
            fmt::print("\n\033[36m{}\033[0m\n", top);
            fmt::print("\033[36m{}\033[0m\033[1;37m {:^{}} \033[0m\033[36m{}\033[0m\n",
                       BOX_V, title, TOTAL_WIDTH - 4, BOX_V);
            fmt::print("\033[36m{}\033[0m\n", sep);

            for (const auto& [label, value, unit] : rows) {
                fmt::print("\033[36m{}\033[0m\033[37m {:<{}} \033[0m"
                           "\033[1;33m{:>{}} \033[0m"
                           "\033[37m{:<{}} \033[0m\033[36m{}\033[0m\n",
                           BOX_V, label, LABEL_WIDTH,
                           value, VALUE_WIDTH,
                           unit, UNIT_WIDTH + 4, BOX_V);
            }
            fmt::print("\033[36m{}\033[0m\n", bottom);
        } else {
            fmt::print("\n{}\n", top);
            fmt::print("{} {:^{}} {}\n", BOX_V, title, TOTAL_WIDTH - 4, BOX_V);
            fmt::print("{}\n", sep);

            for (const auto& [label, value, unit] : rows) {
                fmt::print("{} {:<{}} {:>{}} {:<{}} {}\n",
                           BOX_V, label, LABEL_WIDTH,
                           value, VALUE_WIDTH,
                           unit, UNIT_WIDTH + 4, BOX_V);
            }
            fmt::print("{}\n", bottom);
        }
    }

    if (m_file_stream.is_open()) {
        write_to_file_raw("\n+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
        write_to_file_raw(fmt::format("| {:^{}} |\n", title, TOTAL_WIDTH - 2));
        write_to_file_raw("+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
        for (const auto& [label, value, unit] : rows) {
            write_to_file_raw(fmt::format("| {:<{}} {:>{}} {:<{}} |\n",
                                         label, LABEL_WIDTH, value, VALUE_WIDTH, unit, UNIT_WIDTH));
        }
        write_to_file_raw("+" + std::string(TOTAL_WIDTH - 2, '-') + "+\n");
    }
}

auto LoggingService::print_yield_summary_table(
    const std::string& title,
    const std::vector<std::string>& columns,
    const std::vector<std::vector<std::string>>& rows,
    const std::string& footer
) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<int> widths;
    if (columns.size() == 4) {
        widths = {20, 14, 16, 12};
    } else {
        for (size_t i = 0; i < columns.size(); ++i) {
            widths.push_back(15);
        }
    }

    int total_width = 1;
    for (int w : widths) {
        total_width += w + 3;
    }

    auto make_sep = [&](const char* left, const char* mid, const char* right) {
        std::string line = left;
        for (size_t i = 0; i < widths.size(); ++i) {
            line += repeat(BOX_H, widths[i] + 2);
            if (i < widths.size() - 1) line += mid;
        }
        line += right;
        return line;
    };

    std::string top = std::string(BOX_TL) + repeat(BOX_H, total_width - 2) + BOX_TR;
    std::string title_sep = make_sep(BOX_LM, BOX_TM, BOX_RM);
    std::string header_sep = make_sep(BOX_LM, BOX_X, BOX_RM);
    std::string footer_sep = make_sep(BOX_LM, BOX_BM, BOX_RM);
    std::string bottom = std::string(BOX_BL) + repeat(BOX_H, total_width - 2) + BOX_BR;

    if (m_console_output) {
        if (m_colored_output) {
            fmt::print("\n\033[36m{}\033[0m\n", top);
            fmt::print("\033[36m{}\033[0m\033[1;37m{:^{}}\033[0m\033[36m{}\033[0m\n",
                       BOX_V, title, total_width - 2, BOX_V);
            fmt::print("\033[36m{}\033[0m\n", title_sep);

            fmt::print("\033[36m{}\033[0m", BOX_V);
            for (size_t i = 0; i < columns.size() && i < widths.size(); ++i) {
                fmt::print("\033[1;37m {:^{}} \033[0m\033[36m{}\033[0m",
                           columns[i], widths[i], BOX_V);
            }
            fmt::print("\n");
            fmt::print("\033[36m{}\033[0m\n", header_sep);

            for (const auto& row : rows) {
                fmt::print("\033[36m{}\033[0m", BOX_V);
                for (size_t i = 0; i < row.size() && i < widths.size(); ++i) {
                    bool is_num = !row[i].empty() && (std::isdigit(row[i][0]) || row[i][0] == '-');
                    if (is_num) {
                        fmt::print("\033[33m {:>{}} \033[0m\033[36m{}\033[0m", row[i], widths[i], BOX_V);
                    } else {
                        fmt::print("\033[37m {:<{}} \033[0m\033[36m{}\033[0m", row[i], widths[i], BOX_V);
                    }
                }
                fmt::print("\n");
            }

            if (!footer.empty()) {
                fmt::print("\033[36m{}\033[0m\n", footer_sep);
                fmt::print("\033[36m{}\033[0m\033[1;32m {:<{}} \033[0m\033[36m{}\033[0m\n",
                           BOX_V, footer, total_width - 3, BOX_V);
            }
            fmt::print("\033[36m{}\033[0m\n", bottom);
        } else {
            fmt::print("\n{}\n", top);
            fmt::print("{}{:^{}}{}\n", BOX_V, title, total_width - 2, BOX_V);
            fmt::print("{}\n", title_sep);

            fmt::print("{}", BOX_V);
            for (size_t i = 0; i < columns.size() && i < widths.size(); ++i) {
                fmt::print(" {:^{}} {}", columns[i], widths[i], BOX_V);
            }
            fmt::print("\n{}\n", header_sep);

            for (const auto& row : rows) {
                fmt::print("{}", BOX_V);
                for (size_t i = 0; i < row.size() && i < widths.size(); ++i) {
                    bool is_num = !row[i].empty() && (std::isdigit(row[i][0]) || row[i][0] == '-');
                    if (is_num) {
                        fmt::print(" {:>{}} {}", row[i], widths[i], BOX_V);
                    } else {
                        fmt::print(" {:<{}} {}", row[i], widths[i], BOX_V);
                    }
                }
                fmt::print("\n");
            }

            if (!footer.empty()) {
                fmt::print("{}\n", footer_sep);
                fmt::print("{} {:<{}} {}\n", BOX_V, footer, total_width - 3, BOX_V);
            }
            fmt::print("{}\n", bottom);
        }
    }

    if (m_file_stream.is_open()) {
        write_to_file_raw("\n+" + std::string(total_width - 2, '-') + "+\n");
        write_to_file_raw(fmt::format("|{:^{}}|\n", title, total_width - 2));
        write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");

        std::string header_line = "|";
        for (size_t i = 0; i < columns.size() && i < widths.size(); ++i) {
            header_line += fmt::format(" {:^{}} |", columns[i], widths[i]);
        }
        write_to_file_raw(header_line + "\n");
        write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");

        for (const auto& row : rows) {
            std::string line = "|";
            for (size_t i = 0; i < row.size() && i < widths.size(); ++i) {
                bool is_num = !row[i].empty() && (std::isdigit(row[i][0]) || row[i][0] == '-');
                if (is_num) {
                    line += fmt::format(" {:>{}} |", row[i], widths[i]);
                } else {
                    line += fmt::format(" {:<{}} |", row[i], widths[i]);
                }
            }
            write_to_file_raw(line + "\n");
        }

        if (!footer.empty()) {
            write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");
            write_to_file_raw(fmt::format("| {:<{}} |\n", footer, total_width - 3));
        }
        write_to_file_raw("+" + std::string(total_width - 2, '-') + "+\n");
    }
}

auto LoggingService::print_run_metadata(
    const std::string& title,
    size_t total_runs,
    const std::vector<int>& runs
) -> void {
    std::lock_guard<std::mutex> lock(m_mutex);

    constexpr int WIDTH = 72;

    std::string top = std::string(BOX_TL) + repeat(BOX_H, WIDTH) + BOX_TR;
    std::string sep = std::string(BOX_LM) + repeat(BOX_H, WIDTH) + BOX_RM;
    std::string bottom = std::string(BOX_BL) + repeat(BOX_H, WIDTH) + BOX_BR;

    std::vector<std::string> run_lines;
    std::string current_line;
    int runs_per_line = 10;
    int count = 0;
    for (int run : runs) {
        if (count > 0) current_line += ", ";
        current_line += std::to_string(run);
        count++;
        if (count % runs_per_line == 0) {
            run_lines.push_back(current_line);
            current_line.clear();
        }
    }
    if (!current_line.empty()) {
        run_lines.push_back(current_line);
    }

    if (m_console_output) {
        if (m_colored_output) {
            fmt::print("\n\033[36m{}\033[0m\n", top);
            fmt::print("\033[36m{}\033[0m\033[1;37m {:^{}} \033[0m\033[36m{}\033[0m\n",
                       BOX_V, title, WIDTH - 2, BOX_V);
            fmt::print("\033[36m{}\033[0m\n", sep);
            fmt::print("\033[36m{}\033[0m\033[37m Total runs: \033[1;33m{:<{}}\033[0m\033[36m{}\033[0m\n",
                       BOX_V, total_runs, WIDTH - 13, BOX_V);
            fmt::print("\033[36m{}\033[0m\033[37m Runs: \033[0m", BOX_V);

            for (size_t i = 0; i < run_lines.size(); ++i) {
                if (i > 0) {
                    fmt::print("\033[36m{}\033[0m       ", BOX_V);
                }
                fmt::print("\033[33m{:<{}}\033[0m\033[36m{}\033[0m\n",
                           run_lines[i], WIDTH - 7, BOX_V);
            }
            fmt::print("\033[36m{}\033[0m\n", bottom);
        } else {
            fmt::print("\n{}\n", top);
            fmt::print("{} {:^{}} {}\n", BOX_V, title, WIDTH, BOX_V);
            fmt::print("{}\n", sep);
            fmt::print("{} Total runs: {:<{}} {}\n", BOX_V, total_runs, WIDTH - 13, BOX_V);

            for (size_t i = 0; i < run_lines.size(); ++i) {
                if (i == 0) {
                    fmt::print("{} Runs: {:<{}} {}\n", BOX_V, run_lines[i], WIDTH - 7, BOX_V);
                } else {
                    fmt::print("{}       {:<{}} {}\n", BOX_V, run_lines[i], WIDTH - 7, BOX_V);
                }
            }
            fmt::print("{}\n", bottom);
        }
    }

    if (m_file_stream.is_open()) {
        write_to_file_raw("\n+" + std::string(WIDTH, '-') + "+\n");
        write_to_file_raw(fmt::format("| {:^{}} |\n", title, WIDTH));
        write_to_file_raw("+" + std::string(WIDTH, '-') + "+\n");
        write_to_file_raw(fmt::format("| Total runs: {:<{}} |\n", total_runs, WIDTH - 13));

        for (size_t i = 0; i < run_lines.size(); ++i) {
            if (i == 0) {
                write_to_file_raw(fmt::format("| Runs: {:<{}} |\n", run_lines[i], WIDTH - 7));
            } else {
                write_to_file_raw(fmt::format("|       {:<{}} |\n", run_lines[i], WIDTH - 7));
            }
        }
        write_to_file_raw("+" + std::string(WIDTH, '-') + "+\n");
    }
}

auto LoggingService::format_timestamp() const -> std::string {
    if (!m_show_timestamps) return "";

    auto now = std::chrono::system_clock::now();
    auto time_t_v = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_v), "%H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();

    if (m_colored_output) {
        return fmt::format("\033[90m[{}]\033[0m ", ss.str());
    }
    return fmt::format("[{}] ", ss.str());
}

auto LoggingService::format_duration(double ms) -> std::string {
    if (ms < 1.0) {
        return fmt::format("{:.1f} \302\265s", ms * 1000);
    } else if (ms < 1000.0) {
        return fmt::format("{:.1f} ms", ms);
    } else if (ms < 60000.0) {
        return fmt::format("{:.2f} s", ms / 1000.0);
    } else {
        double minutes = ms / 60000.0;
        return fmt::format("{:.1f} min", minutes);
    }
}

auto LoggingService::effective_level(const std::string& source) const -> LogLevel {
    auto it = m_source_levels.find(source);
    if (it != m_source_levels.end()) {
        return it->second;
    }
    return m_min_level;
}

auto LoggingService::write_to_file(const std::string& level, const std::string& source,
                                   const std::string& message) -> void {
    if (m_file_stream.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time_t_v = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) % 1000;

        m_file_stream << "[" << std::put_time(std::localtime(&time_t_v), "%H:%M:%S")
                      << '.' << std::setfill('0') << std::setw(3) << ms.count() << "] "
                      << std::setw(5) << level << " "
                      << std::setw(16) << std::left << source << " "
                      << message << "\n";
        m_file_stream.flush();
    }
}

auto LoggingService::write_to_file_raw(const std::string& text) -> void {
    if (m_file_stream.is_open()) {
        m_file_stream << text;
        m_file_stream.flush();
    }
}

LoggingService::ScopedLog::ScopedLog(LoggingService* svc, std::string source, std::string operation)
    : m_svc(svc), m_source(std::move(source)), m_operation(std::move(operation)) {
    if (m_svc) m_svc->start(m_source, m_operation);
}

LoggingService::ScopedLog::~ScopedLog() {
    if (m_svc) m_svc->done(m_source, m_operation, m_timer.elapsed_ms());
}

}  // namespace framework
