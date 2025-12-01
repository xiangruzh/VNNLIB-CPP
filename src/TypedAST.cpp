#include "TypedAST.h"  

// ----------- Utility Functions ----------

std::string dtypeToString(TDataType dt) {
  switch (dt) {
    case TDataType::Real: return "Real";
    case TDataType::F16: return "F16";
    case TDataType::F32: return "F32";
    case TDataType::F64: return "F64";
    case TDataType::BF16: return "BF16";
    case TDataType::F8E4M3FN: return "F8E4M3FN";
    case TDataType::F8E5M2: return "F8E5M2";
    case TDataType::F8E4M3FNUZ: return "F8E4M3FNUZ";
    case TDataType::F8E5M2FNUZ: return "F8E5M2FNUZ";
    case TDataType::F4E2M1: return "F4E2M1";
    case TDataType::I8: return "I8";
    case TDataType::I16: return "I16";
    case TDataType::I32: return "I32";
    case TDataType::I64: return "I64";
    case TDataType::U8: return "U8";
    case TDataType::U16: return "U16";
    case TDataType::U32: return "U32";
    case TDataType::U64: return "U64";
    case TDataType::C64: return "C64";
    case TDataType::C128: return "C128";
    case TDataType::Bool: return "Bool";
    case TDataType::String: return "String";
    case TDataType::FloatConstant: return "FloatConstant";
    case TDataType::NegativeIntConstant: return "NegativeIntConstant";
    case TDataType::PositiveIntConstant: return "PositiveIntConstant";
    default: return "Unknown";
  }
}

bool isConstant(TDataType dt) {
    return dt == TDataType::FloatConstant || dt == TDataType::NegativeIntConstant || dt == TDataType::PositiveIntConstant;
}

// Returns true if the data type of the expression is in the same family as a constant data type
bool sameFamily(TDataType exprType, TDataType constType) {
    if (isConstant(constType)) {
        switch (exprType) {
            case TDataType::Real:
            case TDataType::F16:
            case TDataType::F32:
            case TDataType::F64:
            case TDataType::BF16:
            case TDataType::F8E4M3FN:
            case TDataType::F8E5M2:
            case TDataType::F8E4M3FNUZ:
            case TDataType::F8E5M2FNUZ:
            case TDataType::F4E2M1:
              return constType == TDataType::FloatConstant;
            case TDataType::I8:
            case TDataType::I16:
            case TDataType::I32:
            case TDataType::I64:
              return constType == TDataType::NegativeIntConstant || constType == TDataType::PositiveIntConstant;
            case TDataType::U8:
            case TDataType::U16:
            case TDataType::U32:
            case TDataType::U64:
              return constType == TDataType::PositiveIntConstant;
            case TDataType::FloatConstant:
              return constType == TDataType::FloatConstant;
            case TDataType::NegativeIntConstant:
            case TDataType::PositiveIntConstant:
              return constType == TDataType::NegativeIntConstant || constType == TDataType::PositiveIntConstant;
            default:
                return false;
        }
    }
    return false; // If constType is not a constant data type
}

bool sameType(TDataType a, TDataType b) {
    return a == b;
}

std::string shapeToString(const Shape& s) {
	if (s.empty()) return "[]";
	std::ostringstream oss;
	oss << '[';
	for (size_t i = 0; i < s.size(); ++i) {
	if (i) oss << ',';
	oss << s[i];
	}
	oss << ']';
	return oss.str();
}

template <class T>
std::string bnfcPrint(const T* p) {
	if (!p) return "<null>";
	PrintAbsyn pr;
	return pr.print(const_cast<T*>(p));
}


// ---------- TElementType ----------

void TElementType::children(std::vector<const TNode*>& out) const {
	(void)out; // leaf
}

std::string TElementType::toString() const {
    return bnfcPrint(src_ElementType);
}

// ---------- TArithExpr ----------

std::string TArithExpr::toString() const {
    return bnfcPrint(src_ArithExpr);
}

void TVarExpr::children(std::vector<const TNode*>& out) const {
	(void)out;
}

void TLiteral::children(std::vector<const TNode*>& out) const {
	(void)out;
}

void TNegate::children(std::vector<const TNode*>& out) const {
	if (expr) out.push_back(expr.get());
}

void TPlus::children(std::vector<const TNode*>& out) const {
	for (auto const& a : args) if (a) out.push_back(a.get());
}

void TMinus::children(std::vector<const TNode*>& out) const {
	if (head) out.push_back(head.get());
	for (auto const& r : rest) if (r) out.push_back(r.get());
}

void TMultiply::children(std::vector<const TNode*>& out) const {
	for (auto const& a : args) if (a) out.push_back(a.get());
}

// ---------- TBoolExpr ----------

std::string TBoolExpr::toString() const {
    return bnfcPrint(src_BoolExpr);
}

void TCompare::children(std::vector<const TNode*>& out) const {
	if (lhs) out.push_back(lhs.get());
	if (rhs) out.push_back(rhs.get());
}

void TConnective::children(std::vector<const TNode*>& out) const {
	for (auto const& a : args) if (a) out.push_back(a.get());
}

// --- Assertion ---

void TAssertion::children(std::vector<const TNode*>& out) const {
	if (cond) out.push_back(cond.get());
}

std::string TAssertion::toString() const {
    return bnfcPrint(src_Assertion);
}

// --- Definitions ---

void TInputDefinition::children(std::vector<const TNode*>& out) const {
	(void)out; 
}

std::string TInputDefinition::toString() const {
    return bnfcPrint(src_InputDefinition);
}


void THiddenDefinition::children(std::vector<const TNode*>& out) const {
	(void)out;
}

std::string THiddenDefinition::toString() const {
    return bnfcPrint(src_HiddenDefinition);
}

void TOutputDefinition::children(std::vector<const TNode*>& out) const {
	(void)out; 
}

std::string TOutputDefinition::toString() const {
    return bnfcPrint(src_OutputDefinition);
}

// --- Network ---

void TNetworkDefinition::children(std::vector<const TNode*>& out) const {
    for (auto const& n : inputs)  if (n) out.push_back(n.get());
    for (auto const& n : hidden)  if (n) out.push_back(n.get());
    for (auto const& n : outputs) if (n) out.push_back(n.get());
}

std::string TNetworkDefinition::toString() const {
    return bnfcPrint(src_NetworkDefinition);
}

// --- Version ---

void TVersion::children(std::vector<const TNode*>& out) const {
	(void)out;
}

std::string TVersion::toString() const {
    return bnfcPrint(src_Version);
}

// --- Query ---

void TQuery::children(std::vector<const TNode*>& out) const {
	for (auto const& n : networks)   if (n) out.push_back(n.get());
	for (auto const& a : assertions) if (a) out.push_back(a.get());
}

std::string TQuery::toString() const {
    return bnfcPrint(src_Query);
}



