#ifndef VFS_HH
#define VFS_HH

#include "common.hh"
#include "source_map/source_map.hh"
#include "ast/ast.hh"
#include <vector>
#include <optional>
#include <unordered_map>
#include <string_view>
#include <filesystem>
#include <expected>

// VFS 节点 ID 类型
using VfsNodeId                         = u32;

// 无效的 VFS 节点 ID
constexpr VfsNodeId INVALID_VFS_NODE_ID = static_cast<VfsNodeId>(-1);

// 目录种类枚举
enum class DirKind {
    Normal,   // src 目录下所有的文件
    Src,      // 根目录下的 src/
    Build,    // 根目录下的 build/
    Examples, // 根目录下的 examples/
    Tests,    // 根目录下的 tests/
    Docs,     // 根目录下的 docs/
};

// 文件种类枚举
enum class FileKind {
    Normal,        // 普通 Beleg 源文件
    Main,          // src/ 下的 main.bl
    Mod,           // normal 目录下的 mod.bl
    PackageConfig, // 根目录下的 package.toml
    Other,         // 非 Beleg 源文件
};

// VFS 节点类型枚举
enum class VfsNodeType {
    Directory,
    File,
};

// 目录节点数据
struct DirNode {
    DirKind kind;
    std::vector<VfsNodeId> children;

    DirNode(DirKind k) : kind(k) {
    }
};

// 文件节点数据
struct FileNode {
    FileKind kind;
    std::optional<FileId> source_file_id;    // 惰性加载的源文件引用
    std::optional<std::unique_ptr<Ast>> ast; // 惰性加载的完整
                                             // AST 树

    FileNode(FileKind k) : kind(k) {
    }

    // 拷贝构造函数 - 不拷贝 AST（因为 unique_ptr 不能拷贝）
    FileNode(const FileNode& other)
        : kind(other.kind), source_file_id(other.source_file_id) {
        // 不拷贝 AST，保持为空
    }

    // 移动构造函数
    FileNode(FileNode&& other) noexcept
        : kind(other.kind), source_file_id(std::move(other.source_file_id)),
          ast(std::move(other.ast)) {
    }

    // 赋值操作符
    FileNode& operator=(const FileNode& other) {
        if (this != &other) {
            kind           = other.kind;
            source_file_id = other.source_file_id;
            // 不拷贝 AST
            ast.reset();
        }
        return *this;
    }

    // 移动赋值操作符
    FileNode& operator=(FileNode&& other) noexcept {
        if (this != &other) {
            kind           = std::move(other.kind);
            source_file_id = std::move(other.source_file_id);
            ast            = std::move(other.ast);
        }
        return *this;
    }
};

// VFS 节点
struct VfsNode {
    VfsNodeType type;
    String name;                     // 节点名称
    std::optional<VfsNodeId> parent; // 父节点 ID

    // 联合体存储节点特定数据
    union {
        DirNode dir;
        FileNode file;
    };

    // 构造函数
    VfsNode(String name, VfsNodeId parent_id, DirKind dir_kind)
        : type(VfsNodeType::Directory), name(std::move(name)),
          parent(parent_id), dir(dir_kind) {
    }

    VfsNode(String name, VfsNodeId parent_id, FileKind file_kind)
        : type(VfsNodeType::File), name(std::move(name)), parent(parent_id),
          file(file_kind) {
    }

    VfsNode(String name, DirKind dir_kind) // 根节点构造函数
        : type(VfsNodeType::Directory), name(std::move(name)),
          parent(std::nullopt), dir(dir_kind) {
    }

    // 析构函数
    ~VfsNode() {
        if (type == VfsNodeType::Directory) {
            dir.~DirNode();
        } else {
            file.~FileNode();
        }
    }

    // 拷贝构造函数
    VfsNode(const VfsNode& other)
        : type(other.type), name(other.name), parent(other.parent) {
        if (type == VfsNodeType::Directory) {
            new (&dir) DirNode(other.dir);
        } else {
            new (&file) FileNode(other.file); // FileNode
                                              // 现在有自己的拷贝构造函数
        }
    }

