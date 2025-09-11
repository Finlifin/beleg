#include "vfs.hh"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

// 构造函数
Vfs::Vfs(std::filesystem::path root_path) : root_path_(std::move(root_path)), next_node_id_(0) {
    // 创建根节点
    String root_name = root_path_.filename().string();
    VfsNode root_node(std::move(root_name), DirKind::Src);
    root_node_id_ = add_node(std::move(root_node));
}

// 从文件系统构建 VFS
auto Vfs::build_from_fs(std::string_view path) -> std::expected<Vfs, VfsError> {
    fs::path root_path(path);

    // 检查路径是否存在
    if (!fs::exists(root_path)) {
        return std::unexpected(
            VfsError(VfsErrorKind::PathNotFound, "Path does not exist: " + std::string(path)));
    }

    // 检查是否为目录
    if (!fs::is_directory(root_path)) {
        return std::unexpected(
            VfsError(VfsErrorKind::InvalidPath, "Path is not a directory: " + std::string(path)));
    }

    try {
        Vfs vfs(root_path);

        // 扫描根目录
        auto result = vfs.scan_directory(root_path, vfs.root_node_id_);
        if (!result) {
            return std::unexpected(std::move(result.error()));
        }

        // 构建路径映射
        vfs.build_path_mapping(vfs.root_node_id_, fs::path(""));

        return vfs;
    } catch (const std::exception& e) {
        return std::unexpected(
            VfsError(VfsErrorKind::FileSystemError, "File system error: " + std::string(e.what())));
    }
}

// 根据节点 ID 获取节点
auto Vfs::get_node(VfsNodeId node_id) const -> std::optional<const VfsNode*> {
    if (node_id >= nodes_.size()) {
        return std::nullopt;
    }
    return &nodes_[node_id];
}

auto Vfs::get_node_mut(VfsNodeId node_id) -> std::optional<VfsNode*> {
    if (node_id >= nodes_.size()) {
        return std::nullopt;
    }
    return &nodes_[node_id];
}

// 获取绝对路径
auto Vfs::get_absolute_path(VfsNodeId node_id) const -> std::optional<std::filesystem::path> {
    auto node = get_node(node_id);
    if (!node) {
        return std::nullopt;
    }

    // 构建路径
    std::vector<String> path_components;
    VfsNodeId current_id = node_id;

    while (true) {
        auto current_node = get_node(current_id);
        if (!current_node) {
            break;
        }

        path_components.push_back((*current_node)->name);

        if (!(*current_node)->parent) {
            break;
        }
        current_id = *(*current_node)->parent;
    }

    // 反转路径组件
    std::reverse(path_components.begin(), path_components.end());

    // 构建绝对路径
    fs::path abs_path = root_path_;
    for (size_t i = 1; i < path_components.size(); ++i) { // 跳过根节点名称
        abs_path /= path_components[i];
    }

    return abs_path;
}

// 获取项目内路径
auto Vfs::get_project_path(VfsNodeId node_id) const -> std::optional<std::filesystem::path> {
    auto node = get_node(node_id);
    if (!node) {
        return std::nullopt;
    }

    // 构建路径
    std::vector<String> path_components;
    VfsNodeId current_id = node_id;

    while (true) {
        auto current_node = get_node(current_id);
        if (!current_node) {
            break;
        }

        path_components.push_back((*current_node)->name);

        if (!(*current_node)->parent) {
            break;
        }
        current_id = *(*current_node)->parent;
    }

    // 反转路径组件
    std::reverse(path_components.begin(), path_components.end());

    // 构建项目内路径（不包含根路径）
    fs::path project_path;
    for (size_t i = 1; i < path_components.size(); ++i) { // 跳过根节点名称
        project_path /= path_components[i];
    }

    return project_path;
}

