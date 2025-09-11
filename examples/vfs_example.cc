#include "vfs/vfs.hh"
#include "ast/ast.hh"
#include <iostream>

int main() {
    // 构建当前目录的 VFS
    auto vfs_result = Vfs::build_from_fs(".");
    if (!vfs_result) {
        std::cerr << "Failed to build VFS: " << vfs_result.error().message << std::endl;
        return 1;
    }

    auto& vfs = vfs_result.value();

    std::cout << "VFS built successfully!" << std::endl;

    // 解析一些路径
    auto src_id = vfs.resolve("src");
    if (src_id) {
        std::cout << "Found src directory" << std::endl;

        // 获取子节点
        auto children = vfs.get_children(*src_id);
        if (children) {
            std::cout << "src directory has " << children->size() << " children:" << std::endl;
            for (auto child_id : *children) {
                auto child_node = vfs.get_node(child_id);
                if (child_node) {
                    std::cout << "  - " << (*child_node)->name;
                    if ((*child_node)->type == VfsNodeType::Directory) {
                        std::cout << " (directory)";
                    } else {
                        std::cout << " (file)";
                    }
                    std::cout << std::endl;
                }
            }
        }

        // 获取入口文件
        auto entry_file = vfs.get_entry_file(*src_id);
        if (entry_file) {
            auto entry_node = vfs.get_node(*entry_file);
            if (entry_node) {
                std::cout << "Entry file for src: " << (*entry_node)->name << std::endl;
            }
        }
    }

    // 查找 main.bl
    auto main_id = vfs.resolve("src/main.bl");
    if (main_id) {
        std::cout << "Found main.bl" << std::endl;

        // 获取绝对路径
        auto abs_path = vfs.get_absolute_path(*main_id);
        if (abs_path) {
            std::cout << "Absolute path: " << abs_path->string() << std::endl;
        }

        // 获取项目内路径
        auto project_path = vfs.get_project_path(*main_id);
        if (project_path) {
            std::cout << "Project path: " << project_path->string() << std::endl;
        }

        // 演示 AST 管理
        auto ast = std::make_unique<Ast>();
        NodeIndex root_node = ast->add_node(NodeBuilder(NodeKind::Int, Span(0, 1)));
        ast->set_root(root_node);

        if (vfs.set_ast(*main_id, std::move(ast))) {
            std::cout << "AST set successfully for main.bl" << std::endl;

            auto retrieved_ast = vfs.get_ast(*main_id);
            if (retrieved_ast) {
                std::cout << "Retrieved AST from main.bl" << std::endl;
            }
        }
    }

    return 0;
}
