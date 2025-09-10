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
        return file == other.file && line == other.line && column == other.column;
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

    bool is_valid() const {
        return start <= end;
    }

    u32 len() const {
        return end - start;
    }

    bool contains(u32 pos) const {
        return pos >= start && pos < end;
    }

    Span with_offset(u32 offset) const {
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
    Location byte_pos_to_location(u32 byte_pos, FileId file_id) const;

    // 从行列信息获取字节偏移
    std::optional<u32> location_to_byte_pos(u32 line, u32 column) const;

    // 获取指定行的内容
    std::optional<std::string_view> get_line(u32 line_number) const;

    // 获取 span 对应的源码文本
    std::optional<std::string_view> get_span_text(const Span& span) const;

  private:
    void compute_line_starts();
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
    FileId add_file(const std::string& name, const std::string& content);

    // 从文件系统加载文件
    std::optional<FileId> load_file(const std::string& path);

    // 根据文件ID获取源文件
    const SourceFile* get_file(FileId file_id) const;

    // 根据文件名获取文件ID
    std::optional<FileId> get_file_id(const std::string& name) const;

    // 从全局字节偏移获取位置信息
    std::optional<Location> lookup_location(u32 global_pos) const;

    // 从位置信息获取全局字节偏移
    std::optional<u32> lookup_byte_pos(const Location& loc) const;

    // 获取 span 对应的源码文本
    std::optional<std::string> get_span_text(const Span& span) const;

    // 获取指定位置的行内容
    std::optional<std::string_view> get_line_at_location(const Location& loc) const;

    // 创建一个新的 span
    Span make_span(FileId file_id, u32 start_line, u32 start_col, u32 end_line, u32 end_col) const;

    // 获取所有文件
    const std::vector<SourceFile>& get_files() const {
        return files;
    }

    // 格式化位置信息为字符串（用于错误报告）
    std::string format_location(const Location& loc) const;

    // 格式化 span 为字符串
    std::optional<std::string> format_span(const Span& span) const;
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