// 路径解析（向量形式）
auto Vfs::resolve(const std::vector<std::string_view>& path) const -> std::optional<VfsNodeId> {
    if (path.empty()) {
        return root_node_id_;
    }

    VfsNodeId current_id = root_node_id_;

    for (const auto& component : path) {
        auto current_node = get_node(current_id);
        if (!current_node || (*current_node)->type != VfsNodeType::Directory) {
            return std::nullopt;
        }

        // 在子节点中查找
        bool found = false;
        for (VfsNodeId child_id : (*current_node)->dir.children) {
            auto child_node = get_node(child_id);
            if (child_node && (*child_node)->name == component) {
                current_id = child_id;
                found = true;
                break;
            }
        }

        if (!found) {
            return std::nullopt;
        }
    }

    return current_id;
}

// 路径解析（字符串形式）
auto Vfs::resolve(std::string_view path) const -> std::optional<VfsNodeId> {
    if (path.empty()) {
        return root_node_id_;
    }

    // 分割路径
    std::vector<std::string_view> components;
    size_t start = 0;
    size_t pos = 0;

    while ((pos = path.find('/', start)) != std::string_view::npos) {
        if (pos > start) {
            components.push_back(path.substr(start, pos - start));
        }
        start = pos + 1;
    }

    if (start < path.length()) {
        components.push_back(path.substr(start));
    }

    return resolve(components);
}

// 获取文件的源文件 ID
auto Vfs::get_source_file_id(VfsNodeId node_id) const -> std::optional<FileId> {
    auto node = get_node(node_id);
    if (!node || (*node)->type != VfsNodeType::File) {
        return std::nullopt;
    }

    return (*node)->file.source_file_id;
}

// 设置文件的源文件 ID
auto Vfs::set_source_file_id(VfsNodeId node_id, FileId source_file_id) -> bool {
    auto node = get_node_mut(node_id);
    if (!node || (*node)->type != VfsNodeType::File) {
        return false;
    }

    (*node)->file.source_file_id = source_file_id;
    return true;
}

// 获取文件的 AST
auto Vfs::get_ast(VfsNodeId node_id) const -> std::optional<const Ast*> {
    auto node = get_node(node_id);
    if (!node || (*node)->type != VfsNodeType::File) {
        return std::nullopt;
    }

    if ((*node)->file.ast && *(*node)->file.ast) {
        return (*node)->file.ast->get();
    }
    return std::nullopt;
}

// 设置文件的 AST
auto Vfs::set_ast(VfsNodeId node_id, std::unique_ptr<Ast> ast) -> bool {
    auto node = get_node_mut(node_id);
    if (!node || (*node)->type != VfsNodeType::File) {
        return false;
    }

    (*node)->file.ast = std::move(ast);
    return true;
}

// 获取目录的入口文件
auto Vfs::get_entry_file(VfsNodeId dir_node_id) const -> std::optional<VfsNodeId> {
    auto dir_node = get_node(dir_node_id);
    if (!dir_node || (*dir_node)->type != VfsNodeType::Directory) {
        return std::nullopt;
    }

    // 根据目录类型确定入口文件名
    std::string entry_filename;
    switch ((*dir_node)->dir.kind) {
    case DirKind::Src:
        entry_filename = "main.bl";
        break;
    case DirKind::Normal:
        entry_filename = "mod.bl";
        break;
    default:
        // 其他类型的目录没有特定的入口文件
        return std::nullopt;
    }

    // 在目录的子节点中查找入口文件
    for (VfsNodeId child_id : (*dir_node)->dir.children) {
        auto child_node = get_node(child_id);
        if (child_node && (*child_node)->type == VfsNodeType::File &&
            (*child_node)->name == entry_filename) {
            return child_id;
        }
    }

    return std::nullopt;
}

// 获取目录的子节点
auto Vfs::get_children(VfsNodeId node_id) const -> std::optional<std::vector<VfsNodeId>> {
    auto node = get_node(node_id);
    if (!node || (*node)->type != VfsNodeType::Directory) {
        return std::nullopt;
    }

    return (*node)->dir.children;
}

// 检查是否为 Beleg 源文件
auto Vfs::is_beleg_source_file(const std::filesystem::path& path) -> bool {
    auto ext = path.extension().string();
    return ext == ".bl" || ext == ".beleg";
}

