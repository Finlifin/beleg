#include "ast/ast.hh"
#include <gtest/gtest.h>

class ASTTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // Setup code for each test
    }

    void TearDown() override {
        // Cleanup code for each test
    }
};

// Test basic AST construction
TEST_F(ASTTest, BasicConstruction) {
    Ast ast;

    // Test that AST starts with invalid node
    EXPECT_EQ(ast.root(), 0u);
    EXPECT_EQ(ast.nodes().size(), 1u);
    EXPECT_EQ(ast.spans().size(), 1u);

    // Test invalid node access
    EXPECT_FALSE(ast.get_node_kind(0).has_value());
    EXPECT_FALSE(ast.get_span(0).has_value());
    EXPECT_TRUE(ast.get_children(0).empty());
}

// Test node creation and retrieval
TEST_F(ASTTest, NodeCreation) {
    Ast ast;

    // Create a simple identifier node
    NodeBuilder builder(NodeKind::Id, Span(0, 3));
    NodeIndex id_node = ast.add_node(builder);

    EXPECT_EQ(id_node, 1u);
    EXPECT_EQ(ast.nodes().size(), 2u); // Invalid + new node

    // Test node retrieval
    auto node_info = ast.get_node(id_node);
    ASSERT_TRUE(node_info.has_value());

    auto [kind, span, children] = *node_info;
    EXPECT_EQ(kind, NodeKind::Id);
    EXPECT_EQ(span.start, 0u);
    EXPECT_EQ(span.end, 3u);
    EXPECT_TRUE(children.empty());
}

// Test node with children
TEST_F(ASTTest, NodeWithChildren) {
    Ast ast;

    // Create child nodes first
    NodeIndex left_child = ast.add_node(NodeBuilder(NodeKind::Int, Span(0, 1)));
    NodeIndex right_child = ast.add_node(NodeBuilder(NodeKind::Int, Span(2, 3)));

    // Create parent node with children
    NodeBuilder parent_builder(NodeKind::Add, Span(0, 3));
    parent_builder.add_single_child(left_child);
    parent_builder.add_single_child(right_child);

    NodeIndex parent_node = ast.add_node(parent_builder);

    // Test parent node
    auto parent_info = ast.get_node(parent_node);
    ASSERT_TRUE(parent_info.has_value());

    auto [kind, span, children] = *parent_info;
    EXPECT_EQ(kind, NodeKind::Add);
    EXPECT_EQ(children.size(), 2u);
    EXPECT_EQ(children[0], left_child);
    EXPECT_EQ(children[1], right_child);
}

// Test multiple children
TEST_F(ASTTest, MultipleChildren) {
    Ast ast;

    // Create parameter nodes
    std::vector<NodeIndex> params;
    params.push_back(ast.add_node(NodeBuilder(NodeKind::Id, Span(10, 11))));
    params.push_back(ast.add_node(NodeBuilder(NodeKind::Id, Span(13, 14))));

    // Create function node with multiple parameters
    NodeBuilder func_builder(NodeKind::FunctionDef, Span(0, 20));
    func_builder.add_single_child(
        ast.add_node(NodeBuilder(NodeKind::Id, Span(5, 8)))); // function name
    func_builder.add_multiple_children(params);               // parameters

    NodeIndex func_node = ast.add_node(func_builder);

    // Test function node
    auto func_info = ast.get_node(func_node);
    ASSERT_TRUE(func_info.has_value());

    auto [kind, span, children] = *func_info;
    EXPECT_EQ(kind, NodeKind::FunctionDef);
    EXPECT_EQ(children.size(), 2u); // name + params reference

    // Test parameter access
    auto params_slice = ast.get_multi_child_slice(children[1]);
    ASSERT_TRUE(params_slice.has_value());
    EXPECT_EQ(params_slice->size(), 2u);
    EXPECT_EQ((*params_slice)[0], params[0]);
    EXPECT_EQ((*params_slice)[1], params[1]);
}

