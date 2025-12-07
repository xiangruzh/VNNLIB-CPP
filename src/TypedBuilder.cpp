#include "TypedBuilder.h"

// --- Utility Functions ---

template <class T>
std::unique_ptr<T> TypedBuilder::pop(std::vector<std::unique_ptr<T>>& stack) {
    assert(!stack.empty());
    auto p = std::move(stack.back());
    stack.pop_back();
    return p;
}

template <class T>
std::vector<std::unique_ptr<T>> TypedBuilder::popRange(std::vector<std::unique_ptr<T>>& stack,
                                                        size_t lo, size_t hi) {
    std::vector<std::unique_ptr<T>> out;
    out.reserve(hi - lo);
    for (size_t i = lo; i < hi; ++i) {
        out.push_back(std::move(stack[i]));
    }
    stack.resize(lo);
    return out;
}

// --- Entry Point (API) ---

std::unique_ptr<TQuery> TypedBuilder::build(VNNLibQuery* root) {
    root->accept(this);
    return std::move(tquery_);
}

// --- ArithExpr ---

void TypedBuilder::visitScalarVarExpr(ScalarVarExpr* p) {
    TypeChecker::visitScalarVarExpr(p);

    auto node = std::make_unique<TVarExpr>();
    node->src_ArithExpr = static_cast<ArithExpr*>(p);
    node->indices = {}; // Scalar variable has no indices

    auto it = symbolMap_.find(p->variablename_->string_);
    if (it != symbolMap_.end()) {
        node->symbol = it->second;
        node->line = p->variablename_->integer_;
        node->dtype = node->symbol->dtype;
    }
    arithStack_.push_back(std::move(node));
}

void TypedBuilder::visitTensorVarExpr(TensorVarExpr* p) {
    TypeChecker::visitTensorVarExpr(p);

    auto node = std::make_unique<TVarExpr>();
    node->src_ArithExpr = static_cast<ArithExpr*>(p);
    node->indices = mapIndices(p->listnumber_);

    auto it = symbolMap_.find(p->variablename_->string_);
    if (it != symbolMap_.end()) {
        node->symbol = it->second;
        node->line = p->variablename_->integer_;
        node->dtype = node->symbol->dtype;
    }
    arithStack_.push_back(std::move(node));
}

void TypedBuilder::visitValExpr(ValExpr* p) {
    TypeChecker::visitValExpr(p);

    std::unique_ptr<TLiteral> node;
    auto node_str = p->number_->string_;
    if (node_str.find('.') != std::string::npos) {
        node = std::make_unique<TFloat>();
        static_cast<TFloat*>(node.get())->value = std::stod(node_str);
    } else {
        node = std::make_unique<TInt>();
        static_cast<TInt*>(node.get())->value = std::stoll(node_str);
    }
    
    node->lexeme = node_str;
    node->line = p->number_->integer_;
    node->src_ArithExpr = static_cast<ArithExpr*>(p);
    arithStack_.push_back(std::move(node));
}

void TypedBuilder::visitNegate(Negate* p) {
    auto mark = arithStack_.size();
    // visits child using overridden function. The child is then pushed onto the stack.
    TypeChecker::visitNegate(p);

    auto node = std::make_unique<TNegate>();
    node->src_ArithExpr = static_cast<ArithExpr*>(p);

    assert(arithStack_.size() == mark + 1); // ensure only one child was pushed
    node->expr = pop(arithStack_);
    node->dtype = getContext().currentDataType;
    arithStack_.push_back(std::move(node));
}

void TypedBuilder::visitPlus(Plus* p) {
    const auto mark = arithStack_.size();
    TypeChecker::visitPlus(p);       // visits list of args

    auto node = std::make_unique<TPlus>();
    node->src_ArithExpr = static_cast<ArithExpr*>(p);

    node->args = popRange(arithStack_, mark, arithStack_.size());
    node->dtype = getContext().currentDataType;
    arithStack_.push_back(std::move(node));
}