    // 移动构造函数
    VfsNode(VfsNode&& other) noexcept
        : type(other.type), name(std::move(other.name)), parent(other.parent) {
        if (type == VfsNodeType::Directory) {
            new (&dir) DirNode(std::move(other.dir));
        } else {
            new (&file)
                FileNode(std::move(other.file)); // FileNode
                                                 // 现在有自己的移动构造函数
        }
    }

    // 拷贝赋值操作符
    VfsNode& operator=(const VfsNode& other) {
        if (this != &other) {
            this->~VfsNode();          // 清理当前对象
            new (this) VfsNode(other); // 调用拷贝构造函数
        }
        return *this;
    }

    // 移动赋值操作符
    VfsNode& operator=(VfsNode&& other) noexcept {
        if (this != &other) {
            this->~VfsNode();                     // 清理当前对象
            new (this) VfsNode(std::move(other)); // 调用移动构造函数
        }
        return *this;
    }
};

// VFS 错误类型
enum class VfsErrorKind {
    PathNotFound,
    InvalidPath,
    FileSystemError,
    InvalidNodeType,
    NodeNotFound,
};

// VFS 错误
struct VfsError {
    VfsErrorKind kind;
    String message;

    VfsError(VfsErrorKind k, String msg) : kind(k), message(std::move(msg)) {
    }
};

// VFS 类
class Vfs {
  private:
    std::vector<VfsNode> nodes_;
    std::unordered_map<String, VfsNodeId> path_to_node_;
    std::filesystem::path root_path_;
    VfsNodeId root_node_id_;
    VfsNodeId next_node_id_;

  public:
    // 构造函数
    explicit Vfs(std::filesystem::path root_path);

    // 静态方法：从文件系统构建 VFS
    static auto build_from_fs(std::string_view path)
        -> std::expected<Vfs, VfsError>;

    // 获取根节点 ID
    auto root_node_id() const -> VfsNodeId {
        return root_node_id_;
    }

    // 根据节点 ID 获取节点
    auto get_node(VfsNodeId node_id) const -> std::optional<const VfsNode*>;
    auto get_node_mut(VfsNodeId node_id) -> std::optional<VfsNode*>;

    // 获取绝对路径
    auto get_absolute_path(VfsNodeId node_id) const
        -> std::optional<std::filesystem::path>;

    // 获取项目内路径
    auto get_project_path(VfsNodeId node_id) const
        -> std::optional<std::filesystem::path>;

    // 路径解析
    auto resolve(const std::vector<std::string_view>& path) const
        -> std::optional<VfsNodeId>;
    auto resolve(std::string_view path) const -> std::optional<VfsNodeId>;

    // 获取文件的源文件 ID
    auto get_source_file_id(VfsNodeId node_id) const -> std::optional<FileId>;

    // 设置文件的源文件 ID
    auto set_source_file_id(VfsNodeId node_id, FileId source_file_id) -> bool;

    // 获取文件的 AST
    auto get_ast(VfsNodeId node_id) const -> std::optional<const Ast*>;

    // 设置文件的 AST
    auto set_ast(VfsNodeId node_id, std::unique_ptr<Ast> ast) -> bool;

    // 获取目录的入口文件
    auto get_entry_file(VfsNodeId dir_node_id) const
        -> std::optional<VfsNodeId>;

    // 获取目录的子节点
    auto get_children(VfsNodeId node_id) const
        -> std::optional<std::vector<VfsNodeId>>;

    // 检查是否为 Beleg 源文件
    static auto is_beleg_source_file(const std::filesystem::path& path) -> bool;

    // 获取文件类型
    static auto get_file_kind(const std::filesystem::path& path,
                              const std::filesystem::path& relative_path)
        -> FileKind;

    // 获取目录类型
    static auto get_dir_kind(const std::filesystem::path& path,
                             const std::filesystem::path& relative_path)
        -> DirKind;

  private:
    // 添加节点
    auto add_node(VfsNode node) -> VfsNodeId;

    // 构建路径到节点的映射
    auto build_path_mapping(VfsNodeId node_id,
                            const std::filesystem::path& current_path) -> void;

    // 递归扫描目录
    auto scan_directory(const std::filesystem::path& dir_path,
                        VfsNodeId parent_id) -> std::expected<void, VfsError>;
};

#endif // VFS_HH