// Test node type classification
TEST_F(ASTTest, NodeTypeClassification) {
    // Test various node kinds
    EXPECT_EQ(get_node_type(NodeKind::Id), NodeType::NoChild);
    EXPECT_EQ(get_node_type(NodeKind::Add), NodeType::DoubleChildren);
    EXPECT_EQ(get_node_type(NodeKind::Call), NodeType::SingleWithMultiChildren);
    EXPECT_EQ(get_node_type(NodeKind::FunctionDef), NodeType::FunctionDefChildren);
    EXPECT_EQ(get_node_type(NodeKind::Block), NodeType::MultiChildren);
}

// Test span operations
TEST_F(ASTTest, SpanOperations) {
    Span span(10, 20);

    EXPECT_TRUE(span.is_valid());
    EXPECT_EQ(span.len(), 10u);
    EXPECT_TRUE(span.contains(15));
    EXPECT_FALSE(span.contains(25));

    Span offset_span = span.with_offset(5);
    EXPECT_EQ(offset_span.start, 15u);
    EXPECT_EQ(offset_span.end, 25u);
}

// Test Child operations
TEST_F(ASTTest, ChildOperations) {
    Child single_child = Child::single(42);
    EXPECT_TRUE(single_child.is_single());
    EXPECT_FALSE(single_child.is_multiple());
    EXPECT_EQ(single_child.as_single(), 42u);

    std::vector<NodeIndex> multiple_indices = {1, 2, 3};
    Child multiple_child = Child::multiple(multiple_indices);
    EXPECT_FALSE(multiple_child.is_single());
    EXPECT_TRUE(multiple_child.is_multiple());

    auto multiple_span = multiple_child.as_multiple();
    EXPECT_EQ(multiple_span.size(), 3u);
    EXPECT_EQ(multiple_span[0], 1u);
    EXPECT_EQ(multiple_span[1], 2u);
    EXPECT_EQ(multiple_span[2], 3u);
}

// Test NodeBuilder operations
TEST_F(ASTTest, NodeBuilderOperations) {
    NodeBuilder builder(NodeKind::Add, Span(0, 5));

    EXPECT_EQ(builder.kind(), NodeKind::Add);
    EXPECT_EQ(builder.span().start, 0u);
    EXPECT_EQ(builder.span().end, 5u);
    EXPECT_TRUE(builder.children().empty());

    // Add children
    builder.add_single_child(1);
    builder.add_single_child(2);

    EXPECT_EQ(builder.children().size(), 2u);
    EXPECT_TRUE(builder.children()[0].is_single());
    EXPECT_TRUE(builder.children()[1].is_single());

    // Test fluent interface
    NodeBuilder fluent_builder = NodeBuilder(NodeKind::Mul, Span(10, 15))
                                     .add_single_child(3)
                                     .add_single_child(4)
                                     .with_span(Span(10, 20));

    EXPECT_EQ(fluent_builder.kind(), NodeKind::Mul);
    EXPECT_EQ(fluent_builder.span().start, 10u);
    EXPECT_EQ(fluent_builder.span().end, 20u);
    EXPECT_EQ(fluent_builder.children().size(), 2u);
}

// Test root node management
TEST_F(ASTTest, RootNodeManagement) {
    Ast ast;

    EXPECT_EQ(ast.root(), 0u);

    NodeIndex root_node = ast.add_node(NodeBuilder(NodeKind::FileScope, Span(0, 100)));
    ast.set_root(root_node);

    EXPECT_EQ(ast.root(), root_node);
}

// Test invalid node access
TEST_F(ASTTest, InvalidNodeAccess) {
    Ast ast;

    // Test out of bounds access
    EXPECT_FALSE(ast.get_node_kind(999).has_value());
    EXPECT_FALSE(ast.get_span(999).has_value());
    EXPECT_TRUE(ast.get_children(999).empty());

    // Test invalid multi-child slice
    EXPECT_FALSE(ast.get_multi_child_slice(999).has_value());
}

TEST_F(ASTTest, AnotherTestCase) {
    // TODO: Add more specific AST tests
    EXPECT_EQ(1, 1); // Placeholder test
}

// Main function is provided by gtest_main_dep, so no need to define it
