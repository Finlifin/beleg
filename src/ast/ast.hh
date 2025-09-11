#ifndef AST_HH
#define AST_HH

#include "common.hh"
#include "source_map/source_map.hh"
#include <vector>
#include <span>
#include <optional>
#include <variant>

// Node index type, for future extensibility
using NodeIndex = u32;

/// Node kinds enumeration
enum class NodeKind : u32 {
    Invalid = 0,

    // Literals
    Id,
    Str,
    Int,
    Real,
    Char,
    Bool,
    Unit,
    Symbol,

    // Collections
    ListOf,
    Tuple,
    Object,

    // Unary operations
    BoolNot,
    SelfLower,
    SelfCap,
    Null,

    OptionalType,
    PointerType,
    FunctionType,

    // Ranges
    RangeFull,
    RangeTo,
    RangeToInclusive,
    RangeFrom,
    RangeFromTo,
    RangeFromToInclusive,

    // Binary operations
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    AddAdd,
    BoolEq,
    BoolNotEq,
    BoolAnd,
    BoolOr,
    BoolGt,
    BoolGtEq,
    BoolLt,
    BoolLtEq,

    Select,
    Image,

    // Dereference and reference
    Deref,
    Refer,
    TypeCast,

    // Calls
    Call,
    IndexCall,
    ObjectCall,

    // Pattern matching
    PostMatch,
    PatternArm,
    ConditionArm,
    CatchArm,

    // Statements
    ExprStatement,
    Assign,
    AddAssign,
    SubAssign,
    MulAssign,
    DivAssign,
    ConstDecl,
    LetDecl,
    ReturnStatement,
    BreakStatement,
    ContinueStatement,
    IfStatement,
    WhenStatement,
    WhileLoop,
    ForLoop,

    // Patterns
    PatternIfGuard,
    PatternAsBind,
    PatternOptionSome,
    PatternObjectCall,
    PatternRangeTo,
    PatternRangeToInclusive,
    PatternRangeFrom,
    PatternRangeFromTo,
    PatternRangeFromToInclusive,
    PropertyPattern,
    PatternRecord,
    PatternList,
    PatternTuple,

    // Items/Definitions
    FunctionDef,
    StructDef,
    StructField,
    EnumDef,
    EnumVariantWithPattern,
    UnionDef,
    UnionVariant,
    Typealias,
    Newtype,
    ModuleDef,

    // Imports
    ModStatement,
    UseStatement,
    PathSelect,
    PathSelectMulti,
    PathSelectAll,
    SuperPath,
    PackagePath,
    PathAsBind,

    // Parameters
    ParamTyped,
    ParamSelf,
    ParamSelfRef,

    Block,

    // Others
    FileScope,
};

/// Node type classification for structural information
enum class NodeType {
    NoChild,
    SingleChild,
    DoubleChildren,
    TripleChildren,
    QuadrupleChildren,
    MultiChildren,
    SingleWithMultiChildren,
    DoubleWithMultiChildren,
    TripleWithMultiChildren,
    FunctionDefChildren,
    DiamondFunctionDefChildren,
    EffectDefChildren,
    HandlesDefChildren,
    TypeDefChildren,
    TraitDefChildren,
    ImplTraitDefChildren,
    ExtendTraitDefChildren,
    DeriveDefChildren,
    TypeAliasChildren,
};

/// Child representation for node building
class Child {
  public:
    enum class Kind { Single, Multiple };

    static auto single(NodeIndex index) -> Child {
        return Child(Kind::Single, index);
    }

    static auto multiple(std::vector<NodeIndex> indices) -> Child {
        return Child(Kind::Multiple, std::move(indices));
    }

    auto is_single() const -> bool {
        return kind_ == Kind::Single;
    }

    auto is_multiple() const -> bool {
        return kind_ == Kind::Multiple;
    }

    auto as_single() const -> NodeIndex {
        return std::get<NodeIndex>(data_);
    }

    auto as_multiple() const -> std::span<const NodeIndex> {
        return std::span<const NodeIndex>(
            std::get<std::vector<NodeIndex>>(data_));
    }

  private:
    Kind kind_;
    std::variant<NodeIndex, std::vector<NodeIndex>> data_;

    Child(Kind kind, NodeIndex index) : kind_(kind), data_(index) {
    }
    Child(Kind kind, std::vector<NodeIndex> indices)
        : kind_(kind), data_(std::move(indices)) {
    }
};

/// Node builder for constructing AST nodes
class NodeBuilder {
  private:
    NodeKind kind_;
    Span span_;
    std::vector<Child> children_;

  public:
    NodeBuilder(NodeKind kind, Span span) : kind_(kind), span_(span) {
    }

    auto add_single_child(NodeIndex child) -> NodeBuilder& {
        children_.push_back(Child::single(child));
        return *this;
    }

