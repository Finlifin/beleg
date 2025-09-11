#include "diag/diag.hh"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cmath>

namespace {
/// Unicode字符常量
struct UnicodeChars {
    static constexpr const char* UNDERLINE = "─";
};

/// ASCII字符常量
struct AsciiChars {
    static constexpr const char* UNDERLINE = "-";
};

/// 终端颜色和样式
struct TerminalStyle {
    static constexpr const char* RESET         = "\x1b[0m";

    // 颜色定义
    static constexpr const char* BRIGHT_RED    = "\x1b[91m";
    static constexpr const char* BRIGHT_YELLOW = "\x1b[93m";
    static constexpr const char* BRIGHT_BLUE   = "\x1b[94m";

    /// 根据诊断级别获取样式
    static const char* get_style(DiagLevel level, bool use_colors) {
        if (!use_colors)
            return "";

        switch (level) {
        case DiagLevel::Fatal:
        case DiagLevel::Error:
            return BRIGHT_RED;
        case DiagLevel::Warning:
            return BRIGHT_YELLOW;
        case DiagLevel::Note:
            return BRIGHT_BLUE;
        default:
            return "";
        }
    }
};

/// 获取诊断级别的字符串表示
auto get_level_string(DiagLevel level) -> const char* {
    switch (level) {
    case DiagLevel::Fatal:
        return "Fatal";
    case DiagLevel::Error:
        return "Error";
    case DiagLevel::Warning:
        return "Warning";
    case DiagLevel::Note:
        return "Note";
    default:
        return "Unknown";
    }
}
} // namespace

/// 终端输出的诊断发射器实现
class TerminalEmitterImpl : public TerminalEmitter {
  private:
    std::ostream& output_;
    bool use_colors_;
    bool use_unicode_;
    SourceMap* source_map_;

  public:
    TerminalEmitterImpl(std::ostream& output,
                        bool use_colors,
                        bool use_unicode,
                        SourceMap* source_map)
        : output_(output), use_colors_(use_colors), use_unicode_(use_unicode),
          source_map_(source_map) {
    }

    auto emit(const Diag& diag) -> void override {
        // 输出主要错误信息
        render_header(diag);

        // 如果有标签，渲染源代码上下文
        if (!diag.labels.empty()) {
            render_labels(diag);
        }

        // 输出notes
        render_notes(diag);
    }

  private:
    /// 渲染诊断头部信息
    auto render_header(const Diag& diag) -> void {
        const char* style = TerminalStyle::get_style(diag.level, use_colors_);
        const char* reset = use_colors_ ? TerminalStyle::RESET : "";

        // [4002] error: Unresolved identifier: `Student`
        if (diag.error_code) {
            output_ << style << "[" << *diag.error_code << "] "
                    << get_level_string(diag.level) << ": "
                    << diag.primary_message << reset << "\n";
        } else {
            output_ << style << get_level_string(diag.level) << ": "
                    << diag.primary_message << reset << "\n";
        }
    }

    /// 渲染标签和源代码上下文
    auto render_labels(const Diag& diag) -> void {
        // 复制标签并按Span位置排序
        std::vector<Label> sorted_labels = diag.labels;
        std::sort(sorted_labels.begin(),
                  sorted_labels.end(),
                  [](const Label& a, const Label& b) {
                      return a.span.start < b.span.start;
                  });

        for (size_t i = 0; i < sorted_labels.size(); ++i) {
            render_label(sorted_labels[i], i == 0);
        }
    }

    /// 渲染单个标签
    auto render_label(const Label& label, bool is_primary) -> void {
        if (!source_map_)
            return;

        auto location = source_map_->lookup_location(label.span.start);
        if (!location)
            return;

        const auto* source_file = source_map_->get_file(location->file);
        if (!source_file)
            return;

        // 计算需要显示的行范围
        u32 start_line     = (location->line > label.surrounding_lines)
                                 ? location->line - label.surrounding_lines
                                 : 1;

        auto end_location  = source_map_->lookup_location(label.span.end);
        u32 end_line       = end_location
                                 ? end_location->line + label.surrounding_lines
                                 : location->line + label.surrounding_lines;

        // 计算最大行号的宽度，用于对齐
        u32 max_line_width = static_cast<u32>(std::log10(end_line)) + 1;

        // 渲染文件位置头部
        if (is_primary) {
            std::string line_spaces(max_line_width, ' ');

            if (use_unicode_) {
                output_ << " " << line_spaces << " ╭─[ " << source_file->name
                        << ":" << location->line << ":"
                        << (location->column + 1) << " ]\n";
            } else {
                output_ << " " << line_spaces << " +--[ " << source_file->name
                        << ":" << location->line << ":"
                        << (location->column + 1) << " ]\n";
            }

            // 分隔行
            render_empty_line(max_line_width);
        }

        // 渲染每一行
        for (u32 current_line = start_line; current_line <= end_line;
             ++current_line) {
            auto line_content = source_file->get_line(current_line);
            if (line_content) {
                render_source_line_with_width(current_line,
                                              *line_content,
                                              label,
                                              *location,
                                              end_location,
                                              max_line_width);
            }
        }

        // 渲染底部边框（仅主要标签）
        if (is_primary) {
            std::string line_spaces(max_line_width, ' ');

            if (use_unicode_) {
                output_ << " " << line_spaces << " ╰───\n";
            } else {
                output_ << " " << line_spaces << " ---+\n";
            }
        }
    }

