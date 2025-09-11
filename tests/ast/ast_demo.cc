#include "ast/ast.hh"
#include <iostream>

auto main() -> int {
    std::cout << "=== AST Module Functional Demo ===\n";

    Ast ast;

    // Create some basic nodes
    std::cout << "1. Creating basic AST nodes:\n";

    // Create integer literals
    NodeIndex int1 = ast.add_node(NodeBuilder(NodeKind::Int, Span(0, 1)));
    NodeIndex int2 = ast.add_node(NodeBuilder(NodeKind::Int, Span(4, 5)));

    std::cout << "   Created integer nodes: " << int1 << ", " << int2 << "\n";

    // Create an addition expression
    NodeBuilder add_builder(NodeKind::Add, Span(0, 5));
    add_builder.add_single_child(int1);
    add_builder.add_single_child(int2);
    NodeIndex add_expr = ast.add_node(add_builder);

    std::cout << "   Created addition expression: " << add_expr << "\n";

    // Create a variable declaration
    NodeIndex var_name = ast.add_node(NodeBuilder(NodeKind::Id, Span(10, 11)));
    NodeBuilder let_builder(NodeKind::LetDecl, Span(6, 5));
    let_builder.add_single_child(var_name); // variable name
    let_builder.add_single_child(add_expr); // initializer expression
    NodeIndex let_decl = ast.add_node(let_builder);

    std::cout << "   Created let declaration: " << let_decl << "\n";

    // Demonstrate node inspection
    std::cout << "\n2. Inspecting AST nodes:\n";

    auto add_info = ast.get_node(add_expr);
    if (add_info) {
        auto [kind, span, children] = *add_info;
        std::cout << "   Addition node: kind=" << static_cast<u32>(kind)
                  << ", span=[" << span.start << "," << span.end << "]"
                  << ", children=" << children.size() << "\n";

        for (usize i = 0; i < children.size(); ++i) {
            std::cout << "     Child " << i << ": " << children[i] << "\n";
        }
    }

    // Create a function with parameters
    std::cout << "\n3. Creating function with parameters:\n";

    // Function name
    NodeIndex func_name = ast.add_node(NodeBuilder(NodeKind::Id, Span(20, 23)));

    // Parameters
    std::vector<NodeIndex> params;
    params.push_back(
        ast.add_node(NodeBuilder(NodeKind::Id, Span(25, 26)))); // param a
    params.push_back(
        ast.add_node(NodeBuilder(NodeKind::Id, Span(28, 29)))); // param b

    // Function body (simple return statement)
    NodeIndex return_expr
        = ast.add_node(NodeBuilder(NodeKind::Add, Span(35, 39)));
    NodeBuilder return_builder(NodeKind::ReturnStatement, Span(32, 39));
    return_builder.add_single_child(return_expr);
    NodeIndex return_stmt = ast.add_node(return_builder);

    // Function definition
    NodeBuilder func_builder(NodeKind::FunctionDef, Span(15, 40));
    func_builder.add_single_child(func_name);   // function name
    func_builder.add_multiple_children(params); // parameters
    func_builder.add_single_child(return_stmt); // function body

    NodeIndex func_def = ast.add_node(func_builder);

    std::cout << "   Created function definition: " << func_def << "\n";

    // Inspect function parameters
    auto func_info = ast.get_node(func_def);
    if (func_info) {
        auto [kind, span, children] = *func_info;
        std::cout << "   Function has " << children.size()
                  << " direct children\n";

        // Access parameters (stored as multiple children)
        if (children.size() >= 2) {
            auto params_slice = ast.get_multi_child_slice(children[1]);
            if (params_slice) {
                std::cout << "   Function has " << params_slice->size()
                          << " parameters:\n";
                for (usize i = 0; i < params_slice->size(); ++i) {
                    std::cout << "     Parameter " << i << ": node "
                              << (*params_slice)[i] << "\n";
                }
            }
        }
    }

    // Demonstrate node type classification
    std::cout << "\n4. Node type classification:\n";
    std::cout << "   NodeKind::Add -> "
              << static_cast<u32>(get_node_type(NodeKind::Add)) << "\n";
    std::cout << "   NodeKind::FunctionDef -> "
              << static_cast<u32>(get_node_type(NodeKind::FunctionDef)) << "\n";
    std::cout << "   NodeKind::Id -> "
              << static_cast<u32>(get_node_type(NodeKind::Id)) << "\n";

    // Set root and show statistics
    ast.set_root(func_def);

    std::cout << "\n5. AST Statistics:\n";
    std::cout << "   Total nodes: " << ast.nodes().size() << "\n";
    std::cout << "   Root node: " << ast.root() << "\n";
    std::cout << "   Total children storage: " << ast.get_children(0).size()
              << " (flattened)\n";

    std::cout << "\nAST demo completed successfully!\n";
    return 0;
}
