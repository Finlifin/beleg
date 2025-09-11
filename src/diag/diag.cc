#include "diag/diag.hh"

auto DiagCtxt::emit(const Diag& diag) -> void {
    // 检查是否可以发射此级别的诊断
    if (!can_emit(diag.level)) {
        return;
    }

    // 更新计数器
    switch (diag.level) {
    case DiagLevel::Error:
    case DiagLevel::Fatal:
        error_count_++;
        break;
    case DiagLevel::Warning:
        warning_count_++;
        break;
    case DiagLevel::Note:
        break;
    }

    // 向所有发射器发送诊断信息
    for (auto& emitter : emitters_) {
        emitter->emit(diag);
    }
}

auto DiagCtxt::can_emit(DiagLevel level) const -> bool {
    switch (level) {
    case DiagLevel::Error:
    case DiagLevel::Fatal:
        return error_count_ < options_.max_errors;
    case DiagLevel::Warning:
        return warning_count_ < options_.max_warnings;
    case DiagLevel::Note:
        return true;
    default:
        return true;
    }
}

auto DiagBuilder::emit() -> void {
    if (ctxt_) {
        ctxt_->emit(diag_);
    }
}