void TypedBuilder::visitMinus(Minus* p) {
    const auto mark = arithStack_.size();
    TypeChecker::visitMinus(p);      // visits head + rest

    auto node = std::make_unique<TMinus>();
    node->src_ArithExpr = static_cast<ArithExpr*>(p);

    auto children = popRange(arithStack_, mark, arithStack_.size());
    assert(!children.empty());        // subtraction must have at least one operand

    node->head = std::move(children.front());
    children.erase(children.begin());
    node->rest = std::move(children);
    node->dtype = getContext().currentDataType;
    arithStack_.push_back(std::move(node));
}

void TypedBuilder::visitMultiply(Multiply* p) {
    const auto mark = arithStack_.size();
    TypeChecker::visitMultiply(p);

    auto node = std::make_unique<TMultiply>();
    node->src_ArithExpr = static_cast<ArithExpr*>(p);
    node->args = popRange(arithStack_, mark, arithStack_.size());
    node->dtype = getContext().currentDataType;
    arithStack_.push_back(std::move(node));
}

// --- BoolExpr ---

void TypedBuilder::visitGreaterThan(GreaterThan* p) {
    // Let base visit both sides
    TypeChecker::visitGreaterThan(p);
    auto rhs = pop(arithStack_);    
    auto lhs = pop(arithStack_);

    auto node = std::make_unique<TGreaterThan>();
    node->src_BoolExpr = static_cast<BoolExpr*>(p);

    node->lhs = std::move(lhs);
    node->rhs = std::move(rhs);
    boolStack_.push_back(std::move(node));
}

void TypedBuilder::visitLessThan(LessThan* p) {
    TypeChecker::visitLessThan(p);
    auto rhs = pop(arithStack_);
    auto lhs = pop(arithStack_);

    auto node = std::make_unique<TLessThan>();
    node->src_BoolExpr = static_cast<BoolExpr*>(p);

    node->lhs = std::move(lhs);
    node->rhs = std::move(rhs);
    boolStack_.push_back(std::move(node));
}

void TypedBuilder::visitGreaterEqual(GreaterEqual* p) {
    TypeChecker::visitGreaterEqual(p);
    auto rhs = pop(arithStack_);
    auto lhs = pop(arithStack_);

    auto node = std::make_unique<TGreaterEqual>();
    node->src_BoolExpr = static_cast<BoolExpr*>(p);

    node->lhs = std::move(lhs);
    node->rhs = std::move(rhs);
    boolStack_.push_back(std::move(node));
}

void TypedBuilder::visitLessEqual(LessEqual* p) {
    TypeChecker::visitLessEqual(p);
    auto rhs = pop(arithStack_);
    auto lhs = pop(arithStack_);

    auto node = std::make_unique<TLessEqual>();
    node->src_BoolExpr = static_cast<BoolExpr*>(p);

    node->lhs = std::move(lhs);
    node->rhs = std::move(rhs);
    boolStack_.push_back(std::move(node));
}

void TypedBuilder::visitNotEqual(NotEqual* p) {
    TypeChecker::visitNotEqual(p);
    auto rhs = pop(arithStack_);
    auto lhs = pop(arithStack_);

    auto node = std::make_unique<TNotEqual>();
    node->src_BoolExpr = static_cast<BoolExpr*>(p);

    node->lhs = std::move(lhs);
    node->rhs = std::move(rhs);
    boolStack_.push_back(std::move(node));
}

void TypedBuilder::visitEqual(Equal* p) {
    TypeChecker::visitEqual(p);
    auto rhs = pop(arithStack_);
    auto lhs = pop(arithStack_);

    auto node = std::make_unique<TEqual>();
    node->src_BoolExpr = static_cast<BoolExpr*>(p);

    node->lhs = std::move(lhs);
    node->rhs = std::move(rhs);
    boolStack_.push_back(std::move(node));
}

