#include <gtest/gtest.h>
#include "vfs/vfs.hh"
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

class VfsTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // 创建临时测试目录结构
        test_dir = fs::temp_directory_path() / "vfs_test";
        fs::remove_all(test_dir);
        fs::create_directories(test_dir);

        // 创建测试文件结构
        fs::create_directories(test_dir / "src");
        fs::create_directories(test_dir / "examples");
        fs::create_directories(test_dir / "tests");
        fs::create_directories(test_dir / "build");
        fs::create_directories(test_dir / "src" / "utils");

        // 创建文件
        create_file(test_dir / "package.toml", "[package]\nname = \"test\"\n");
        create_file(test_dir / "src" / "main.bl", "fn main() {}\n");
        create_file(test_dir / "src" / "lib.bl", "pub fn hello() {}\n");
        create_file(test_dir / "src" / "utils" / "mod.bl", "pub mod helper;\n");
        create_file(test_dir / "src" / "utils" / "helper.bl",
                    "pub fn help() {}\n");
        create_file(test_dir / "examples" / "example1.bl", "use lib::hello;\n");
        create_file(test_dir / "README.md", "# Test Project\n");
    }

    void TearDown() override {
        fs::remove_all(test_dir);
    }

    void create_file(const fs::path& path, const std::string& content) {
        std::ofstream file(path);
        file << content;
    }

    fs::path test_dir;
};

TEST_F(VfsTest, BuildFromFs) {
    auto vfs_result = Vfs::build_from_fs(test_dir.string());
    ASSERT_TRUE(vfs_result.has_value())
        << "Failed to build VFS from filesystem";

    auto& vfs    = vfs_result.value();

    // 测试根节点
    auto root_id = vfs.root_node_id();
    EXPECT_NE(root_id, INVALID_VFS_NODE_ID);

    auto root_node = vfs.get_node(root_id);
    ASSERT_TRUE(root_node.has_value());
    EXPECT_EQ((*root_node)->type, VfsNodeType::Directory);
    EXPECT_EQ((*root_node)->dir.kind, DirKind::Src);
}

TEST_F(VfsTest, PathResolution) {
    auto vfs_result = Vfs::build_from_fs(test_dir.string());
    ASSERT_TRUE(vfs_result.has_value());

    auto& vfs   = vfs_result.value();

    // 测试路径解析
    auto src_id = vfs.resolve("src");
    ASSERT_TRUE(src_id.has_value());

    auto src_node = vfs.get_node(*src_id);
    ASSERT_TRUE(src_node.has_value());
    EXPECT_EQ((*src_node)->type, VfsNodeType::Directory);
    EXPECT_EQ((*src_node)->dir.kind, DirKind::Src);

    // 测试嵌套路径解析
    auto main_id = vfs.resolve("src/main.bl");
    ASSERT_TRUE(main_id.has_value());

    auto main_node = vfs.get_node(*main_id);
    ASSERT_TRUE(main_node.has_value());
    EXPECT_EQ((*main_node)->type, VfsNodeType::File);
    EXPECT_EQ((*main_node)->file.kind, FileKind::Main);

    // 测试向量形式的路径解析
    std::vector<std::string_view> path_components = {"src", "utils", "mod.bl"};
    auto mod_id = vfs.resolve(path_components);
    ASSERT_TRUE(mod_id.has_value());

    auto mod_node = vfs.get_node(*mod_id);
    ASSERT_TRUE(mod_node.has_value());
    EXPECT_EQ((*mod_node)->type, VfsNodeType::File);
    EXPECT_EQ((*mod_node)->file.kind, FileKind::Mod);
}

TEST_F(VfsTest, FileKindDetection) {
    auto vfs_result = Vfs::build_from_fs(test_dir.string());
    ASSERT_TRUE(vfs_result.has_value());

    auto& vfs       = vfs_result.value();

    // 测试 package.toml
    auto package_id = vfs.resolve("package.toml");
    ASSERT_TRUE(package_id.has_value());
    auto package_node = vfs.get_node(*package_id);
    ASSERT_TRUE(package_node.has_value());
    EXPECT_EQ((*package_node)->file.kind, FileKind::PackageConfig);

    // 测试 main.bl
    auto main_id = vfs.resolve("src/main.bl");
    ASSERT_TRUE(main_id.has_value());
    auto main_node = vfs.get_node(*main_id);
    ASSERT_TRUE(main_node.has_value());
    EXPECT_EQ((*main_node)->file.kind, FileKind::Main);

    // 测试 mod.bl
    auto mod_id = vfs.resolve("src/utils/mod.bl");
    ASSERT_TRUE(mod_id.has_value());
    auto mod_node = vfs.get_node(*mod_id);
    ASSERT_TRUE(mod_node.has_value());
    EXPECT_EQ((*mod_node)->file.kind, FileKind::Mod);

    // 测试普通文件
    auto lib_id = vfs.resolve("src/lib.bl");
    ASSERT_TRUE(lib_id.has_value());
    auto lib_node = vfs.get_node(*lib_id);
    ASSERT_TRUE(lib_node.has_value());
    EXPECT_EQ((*lib_node)->file.kind, FileKind::Normal);

    // 测试其他文件
    auto readme_id = vfs.resolve("README.md");
    ASSERT_TRUE(readme_id.has_value());
    auto readme_node = vfs.get_node(*readme_id);
    ASSERT_TRUE(readme_node.has_value());
    EXPECT_EQ((*readme_node)->file.kind, FileKind::Other);
}

