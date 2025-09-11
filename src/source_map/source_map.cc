#include "source_map/source_map.hh"
#include <algorithm>
#include <sstream>
#include <fstream>

SourceFile::SourceFile(String name, String content, u32 start_pos)
    : name(std::move(name)), content(std::move(content)), start_pos(start_pos) {
    compute_line_starts();
}

auto SourceFile::compute_line_starts() -> void {
    line_starts.clear();
    line_starts.push_back(0); // First line starts at byte 0

    for (u32 i = 0; i < content.size(); ++i) {
        if (content[i] == '\n') {
            line_starts.push_back(i + 1);
        }
    }
}

auto SourceFile::byte_pos_to_location(u32 byte_pos, FileId file_id) const
    -> Location {
    if (byte_pos >= content.size()) {
        // Handle EOF position
        if (line_starts.empty()) {
            return Location(file_id, 1, 0);
        }
        u32 last_line       = static_cast<u32>(line_starts.size());
        u32 last_line_start = line_starts.back();
        u32 column = static_cast<u32>(content.size()) - last_line_start;
        return Location(file_id, last_line, column);
    }

    // Binary search to find the line
    auto it
        = std::upper_bound(line_starts.begin(), line_starts.end(), byte_pos);
    --it; // Move to the line start that's <= byte_pos

    u32 line_number = static_cast<u32>(it - line_starts.begin() + 1); // 1-based
    u32 column      = byte_pos - *it;                                 // 0-based

    return Location(file_id, line_number, column);
}

auto SourceFile::location_to_byte_pos(u32 line, u32 column) const
    -> std::optional<u32> {
    if (line == 0 || line > line_starts.size()) {
        return std::nullopt;
    }

    u32 line_start = line_starts[line - 1]; // Convert to 0-based
    u32 byte_pos   = line_start + column;

    // Check if the position is valid
    if (line < line_starts.size()) {
        // Not the last line, check against next line start
        u32 next_line_start = line_starts[line];
        if (byte_pos > next_line_start) {
            return std::nullopt;
        }
    } else {
        // Last line, check against content size
        if (byte_pos > content.size()) {
            return std::nullopt;
        }
    }

    return byte_pos;
}

auto SourceFile::get_line(u32 line_number) const
    -> std::optional<std::string_view> {
    if (line_number == 0 || line_number > line_starts.size()) {
        return std::nullopt;
    }

    u32 start = line_starts[line_number - 1];
    u32 end;

    if (line_number < line_starts.size()) {
        end = line_starts[line_number] - 1; // Exclude the newline
    } else {
        end = static_cast<u32>(content.size());
    }

    // Handle case where line ends with \n
    if (end > start && end <= content.size() && content[end - 1] == '\n') {
        end--;
    }

    return std::string_view(content.data() + start, end - start);
}

auto SourceFile::get_span_text(const Span& span) const
    -> std::optional<std::string_view> {
    if (!span.is_valid() || span.end > content.size()) {
        return std::nullopt;
    }
    return std::string_view(content.data() + span.start, span.len());
}

auto SourceMap::add_file(const std::string& name, const std::string& content)
    -> FileId {
    // Check if file already exists
    auto it = file_id_map.find(name);
    if (it != file_id_map.end()) {
        return it->second;
    }

    u32 file_id = static_cast<u32>(files.size());
    FileId fid(file_id);

    auto source_file = SourceFile(name, content, next_start_pos);
    files.push_back(std::move(source_file));
    file_id_map[name] = fid;

    next_start_pos += static_cast<u32>(content.size());

    return fid;
}

auto SourceMap::load_file(const std::string& path) -> std::optional<FileId> {
    // Check if file already loaded
    auto it = file_id_map.find(path);
    if (it != file_id_map.end()) {
        return it->second;
    }

    // Try to read file from filesystem
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt; // File doesn't exist or can't
                             // be opened
    }

    // Read file content
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // Add the file using existing add_file logic
    return add_file(path, content);
}

auto SourceMap::get_file(FileId file_id) const -> const SourceFile* {
    if (file_id.id >= files.size()) {
        return nullptr;
    }
    return &files[file_id.id];
}