void TypedBuilder::visitAnd(And* p) {
    const auto mark = boolStack_.size();
    TypeChecker::visitAnd(p);        // visits list of args

    auto node = std::make_unique<TAnd>();
    node->src_BoolExpr = static_cast<BoolExpr*>(p);

    node->args = popRange(boolStack_, mark, boolStack_.size());
    boolStack_.push_back(std::move(node));
}

void TypedBuilder::visitOr(Or* p) {
    const auto mark = boolStack_.size();
    TypeChecker::visitOr(p);       

    auto node = std::make_unique<TOr>();
    node->src_BoolExpr = static_cast<BoolExpr*>(p);

    node->args = popRange(boolStack_, mark, boolStack_.size());
    boolStack_.push_back(std::move(node));
}

// --- Assertion ---

void TypedBuilder::visitAssert(Assert* p) {
    TypeChecker::visitAssert(p);

    auto node = std::make_unique<TAssertion>();
    node->src_Assertion = static_cast<Assertion*>(p);

    node->cond = pop(boolStack_);
    tquery_->assertions.push_back(std::move(node));
}

// --- Definitions ---

void TypedBuilder::visitInputDef(InputDef* p) {
    TypeChecker::visitInputDef(p);

    auto symbol = std::make_shared<SymbolInfo>(
        p->variablename_->string_, mapDType(p->elementtype_), mapShape(p->tensorshape_), SymbolKind::Input, ""
    );
    if (!netStack_.empty()) symbol->networkName = netStack_.back()->networkName;
    symbolMap_[symbol->name] = symbol;

    auto node = std::make_unique<TInputDefinition>();
    node->symbol = std::move(symbol);
    node->src_InputDefinition = static_cast<InputDefinition*>(p);

    auto lastNetwork = netStack_.back();
    lastNetwork->inputs.push_back(std::move(node));
}

void TypedBuilder::visitHiddenDef(HiddenDef* p) {
    TypeChecker::visitHiddenDef(p);

    NodeName *onnxName = dynamic_cast<NodeName*>(p->onnxname_);
    std::string onnxNameStr = onnxName->onnxstring_;
    auto symbol = std::make_shared<SymbolInfo>(
        p->variablename_->string_, mapDType(p->elementtype_), mapShape(p->tensorshape_), SymbolKind::Hidden, onnxNameStr
    );
    if (!netStack_.empty()) symbol->networkName = netStack_.back()->networkName;
    symbolMap_[symbol->name] = symbol;

    auto node = std::make_unique<THiddenDefinition>();
    node->symbol = std::move(symbol);
    node->src_HiddenDefinition = static_cast<HiddenDefinition*>(p);

    auto lastNetwork = netStack_.back();
    lastNetwork->hidden.push_back(std::move(node));
}

void TypedBuilder::visitOutputDef(OutputDef* p) {
    TypeChecker::visitOutputDef(p);

    auto symbol = std::make_shared<SymbolInfo>(
        p->variablename_->string_, mapDType(p->elementtype_), mapShape(p->tensorshape_), SymbolKind::Output, ""
    );
    if (!netStack_.empty()) symbol->networkName = netStack_.back()->networkName;
    symbolMap_[symbol->name] = symbol;

    auto node = std::make_unique<TOutputDefinition>();
    node->symbol = std::move(symbol);
    node->src_OutputDefinition = static_cast<OutputDefinition*>(p);

    auto lastNetwork = netStack_.back();
    lastNetwork->outputs.push_back(std::move(node));
}

// --- Relations ---

void TypedBuilder::visitEqualTo(EqualTo *p) {
    TypeChecker::visitEqualTo(p);

    if (!netStack_.empty()) {
        netStack_.back()->equalTo = p->variablename_->string_;
    }
}

void TypedBuilder::visitIsomorphicTo(IsomorphicTo *p) {
    TypeChecker::visitIsomorphicTo(p);

    if (!netStack_.empty()) {
        netStack_.back()->isometricTo = p->variablename_->string_;
    }
}

// --- Network ---