TEST_F(VfsTest, DirKindDetection) {
    auto vfs_result = Vfs::build_from_fs(test_dir.string());
    ASSERT_TRUE(vfs_result.has_value());

    auto& vfs   = vfs_result.value();

    // 测试 src 目录
    auto src_id = vfs.resolve("src");
    ASSERT_TRUE(src_id.has_value());
    auto src_node = vfs.get_node(*src_id);
    ASSERT_TRUE(src_node.has_value());
    EXPECT_EQ((*src_node)->dir.kind, DirKind::Src);

    // 测试 examples 目录
    auto examples_id = vfs.resolve("examples");
    ASSERT_TRUE(examples_id.has_value());
    auto examples_node = vfs.get_node(*examples_id);
    ASSERT_TRUE(examples_node.has_value());
    EXPECT_EQ((*examples_node)->dir.kind, DirKind::Examples);

    // 测试 tests 目录
    auto tests_id = vfs.resolve("tests");
    ASSERT_TRUE(tests_id.has_value());
    auto tests_node = vfs.get_node(*tests_id);
    ASSERT_TRUE(tests_node.has_value());
    EXPECT_EQ((*tests_node)->dir.kind, DirKind::Tests);

    // 测试 build 目录
    auto build_id = vfs.resolve("build");
    ASSERT_TRUE(build_id.has_value());
    auto build_node = vfs.get_node(*build_id);
    ASSERT_TRUE(build_node.has_value());
    EXPECT_EQ((*build_node)->dir.kind, DirKind::Build);

    // 测试普通目录
    auto utils_id = vfs.resolve("src/utils");
    ASSERT_TRUE(utils_id.has_value());
    auto utils_node = vfs.get_node(*utils_id);
    ASSERT_TRUE(utils_node.has_value());
    EXPECT_EQ((*utils_node)->dir.kind, DirKind::Normal);
}

TEST_F(VfsTest, PathGeneration) {
    auto vfs_result = Vfs::build_from_fs(test_dir.string());
    ASSERT_TRUE(vfs_result.has_value());

    auto& vfs    = vfs_result.value();

    // 测试绝对路径生成
    auto main_id = vfs.resolve("src/main.bl");
    ASSERT_TRUE(main_id.has_value());

    auto abs_path = vfs.get_absolute_path(*main_id);
    ASSERT_TRUE(abs_path.has_value());
    EXPECT_EQ(*abs_path, test_dir / "src" / "main.bl");

    // 测试项目内路径生成
    auto project_path = vfs.get_project_path(*main_id);
    ASSERT_TRUE(project_path.has_value());
    EXPECT_EQ(*project_path, fs::path("src") / "main.bl");
}

TEST_F(VfsTest, Children) {
    auto vfs_result = Vfs::build_from_fs(test_dir.string());
    ASSERT_TRUE(vfs_result.has_value());

    auto& vfs   = vfs_result.value();

    // 测试获取子节点
    auto src_id = vfs.resolve("src");
    ASSERT_TRUE(src_id.has_value());

    auto children = vfs.get_children(*src_id);
    ASSERT_TRUE(children.has_value());
    EXPECT_GE(children->size(),
              3); // 至少包含 main.bl, lib.bl, utils/

    // 验证子节点名称
    std::vector<String> child_names;
    for (auto child_id : *children) {
        auto child_node = vfs.get_node(child_id);
        ASSERT_TRUE(child_node.has_value());
        child_names.push_back((*child_node)->name);
    }

    EXPECT_TRUE(std::find(child_names.begin(), child_names.end(), "main.bl")
                != child_names.end());
    EXPECT_TRUE(std::find(child_names.begin(), child_names.end(), "lib.bl")
                != child_names.end());
    EXPECT_TRUE(std::find(child_names.begin(), child_names.end(), "utils")
                != child_names.end());
}