    /// 渲染空行（用于分隔）
    auto render_empty_line(u32 line_width) -> void {
        std::string line_spaces(line_width, ' ');
        const char* line_prefix = use_unicode_ ? " │" : " |";
        output_ << " " << line_spaces << line_prefix << "\n";
    }

    /// 渲染源代码行（带宽度格式化）
    auto render_source_line_with_width(u32 line_num,
                                       std::string_view line_text,
                                       const Label& label,
                                       const Location& start_loc,
                                       const std::optional<Location>& end_loc,
                                       u32 line_width) -> void {
        const char* line_prefix = use_unicode_ ? " │ " : " | ";

        // 格式化行号，右对齐以保持统一宽度
        output_ << " " << std::setw(line_width) << line_num << line_prefix
                << line_text << "\n";

        // 如果是错误行，渲染下划线和指针
        if (line_num == start_loc.line) {
            render_underline_with_width(label, start_loc, end_loc, line_width);
        }
    }

    /// 渲染下划线和指针（带宽度格式化）
    auto render_underline_with_width(const Label& label,
                                     const Location& start_loc,
                                     const std::optional<Location>& end_loc,
                                     u32 line_width) -> void {
        const char* style = TerminalStyle::get_style(label.level, use_colors_);
        const char* reset = use_colors_ ? TerminalStyle::RESET : "";
        const char* line_prefix = use_unicode_ ? " │ " : " | ";

        std::string line_spaces(line_width, ' ');

        // 生成下划线
        output_ << " " << line_spaces << line_prefix;

        // 添加前导空格到错误位置
        for (u32 i = 0; i < start_loc.column; ++i) {
            output_ << " ";
        }

        // 渲染下划线
        const char* underline_char
            = use_unicode_ ? UnicodeChars::UNDERLINE : AsciiChars::UNDERLINE;
        const char* pointer_char
            = use_unicode_ ? UnicodeChars::UNDERLINE : AsciiChars::UNDERLINE;

        output_ << style;

        // 计算下划线长度
        u32 span_len = 1; // 默认长度
        if (end_loc && start_loc.line == end_loc->line) {
            span_len = (end_loc->column > start_loc.column)
                           ? end_loc->column - start_loc.column
                           : 1;
        }

        if (span_len == 0) {
            // 空span，显示插入点
            output_ << (use_unicode_ ? "│" : "|");
        } else if (span_len == 1) {
            // 单字符span
            output_ << pointer_char;
        } else {
            // 多字符span
            for (u32 i = 0; i < span_len; ++i) {
                if (i == span_len - 1) {
                    output_ << pointer_char;
                } else {
                    output_ << underline_char;
                }
            }
        }

        output_ << reset;

        // 添加标签消息
        if (!label.text.empty()) {
            output_ << " " << label.text;
        }
        output_ << "\n";
    }

    /// 渲染备注
    auto render_notes(const Diag& diag) -> void {
        const char* style
            = TerminalStyle::get_style(DiagLevel::Note, use_colors_);
        const char* reset = use_colors_ ? TerminalStyle::RESET : "";

        for (const auto& note : diag.notes) {
            output_ << style << "note" << reset << ": " << note << "\n";
        }
    }
};

// 工厂函数创建终端发射器
auto create_terminal_emitter(std::ostream& output,
                             bool use_colors,
                             bool use_unicode,
                             SourceMap* source_map)
    -> std::unique_ptr<TerminalEmitter> {
    auto emitter = std::make_unique<TerminalEmitterImpl>(output,
                                                         use_colors,
                                                         use_unicode,
                                                         source_map);
    return emitter;
}