void TypedBuilder::visitNetworkDef(NetworkDef* p) {
    TypeChecker::visitNetworkDefinition(p);

    auto node = std::make_unique<TNetworkDefinition>();
    node->src_NetworkDefinition = static_cast<NetworkDefinition*>(p);
    node->networkName = p->variablename_->string_;

    // Add network to context stack that can be accessed by its children
    netStack_.push_back(node.get());
    TypeChecker::visitNetworkDef(p);

    netStack_.pop_back();
    tquery_->networks.push_back(std::move(node)); // Add network to query
}

// --- Version ---

void TypedBuilder::visitVNNLibVersion(VNNLibVersion *p) {
    TypeChecker::visitVNNLibVersion(p);

    auto node = std::make_unique<TVersion>();
    node->src_Version = static_cast<Version*>(p);
    // Format: <Major.Minor>
    node->major = std::stoi(p->versiontoken_->string_.substr(1));  
    node->minor = std::stoi(p->versiontoken_->string_.substr(3));
    tquery_->version = std::move(node);    
}

// --- Query ---

void TypedBuilder::visitVNNLibQuery(VNNLibQuery* p) {
    tquery_ = std::make_unique<TQuery>();
    tquery_->src_Query = static_cast<Query*>(p);

    TypeChecker::visitVNNLibQuery(p);
}

// --- Unused visitor methods ---
// These methods delegate to the parent TypeChecker class

void TypedBuilder::visitScalarDims(ScalarDims *p) {
    TypeChecker::visitScalarDims(p);
}

void TypedBuilder::visitTensorDims(TensorDims *p) {
    TypeChecker::visitTensorDims(p);
}

void TypedBuilder::visitDType(DType *p) {
    TypeChecker::visitDType(p);
}

void TypedBuilder::visitListArithExpr(ListArithExpr *p) {
    TypeChecker::visitListArithExpr(p);
}

void TypedBuilder::visitListBoolExpr(ListBoolExpr *p) {
    TypeChecker::visitListBoolExpr(p);
}

void TypedBuilder::visitListAssertion(ListAssertion *p) {
    TypeChecker::visitListAssertion(p);
}

void TypedBuilder::visitListInputDefinition(ListInputDefinition *p) {
    TypeChecker::visitListInputDefinition(p);
}

void TypedBuilder::visitListHiddenDefinition(ListHiddenDefinition *p) {
    TypeChecker::visitListHiddenDefinition(p);
}

void TypedBuilder::visitListOutputDefinition(ListOutputDefinition *p) {
    TypeChecker::visitListOutputDefinition(p);
}

void TypedBuilder::visitListNetworkEquivalence(ListNetworkEquivalence *p) {
    TypeChecker::visitListNetworkEquivalence(p);
}

void TypedBuilder::visitListNetworkDefinition(ListNetworkDefinition *p) {
    TypeChecker::visitListNetworkDefinition(p);
}

void TypedBuilder::visitListNumber(ListNumber *p) {
    TypeChecker::visitListNumber(p);
}

void TypedBuilder::visitInteger(Integer x) {
    TypeChecker::visitInteger(x);
}

void TypedBuilder::visitChar(Char x) {
    TypeChecker::visitChar(x);
}

void TypedBuilder::visitDouble(Double x) {
    TypeChecker::visitDouble(x);
}

void TypedBuilder::visitString(String x) {
    TypeChecker::visitString(x);
}

void TypedBuilder::visitIdent(Ident x) {
    TypeChecker::visitIdent(x);
}

void TypedBuilder::visitOnnxString(OnnxString x) {
    TypeChecker::visitOnnxString(x);
}

void TypedBuilder::visitVariableName(VariableName *p) {
    TypeChecker::visitVariableName(p);
}

void TypedBuilder::visitNumber(Number *p) {
    TypeChecker::visitNumber(p);
}

void TypedBuilder::visitVersionToken(VersionToken *p) {
    TypeChecker::visitVersionToken(p);
}
