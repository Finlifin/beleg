#ifndef DIAG_HH
#define DIAG_HH

#include "common.hh"
#include "source_map/source_map.hh"
#include <memory>
#include <optional>
#include <iosfwd>

// 通常由各个pass管理，各个pass通过issue 生成diag
class Issue;
class DiagCtxt;
struct Diag;

/// 诊断上下文选项
struct DiagCtxtOptions {
    /// 最大错误数，超过后停止编译
    u32 max_errors = 100;
    /// 最大警告数，超过后停止报告警告
    u32 max_warnings = 1000;
    /// 是否启用颜色输出
    bool use_colors = true;
    /// 是否在第一个错误后停止
    bool abort_on_first_error = false;
    /// 默认的额外上下文行数
    u32 default_context_lines = 0;
};

// 消费diag
class DiagEmitter {
  public:
    virtual void emit(const Diag& diag) = 0;
    virtual ~DiagEmitter() = default;
};

class TerminalEmitter : public DiagEmitter {};

// 工厂函数声明
std::unique_ptr<TerminalEmitter> create_terminal_emitter(std::ostream& output,
                                                         bool use_colors = true,
                                                         bool use_unicode = true,
                                                         SourceMap* source_map = nullptr);

enum class DiagLevel { Note, Warning, Error, Fatal };

struct Label {
    Span span;
    String text;
    DiagLevel level = DiagLevel::Error;
    u32 surrounding_lines = 1;
};

struct Diag {
    DiagLevel level;
    std::optional<u32> error_code;
    String primary_message;
    Span primary_span;
    std::vector<Label> labels;
    std::vector<String> notes;
};

class DiagBuilder {
  private:
    Diag diag_;
    DiagCtxt* ctxt_;

  public:
    DiagBuilder(DiagCtxt* ctxt, DiagLevel level, const String& message, const Span& span)
        : ctxt_(ctxt) {
        diag_.level = level;
        diag_.primary_message = message;
        diag_.primary_span = span;
    }

    DiagBuilder& code(u32 error_code) {
        diag_.error_code = error_code;
        return *this;
    }

    DiagBuilder& label(const Span& span, const String& text, DiagLevel level = DiagLevel::Error) {
        diag_.labels.push_back({span, text, level});
        return *this;
    }

    DiagBuilder& note(const String& note) {
        diag_.notes.push_back(note);
        return *this;
    }

    DiagBuilder& span_label(const Span& span, const String& text) {
        return label(span, text, diag_.level);
    }

    void emit();
};

// 管理diag，提供diag builder, 向emitter提供diag
class DiagCtxt {
  private:
    DiagCtxtOptions options_;
    std::vector<std::unique_ptr<DiagEmitter>> emitters_;
    SourceMap* source_map_ = nullptr;

    u32 error_count_ = 0;
    u32 warning_count_ = 0;

  public:
    DiagCtxt() = default;
    explicit DiagCtxt(DiagCtxtOptions options) : options_(options) {
    }
    explicit DiagCtxt(DiagCtxtOptions options, SourceMap* source_map)
        : options_(options), source_map_(source_map) {
    }

    void add_emitter(std::unique_ptr<DiagEmitter> emitter) {
        emitters_.push_back(std::move(emitter));
    }

    void emit(const Diag& diag);

    bool can_emit(DiagLevel level) const;

    u32 error_count() const {
        return error_count_;
    }
    u32 warning_count() const {
        return warning_count_;
    }

    auto diag_builder(DiagLevel level, const String& primary_message, const Span& primary_span) {
        return DiagBuilder(this, level, primary_message, primary_span);
    }
};

#endif