    auto add_multiple_children(std::vector<NodeIndex> children)
        -> NodeBuilder& {
        children_.push_back(Child::multiple(std::move(children)));
        return *this;
    }

    auto with_span(Span span) -> NodeBuilder& {
        span_ = span;
        return *this;
    }

    auto with_children(std::vector<Child> children) -> NodeBuilder& {
        children_ = std::move(children);
        return *this;
    }

    auto with_node_kind(NodeKind kind) -> NodeBuilder& {
        kind_ = kind;
        return *this;
    }

    auto kind() const -> NodeKind {
        return kind_;
    }
    auto span() const -> Span {
        return span_;
    }
    auto children() const -> std::span<const Child> {
        return children_;
    }
};

/// Main AST structure
class Ast {
  private:
    // Node data - indexed by NodeIndex (starting from 1)
    std::vector<NodeKind> nodes_;
    std::vector<Span> spans_;
    std::vector<NodeIndex> children_start_;

    // Flattened children storage
    std::vector<NodeIndex> children_;

    NodeIndex root_;

  public:
    Ast() : root_(0) {
        // Initialize with invalid node at index 0
        nodes_.push_back(NodeKind::Invalid);
        spans_.push_back(Span());
        children_start_.push_back(0);
        children_.push_back(0); // Invalid child index
    }

    /// Add a node to the AST
    auto add_node(const NodeBuilder& builder) -> NodeIndex {
        // Process children
        std::vector<NodeIndex> child_indices;
        for (const auto& child : builder.children()) {
            if (child.is_single()) {
                child_indices.push_back(child.as_single());
            } else {
                // Store multiple children: first the count,
                // then the indices
                NodeIndex len_index  = children_.size();
                const auto& multiple = child.as_multiple();
                children_.push_back(multiple.size());
                for (auto idx : multiple) {
                    children_.push_back(idx);
                }
                child_indices.push_back(len_index);
            }
        }

        NodeIndex node_index         = nodes_.size();
        NodeIndex children_start_pos = children_.size();

        // Add child indices to flattened storage
        children_.insert(children_.end(),
                         child_indices.begin(),
                         child_indices.end());

        // Add node data
        nodes_.push_back(builder.kind());
        spans_.push_back(builder.span());
        children_start_.push_back(children_start_pos);

        return node_index;
    }

    /// Get children of a node
    auto get_children(NodeIndex node_index) const
        -> std::span<const NodeIndex> {
        if (node_index == 0 || node_index >= nodes_.size()) {
            return {};
        }

        usize start = children_start_[node_index];
        usize end   = (node_index + 1 < children_start_.size())
                          ? children_start_[node_index + 1]
                          : children_.size();

        return std::span<const NodeIndex>(children_.data() + start,
                                          end - start);
    }

    /// Get node kind
    auto get_node_kind(NodeIndex node_index) const -> std::optional<NodeKind> {
        if (node_index == 0 || node_index >= nodes_.size()) {
            return std::nullopt;
        }
        return nodes_[node_index];
    }

    /// Get complete node information
    auto get_node(NodeIndex node_index) const -> std::optional<
        std::tuple<NodeKind, Span, std::span<const NodeIndex>>> {
        if (node_index == 0 || node_index >= nodes_.size()) {
            return std::nullopt;
        }

        return std::make_tuple(nodes_[node_index],
                               spans_[node_index],
                               get_children(node_index));
    }

    /// Get span of a node
    auto get_span(NodeIndex node_index) const -> std::optional<Span> {
        if (node_index == 0 || node_index >= nodes_.size()) {
            return std::nullopt;
        }
        return spans_[node_index];
    }

    /// Get multiple children slice
    auto get_multi_child_slice(NodeIndex slice_len_index) const
        -> std::optional<std::span<const NodeIndex>> {
        if (slice_len_index == 0 || slice_len_index >= children_.size()) {
            return std::nullopt;
        }

        usize slice_start = slice_len_index;
        usize count       = children_[slice_start];
        usize data_start  = slice_start + 1;
        usize data_end    = data_start + count;

        if (data_end > children_.size()) {
            return std::nullopt;
        }

        return std::span<const NodeIndex>(children_.data() + data_start, count);
    }

    /// Set root node
    auto set_root(NodeIndex root) -> void {
        root_ = root;
    }

    /// Get root node
    auto root() const -> NodeIndex {
        return root_;
    }

    /// Get all nodes (for debugging/inspection)
    auto nodes() const -> std::span<const NodeKind> {
        return nodes_;
    }

    /// Get all spans
    auto spans() const -> std::span<const Span> {
        return spans_;
    }
};

/// Get node type classification for a node kind
auto get_node_type(NodeKind kind) -> NodeType;

#endif // AST_HH