#ifndef SOURCE_MAP_HH
#define SOURCE_MAP_HH

#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include "common.hh"

// 文件 ID，用于唯一标识源文件
struct FileId {
    u32 id;

    FileId() : id(0) {
    }
    explicit FileId(u32 id) : id(id) {
    }

    bool operator==(const FileId& other) const {
        return id == other.id;
    }
    bool operator!=(const FileId& other) const {
        return id != other.id;
    }
    bool operator<(const FileId& other) const {
        return id < other.id;
    }
};

// 表示源码中的位置（行列信息）
struct Location {
    FileId file;
    u32 line;   // 1-based line number
    u32 column; // 0-based column number

    Location(FileId f, u32 l, u32 c) : file(f), line(l), column(c) {
    }

    bool operator==(const Location& other) const {
        return file == other.file && line == other.line
               && column == other.column;
    }
};

// 表示源码中的字节偏移范围
struct Span {
    u32 start;
    u32 end;

    Span() : start(0), end(0) {
    }
    Span(u32 s, u32 e) : start(s), end(e) {
    }

    auto is_valid() const -> bool {
        return start <= end;
    }

    auto len() const -> u32 {
        return end - start;
    }

    auto contains(u32 pos) const -> bool {
        return pos >= start && pos < end;
    }

    auto with_offset(u32 offset) const -> Span {
        return Span(start + offset, end + offset);
    }

    bool operator==(const Span& other) const {
        return start == other.start && end == other.end;
    }
};

// 源文件信息
struct SourceFile {
    String name;                  // 文件名或路径
    String content;               // 文件内容
    u32 start_pos;                // 在全局字节偏移中的起始位置
    std::vector<u32> line_starts; // 每行在文件中的字节偏移

    SourceFile(String name, String content, u32 start_pos);

    // 从字节偏移获取行列信息
    auto byte_pos_to_location(u32 byte_pos, FileId file_id) const -> Location;

    // 从行列信息获取字节偏移
    auto location_to_byte_pos(u32 line, u32 column) const -> std::optional<u32>;

    // 获取指定行的内容
    auto get_line(u32 line_number) const -> std::optional<std::string_view>;

    // 获取 span 对应的源码文本
    auto get_span_text(const Span& span) const
        -> std::optional<std::string_view>;

  private:
    auto compute_line_starts() -> void;
};

// 源码映射管理器
class SourceMap {
  private:
    std::vector<SourceFile> files;
    std::unordered_map<std::string, FileId> file_id_map;
    u32 next_start_pos = 0;

  public:
    SourceMap() = default;

    // 添加源文件
    auto add_file(const std::string& name, const std::string& content)
        -> FileId;

    // 从文件系统加载文件
    auto load_file(const std::string& path) -> std::optional<FileId>;

    // 根据文件ID获取源文件
    auto get_file(FileId file_id) const -> const SourceFile*;

    // 根据文件名获取文件ID
    auto get_file_id(const std::string& name) const -> std::optional<FileId>;

    // 从全局字节偏移获取位置信息
    auto lookup_location(u32 global_pos) const -> std::optional<Location>;

    // 从位置信息获取全局字节偏移
    auto lookup_byte_pos(const Location& loc) const -> std::optional<u32>;

    // 获取 span 对应的源码文本
    auto get_span_text(const Span& span) const -> std::optional<std::string>;

    // 获取指定位置的行内容
    auto get_line_at_location(const Location& loc) const
        -> std::optional<std::string_view>;

    // 创建一个新的 span
    auto make_span(FileId file_id,
                   u32 start_line,
                   u32 start_col,
                   u32 end_line,
                   u32 end_col) const -> Span;

    // 获取所有文件
    auto get_files() const -> const std::vector<SourceFile>& {
        return files;
    }

    // 格式化位置信息为字符串（用于错误报告）
    auto format_location(const Location& loc) const -> std::string;

    // 格式化 span 为字符串
    auto format_span(const Span& span) const -> std::optional<std::string>;
};

// Hash support for FileId (for unordered containers)
namespace std {
template <>
struct hash<FileId> {
    size_t operator()(const FileId& file_id) const {
        return hash<u32>()(file_id.id);
    }
};
} // namespace std

#endif