auto SourceMap::get_file_id(const std::string& name) const
    -> std::optional<FileId> {
    auto it = file_id_map.find(name);
    if (it != file_id_map.end()) {
        return it->second;
    }
    return std::nullopt;
}

auto SourceMap::lookup_location(u32 global_pos) const
    -> std::optional<Location> {
    // Find which file contains this global position
    for (u32 i = 0; i < files.size(); ++i) {
        const auto& file = files[i];
        u32 file_end = file.start_pos + static_cast<u32>(file.content.size());

        if (global_pos >= file.start_pos && global_pos < file_end) {
            u32 local_pos = global_pos - file.start_pos;
            return file.byte_pos_to_location(local_pos, FileId(i));
        }
    }
    return std::nullopt;
}

auto SourceMap::lookup_byte_pos(const Location& loc) const
    -> std::optional<u32> {
    const SourceFile* file = get_file(loc.file);
    if (!file) {
        return std::nullopt;
    }

    auto local_pos = file->location_to_byte_pos(loc.line, loc.column);
    if (!local_pos) {
        return std::nullopt;
    }

    return file->start_pos + *local_pos;
}

auto SourceMap::get_span_text(const Span& span) const
    -> std::optional<std::string> {
    // Check if span is valid
    if (!span.is_valid()) {
        return std::nullopt;
    }

    // Find which file(s) contain this span - all of the
    // span must be covered
    std::string result;
    u32 covered_end = span.start; // Track how much we've covered

    for (const auto& file : files) {
        u32 file_end = file.start_pos + static_cast<u32>(file.content.size());

        // Check if this file overlaps with the uncovered
        // part of the span
        if (covered_end < file_end && span.end > file.start_pos
            && covered_end >= file.start_pos) {
            u32 local_start
                = std::max(covered_end, file.start_pos) - file.start_pos;
            u32 local_end = std::min(span.end, file_end) - file.start_pos;

            Span local_span(local_start, local_end);
            auto span_text = file.get_span_text(local_span);
            if (span_text) {
                result += *span_text;
                covered_end = file.start_pos + local_end;
            } else {
                return std::nullopt;
            }
        }
    }

    // Check if we covered the entire span
    return (covered_end >= span.end) ? std::optional<std::string>(result)
                                     : std::nullopt;
}

auto SourceMap::get_line_at_location(const Location& loc) const
    -> std::optional<std::string_view> {
    const SourceFile* file = get_file(loc.file);
    if (!file) {
        return std::nullopt;
    }

    return file->get_line(loc.line);
}

auto SourceMap::make_span(FileId file_id,
                          u32 start_line,
                          u32 start_col,
                          u32 end_line,
                          u32 end_col) const -> Span {
    const SourceFile* file = get_file(file_id);
    if (!file) {
        return Span();
    }

    auto start_pos = file->location_to_byte_pos(start_line, start_col);
    auto end_pos   = file->location_to_byte_pos(end_line, end_col);

    if (!start_pos || !end_pos) {
        return Span();
    }

    u32 global_start = file->start_pos + *start_pos;
    u32 global_end   = file->start_pos + *end_pos;

    return Span(global_start, global_end);
}

auto SourceMap::format_location(const Location& loc) const -> std::string {
    const SourceFile* file = get_file(loc.file);
    if (!file) {
        return "<unknown>";
    }

    std::ostringstream oss;
    oss << file->name << ":" << loc.line << ":"
        << (loc.column + 1); // Make column 1-based for display
    return oss.str();
}

auto SourceMap::format_span(const Span& span) const
    -> std::optional<std::string> {
    auto start_loc = lookup_location(span.start);
    auto end_loc   = lookup_location(span.end - 1); // end is exclusive

    if (!start_loc || !end_loc) {
        return std::nullopt;
    }

    std::ostringstream oss;
    if (start_loc->file == end_loc->file && start_loc->line == end_loc->line) {
        // Same line
        const SourceFile* file = get_file(start_loc->file);
        if (!file) {
            return std::nullopt;
        }
        oss << file->name << ":" << start_loc->line << ":"
            << (start_loc->column + 1) << "-" << (end_loc->column + 1);
    } else {
        // Multiple lines
        oss << format_location(*start_loc) << "-" << format_location(*end_loc);
    }

    return oss.str();
}