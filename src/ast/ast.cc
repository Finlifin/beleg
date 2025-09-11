#include "ast/ast.hh"

auto get_node_type(NodeKind kind) -> NodeType {
    using enum NodeKind;
    using enum NodeType;

    switch (kind) {
    // No children
    case Invalid:
    case Id:
    case Str:
    case Int:
    case Real:
    case Char:
    case Bool:
    case Unit:
    case Symbol:
    case SelfLower:
    case SelfCap:
    case Null:
    case ParamSelf:
    case ParamSelfRef:
    case RangeFull:
        return NoChild;

    // Single child
    case BoolNot:
    case OptionalType:
    case PointerType:
    case FunctionType:
    case RangeTo:
    case RangeToInclusive:
    case RangeFrom:
    case Deref:
    case Refer:
    case TypeCast:
    case ExprStatement:
    case PatternOptionSome:
    case PatternRangeTo:
    case PatternRangeToInclusive:
    case PatternRangeFrom:
    case ModStatement:
    case UseStatement:
    case PathSelectAll:
    case SuperPath:
    case PackagePath:
    case ReturnStatement:
    case BreakStatement:
    case ContinueStatement:
        return SingleChild;

    // Double children
    case RangeFromTo:
    case RangeFromToInclusive:
    case Add:
    case Sub:
    case Mul:
    case Div:
    case Mod:
    case AddAdd:
    case BoolEq:
    case BoolNotEq:
    case BoolAnd:
    case BoolOr:
    case BoolGt:
    case BoolGtEq:
    case BoolLt:
    case BoolLtEq:
    case Select:
    case Image:
    case IndexCall:
    case PatternArm:
    case ConditionArm:
    case CatchArm:
    case PatternRangeFromTo:
    case PatternRangeFromToInclusive:
    case PropertyPattern:
    case StructField:
    case UnionVariant:
    case PathSelect:
    case PathAsBind:
    case ParamTyped:
    case Assign:
    case AddAssign:
    case SubAssign:
    case MulAssign:
    case DivAssign:
        return DoubleChildren;

    // Triple children
    case ConstDecl:
    case LetDecl:
    case IfStatement:
    case WhileLoop:
    case PatternIfGuard:
    case PatternAsBind:
        return TripleChildren;

    // Quadruple children
    case ForLoop:
        return QuadrupleChildren;

    // Multi children
    case ListOf:
    case Tuple:
    case Object:
    case Block:
    case PatternRecord:
    case PatternList:
    case PatternTuple:
    case WhenStatement:
    case FileScope:
        return MultiChildren;

    // Single with multi children
    case Call:
    case ObjectCall:
    case PostMatch:
    case PatternObjectCall:
        return SingleWithMultiChildren;

    // Complex children patterns
    case FunctionDef:
        return FunctionDefChildren;
    case StructDef:
    case EnumDef:
    case UnionDef:
    case ModuleDef:
        return TypeDefChildren;
    case Typealias:
    case Newtype:
        return TypeAliasChildren;

    default:
        return NoChild;
    }
}