// 获取文件类型
auto Vfs::get_file_kind(const std::filesystem::path& path,
                        const std::filesystem::path& relative_path) -> FileKind {
    auto filename = path.filename().string();
    auto relative_str = relative_path.string();

    // 检查是否为 package.toml
    if (filename == "package.toml" && relative_path.parent_path().empty()) {
        return FileKind::PackageConfig;
    }

    // 检查是否为 main.bl
    if (filename == "main.bl" && relative_path.parent_path() == "src") {
        return FileKind::Main;
    }

    // 检查是否为 mod.bl
    if (filename == "mod.bl") {
        return FileKind::Mod;
    }

    // 检查是否为 Beleg 源文件
    if (is_beleg_source_file(path)) {
        return FileKind::Normal;
    }

    return FileKind::Other;
}

// 获取目录类型
auto Vfs::get_dir_kind(const std::filesystem::path& path,
                       const std::filesystem::path& relative_path) -> DirKind {
    auto dirname = path.filename().string();
    auto relative_str = relative_path.string();

    // 根据相对路径判断目录类型
    if (relative_path.empty()) {
        return DirKind::Src; // 根目录默认为 Src
    }

    if (relative_path == "src") {
        return DirKind::Src;
    }

    if (relative_path == "build") {
        return DirKind::Build;
    }

    if (relative_path == "examples") {
        return DirKind::Examples;
    }

    if (relative_path == "tests") {
        return DirKind::Tests;
    }

    if (relative_path == "docs") {
        return DirKind::Docs;
    }

    return DirKind::Normal;
}

// 添加节点
auto Vfs::add_node(VfsNode node) -> VfsNodeId {
    VfsNodeId node_id = next_node_id_++;
    nodes_.push_back(std::move(node));
    return node_id;
}

// 构建路径映射
auto Vfs::build_path_mapping(VfsNodeId node_id, const std::filesystem::path& current_path) -> void {
    auto node = get_node(node_id);
    if (!node) {
        return;
    }

    // 添加当前路径映射
    path_to_node_[current_path.string()] = node_id;

    // 如果是目录，递归处理子节点
    if ((*node)->type == VfsNodeType::Directory) {
        for (VfsNodeId child_id : (*node)->dir.children) {
            auto child_node = get_node(child_id);
            if (child_node) {
                fs::path child_path = current_path / (*child_node)->name;
                build_path_mapping(child_id, child_path);
            }
        }
    }
}

// 递归扫描目录
auto Vfs::scan_directory(const std::filesystem::path& dir_path, VfsNodeId parent_id)
    -> std::expected<void, VfsError> {
    try {
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            auto entry_path = entry.path();
            auto relative_path = fs::relative(entry_path, root_path_);
            String entry_name = entry_path.filename().string();

            if (entry.is_directory()) {
                // 创建目录节点
                auto dir_kind = get_dir_kind(entry_path, relative_path);
                VfsNode dir_node(std::move(entry_name), parent_id, dir_kind);
                VfsNodeId dir_id = add_node(std::move(dir_node));

                // 添加到父节点的子节点列表
                auto parent_node = get_node_mut(parent_id);
                if (parent_node && (*parent_node)->type == VfsNodeType::Directory) {
                    (*parent_node)->dir.children.push_back(dir_id);
                }

                // 递归扫描子目录
                auto result = scan_directory(entry_path, dir_id);
                if (!result) {
                    return result;
                }

            } else if (entry.is_regular_file()) {
                // 创建文件节点
                auto file_kind = get_file_kind(entry_path, relative_path);
                VfsNode file_node(std::move(entry_name), parent_id, file_kind);
                VfsNodeId file_id = add_node(std::move(file_node));

                // 添加到父节点的子节点列表
                auto parent_node = get_node_mut(parent_id);
                if (parent_node && (*parent_node)->type == VfsNodeType::Directory) {
                    (*parent_node)->dir.children.push_back(file_id);
                }
            }
        }

        return {};
    } catch (const std::exception& e) {
        return std::unexpected(
            VfsError(VfsErrorKind::FileSystemError,
                     "Error scanning directory " + dir_path.string() + ": " + e.what()));
    }
}