TEST_F(VfsTest, SourceFileAndAstManagement) {
    auto vfs_result = Vfs::build_from_fs(test_dir.string());
    ASSERT_TRUE(vfs_result.has_value());

    auto& vfs    = vfs_result.value();

    auto main_id = vfs.resolve("src/main.bl");
    ASSERT_TRUE(main_id.has_value());

    // 测试初始状态（应该没有源文件 ID 和 AST）
    EXPECT_FALSE(vfs.get_source_file_id(*main_id).has_value());
    EXPECT_FALSE(vfs.get_ast(*main_id).has_value());

    // 设置源文件 ID
    FileId file_id(42);
    EXPECT_TRUE(vfs.set_source_file_id(*main_id, file_id));
    auto retrieved_file_id = vfs.get_source_file_id(*main_id);
    ASSERT_TRUE(retrieved_file_id.has_value());
    EXPECT_EQ(retrieved_file_id->id, 42);

    // 创建并设置 AST
    auto ast            = std::make_unique<Ast>();
    NodeIndex root_node = ast->add_node(NodeBuilder(NodeKind::Int, Span(0, 1)));
    ast->set_root(root_node);

    EXPECT_TRUE(vfs.set_ast(*main_id, std::move(ast)));
    auto retrieved_ast = vfs.get_ast(*main_id);
    ASSERT_TRUE(retrieved_ast.has_value());
    EXPECT_EQ((*retrieved_ast)->root(), root_node);
}

TEST_F(VfsTest, GetEntryFile) {
    auto vfs_result = Vfs::build_from_fs(test_dir.string());
    ASSERT_TRUE(vfs_result.has_value());

    auto& vfs   = vfs_result.value();

    // 测试 src 目录的入口文件（应该是 main.bl）
    auto src_id = vfs.resolve("src");
    ASSERT_TRUE(src_id.has_value());
    auto entry_file = vfs.get_entry_file(*src_id);
    ASSERT_TRUE(entry_file.has_value());

    auto entry_node = vfs.get_node(*entry_file);
    ASSERT_TRUE(entry_node.has_value());
    EXPECT_EQ((*entry_node)->name, "main.bl");
    EXPECT_EQ((*entry_node)->file.kind, FileKind::Main);

    // 测试 src/utils 目录的入口文件（应该是 mod.bl）
    auto utils_id = vfs.resolve("src/utils");
    ASSERT_TRUE(utils_id.has_value());
    auto utils_entry = vfs.get_entry_file(*utils_id);
    ASSERT_TRUE(utils_entry.has_value());

    auto utils_entry_node = vfs.get_node(*utils_entry);
    ASSERT_TRUE(utils_entry_node.has_value());
    EXPECT_EQ((*utils_entry_node)->name, "mod.bl");
    EXPECT_EQ((*utils_entry_node)->file.kind, FileKind::Mod);

    // 测试 build 目录（没有入口文件）
    auto build_id = vfs.resolve("build");
    ASSERT_TRUE(build_id.has_value());
    auto build_entry = vfs.get_entry_file(*build_id);
    EXPECT_FALSE(build_entry.has_value());

    // 测试不存在的目录
    auto invalid_entry = vfs.get_entry_file(999999);
    EXPECT_FALSE(invalid_entry.has_value());
}

TEST_F(VfsTest, ErrorHandling) {
    // 测试不存在的路径
    auto vfs_result = Vfs::build_from_fs("/nonexistent/path");
    EXPECT_FALSE(vfs_result.has_value());
    EXPECT_EQ(vfs_result.error().kind, VfsErrorKind::PathNotFound);

    // 测试有效的 VFS 上的无效操作
    auto valid_vfs = Vfs::build_from_fs(test_dir.string());
    ASSERT_TRUE(valid_vfs.has_value());

    auto& vfs       = valid_vfs.value();

    // 测试不存在的路径解析
    auto invalid_id = vfs.resolve("nonexistent/path");
    EXPECT_FALSE(invalid_id.has_value());

    // 测试无效节点 ID
    auto invalid_node = vfs.get_node(999999);
    EXPECT_FALSE(invalid_node.has_value());

    // 测试在文件节点上调用目录操作
    auto main_id = vfs.resolve("src/main.bl");
    ASSERT_TRUE(main_id.has_value());
    auto children = vfs.get_children(*main_id);
    EXPECT_FALSE(children.has_value());
}
