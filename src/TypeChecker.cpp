#include "TypeChecker.h"

// --- Utility methods ---

// Map ElementType to DType
TDataType TypeChecker::mapDType(ElementType* e) {
    DType *dt = dynamic_cast<DType *>(e);
    std::string typeName = dt->variablename_->string_;
    if (typeName == "Real") return TDataType::Real;
    else if (typeName == "F16") return TDataType::F16;
    else if (typeName == "F32") return TDataType::F32;
    else if (typeName == "F64") return TDataType::F64;
    else if (typeName == "BF16") return TDataType::BF16;
    else if (typeName == "F8E4M3FN") return TDataType::F8E4M3FN;
    else if (typeName == "F8E5M2") return TDataType::F8E5M2;
    else if (typeName == "F8E4M3FNUZ") return TDataType::F8E4M3FNUZ;
    else if (typeName == "F8E5M2FNUZ") return TDataType::F8E5M2FNUZ;
    else if (typeName == "F4E2M1") return TDataType::F4E2M1;
    else if (typeName == "I8") return TDataType::I8;
    else if (typeName == "I16") return TDataType::I16;
    else if (typeName == "I32") return TDataType::I32;
    else if (typeName == "I64") return TDataType::I64;
    else if (typeName == "U8") return TDataType::U8;
    else if (typeName == "U16") return TDataType::U16;
    else if (typeName == "U32") return TDataType::U32;
    else if (typeName == "U64") return TDataType::U64;
    else if (typeName == "C64") return TDataType::C64;
    else if (typeName == "C128") return TDataType::C128;
    else if (typeName == "Bool") return TDataType::Bool;
    else return TDataType::Unknown; // Unknown type
}

// Map TensorShape to Indices
Shape TypeChecker::mapShape(TensorShape* shape) {
    Shape out;
    if (!shape || dynamic_cast<ScalarDims*>(shape)) return out; // scalar
    if (auto tensorShape = dynamic_cast<TensorDims*>(shape)) {
        if (tensorShape->listnumber_) {
            out.reserve(tensorShape->listnumber_->size());
            for (auto& dim : *tensorShape->listnumber_) {
                try { 
                    out.push_back(std::stoll(dim->string_)); 
                } catch (const std::exception& e) { 
                    out.push_back(-1); 
                }
            }
        }
    }
    return out;
}

// Map ListInt to Indices
Indices TypeChecker::mapIndices(const ListNumber* inds) {
    Indices out;
    if (!inds) return out;
    out.reserve(inds->size());
    for (const auto& indTok : *inds) {
        try { 
            out.push_back(std::stoll(indTok->string_)); 
        } catch (const std::invalid_argument& e) { 
            out.push_back(-1); 
        }
    }
    return out;
}

// Utility function for string formatting (@iFreilicht)
template<typename ... Args>
static std::string string_format( const std::string& format, Args ... args )
{
    int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    std::snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

// Helper function to convert SymbolKind to string
static std::string kindToString(SymbolKind kind) {
    switch (kind) {
        case SymbolKind::Input: return "input";
        case SymbolKind::Hidden: return "hidden";
        case SymbolKind::Output: return "output";
        default: return "unknown";
    }
}

// --- Context methods ---

Context::Context(TypeChecker* typeChecker) :
    currentDataType(TDataType::Unknown),
    lastScannedVariable(""),
    symbolMap(),
    checker(typeChecker) {}

// Add a symbol to the context. Returns true if successful, false if a symbol with the same name already exists, or if the symbol is invalid.
bool Context::addSymbol(VariableName *name, ElementType *type, ListNumber shape, SymbolKind kind, std::string onnxName) {
    if (symbolMap.find(name->string_) != symbolMap.end()) {
        checker->addDiagnostic(Severity::Error, 
                    static_cast<int>(ErrorCode::MultipleDeclaration), 
                    "Duplicate variable declaration", 
                    name->string_,
                    "Variable names must be unique within the specification",
                    name->integer_
                );
        return false;
    }

    Indices tmp;
    TDataType dtype = TypeChecker::mapDType(type);
    
    // Convert shape dimensions to integers for validation
    for (const auto& dim : shape) {
        int64_t dim_val;
        try {
            dim_val = std::stoll(dim->string_); // If conversion fails, user has passed a non-int to shape
        } catch (std::invalid_argument& e) {
            dim_val = -1;
        }
        if (dim_val < 1) {
            checker->addDiagnostic(Severity::Error, 
                static_cast<int>(ErrorCode::InvalidDimensions), 
                "Failed to parse dimension - Invalid size", 
                name->string_,
                "Dimensions must be positive integers.",
                name->integer_
            );
            return false;
        }
        tmp.push_back(dim_val);
    }

    symbolMap.try_emplace(
        name->string_,
        name->string_,
        dtype,
        std::move(tmp),
        kind,
        onnxName
    );
    return true;
}

// Get a symbol from the context by name. Returns a pointer to the symbol if found, or nullptr if not.
SymbolInfo *Context::getSymbol(const VariableName &name) {
    auto it = symbolMap.find(name.string_);
    if (it != symbolMap.end()) {
        return &it->second;
    }
    return nullptr; // Symbol not found
}

TypeChecker::TypeChecker() {
    ctx = std::make_unique<Context>(this);
}

TypeChecker::~TypeChecker() = default;

// --- Error collection and reporting methods ---

// Convert error code to string representation
std::string Diagnostic::codeToString() const {
    if (severity_ == Severity::Error) {
        auto ec = static_cast<ErrorCode>(code_);
        switch (ec) {
            case ErrorCode::MultipleDeclaration: return "MultipleDeclaration";
            case ErrorCode::TypeMismatch: return "TypeMismatch";
            case ErrorCode::UndeclaredVariable: return "UndeclaredVariable";
            case ErrorCode::IndexOutOfBounds: return "IndexOutOfBounds";
            case ErrorCode::TooManyIndices: return "TooManyIndices";
            case ErrorCode::NotEnoughIndices: return "NotEnoughIndices";
            case ErrorCode::InvalidDimensions: return "InvalidDimensions";
            case ErrorCode::MajorVersionMismatch: return "MajorVersionMismatch";
            case ErrorCode::VariableCountMismatch: return "VariableCountMismatch";
            case ErrorCode::VariableShapeMismatch: return "VariableShapeMismatch";
            case ErrorCode::VariableKindMismatch: return "VariableKindMismatch";
            case ErrorCode::MissingNetwork: return "MissingNetwork";
            case ErrorCode::ChainedEquivalence: return "ChainedEquivalence";
            case ErrorCode::UnknownType: return "UnknownType";
            default: break;
        }
    } else if (severity_ == Severity::Warning) {
        auto wc = static_cast<WarningCode>(code_);
        switch (wc) {
            case WarningCode::MinorVersionMismatch: return "MinorVersionMismatch";
            default: break;
        }
    }
    return "UnknownCode";
}

// Create a JSON representation of a type-check error
std::string Diagnostic::toJson() const {
    nlohmann::json j;
    j["errorCode"] = codeToString();  // Use string representation instead of int
    j["message"] = message_;
    j["offendingSymbol"] = offending_symbol_;
    j["hint"] = hint_;
    if (line_ != -1) j["line"] = line_;
    return j.dump();
}

// Add a diagnostic to the collection
void TypeChecker::addDiagnostic(Severity severity, int code, const std::string &message,
                                const std::string &offending_symbol,
                                const std::string &hint, int line) {
    if (severity == Severity::Error) {
        errors.emplace_back(severity, code, message, offending_symbol, hint, line);
    } else if (severity == Severity::Warning) {
        warnings.emplace_back(severity, code, message, offending_symbol, hint, line);
    }
}

int TypeChecker::getErrorCount() const {
    return static_cast<int>(errors.size());
}

int TypeChecker::getWarningCount() const {
    return static_cast<int>(warnings.size());
}

// Get a JSON representation of all the errors collected
std::string TypeChecker::getErrorReport() const {
    nlohmann::json report;
    if (errors.size() > 0) {
        report["status"] = "failure";
    } else {
        report["status"] = "success";
    }
    report["error_count"] = errors.size();
    report["errors"] = nlohmann::json::array();
    
    for (const auto& error : errors) {
        nlohmann::json errorJson = nlohmann::json::parse(error.toJson());
        report["errors"].push_back(errorJson);
    }

    report["warning_count"] = warnings.size();
    report["warnings"] = nlohmann::json::array();

    for (const auto& warning : warnings) {
        nlohmann::json warningJson = nlohmann::json::parse(warning.toJson());
        report["warnings"].push_back(warningJson);
    }

    return report.dump(2);
}

// --- Visitor Methods --- 

void TypeChecker::visitTensorShape(TensorShape *p) {}       // Abstract class
void TypeChecker::visitScalarDims(ScalarDims *p) {}

void TypeChecker::visitTensorDims(TensorDims *p) {
    visitListNumber(p->listnumber_);
}

void TypeChecker::visitArithExpr(ArithExpr *p) {} // abstract class

void TypeChecker::visitScalarVarExpr(ScalarVarExpr* p) {
    validateVariableAccess(p->variablename_);
    validateTensorIndexing(p->variablename_, {}); // Scalars have no indices
    p->variablename_->accept(this);
}

void TypeChecker::visitTensorVarExpr(TensorVarExpr* p) {
    validateVariableAccess(p->variablename_);
    validateTensorIndexing(p->variablename_, TypeChecker::mapIndices(p->listnumber_));
    p->variablename_->accept(this);
}

void TypeChecker::visitValExpr(ValExpr *p) {
    // If currentDataType is unset, assign a new FloatConstant
    std::string valTok = p->number_->string_;
    TDataType newType = TDataType::Unknown;

    if (valTok.find('.') != std::string::npos) {
        newType = TDataType::FloatConstant;
    } else {
        if (valTok.find('-') != std::string::npos) {
            newType = TDataType::NegativeIntConstant;
        } else {
            newType = TDataType::PositiveIntConstant;
        }
    }

    if (ctx->currentDataType == TDataType::Unknown) {
        ctx->currentDataType = newType;
        ctx->lastScannedVariable = valTok;
    // if the currentDataType is incompatible with the constant type, add error
    } else if (!sameFamily(ctx->currentDataType, newType)) {
        addDiagnostic(
            Severity::Error,
            static_cast<int>(ErrorCode::TypeMismatch),
            "Type mismatch in arithmetic expression",
            valTok,
            string_format(
                "Expected type '%s' (from '%s'), but found a %s constant '%s'.",
                dtypeToString(ctx->currentDataType).c_str(),
                ctx->lastScannedVariable.c_str(), 
                dtypeToString(newType).c_str(),
                valTok.c_str()
            ),
            p->number_->integer_
        );
        return;
    }
}

void TypeChecker::visitNegate(Negate *p) {
    p->arithexpr_->accept(this);
}

void TypeChecker::visitPlus(Plus *p) {
    visitListArithExpr(p->listarithexpr_);
}

void TypeChecker::visitMinus(Minus *p) {
    p->arithexpr_->accept(this);
    visitListArithExpr(p->listarithexpr_);
}

void TypeChecker::visitMultiply(Multiply *p) {
    visitListArithExpr(p->listarithexpr_);
}

void TypeChecker::visitListArithExpr(ListArithExpr *p) {
    for (auto &arithExpr : *p) {
        arithExpr->accept(this);
    }
}

void TypeChecker::visitBoolExpr(BoolExpr *p) {} // abstract class

void TypeChecker::visitGreaterThan(GreaterThan *p) {
    ctx->currentDataType = TDataType::Unknown;
    p->arithexpr_1->accept(this);
    p->arithexpr_2->accept(this);
}

void TypeChecker::visitLessThan(LessThan *p) {
    ctx->currentDataType = TDataType::Unknown;
    p->arithexpr_1->accept(this);
    p->arithexpr_2->accept(this);
}

void TypeChecker::visitGreaterEqual(GreaterEqual *p) {
    ctx->currentDataType = TDataType::Unknown;
    p->arithexpr_1->accept(this);
    p->arithexpr_2->accept(this);
}

void TypeChecker::visitLessEqual(LessEqual *p) {
    ctx->currentDataType = TDataType::Unknown;
    p->arithexpr_1->accept(this);
    p->arithexpr_2->accept(this);
}

void TypeChecker::visitNotEqual(NotEqual *p) {
    ctx->currentDataType = TDataType::Unknown;
    p->arithexpr_1->accept(this);
    p->arithexpr_2->accept(this);
}

void TypeChecker::visitEqual(Equal *p) {
    ctx->currentDataType = TDataType::Unknown;
    p->arithexpr_1->accept(this);
    p->arithexpr_2->accept(this);
}

void TypeChecker::visitAnd(And *p) {
    visitListBoolExpr(p->listboolexpr_);
}

void TypeChecker::visitOr(Or *p) {
    visitListBoolExpr(p->listboolexpr_);
}

void TypeChecker::visitListBoolExpr(ListBoolExpr *p) {
    for (auto &boolExpr : *p) {
        boolExpr->accept(this);
    }
}

void TypeChecker::visitAssertion(Assertion *p) {} // abstract class

void TypeChecker::visitAssert(Assert *p) {
    p->boolexpr_->accept(this);
}

void TypeChecker::visitListAssertion(ListAssertion *p) {
    for (auto &assertion : *p) {
        assertion->accept(this);
    }
}

void TypeChecker::visitElementType(ElementType *p) {}   // abstract class

void TypeChecker::visitOnnxName(OnnxName *p) {} // abstract class

void TypeChecker::visitNodeName(NodeName *p) {}

void TypeChecker::visitDType(DType *p) {}

void TypeChecker::visitInputDefinition(InputDefinition *p) {} // abstract class

void TypeChecker::visitInputDef(InputDef *p) {
    visitVariableName(p->variablename_);
    p->elementtype_->accept(this);
    p->tensorshape_->accept(this);
    p->elementtype_->accept(this);

    auto* shape = dynamic_cast<TensorDims*>(p->tensorshape_);
    // Set dims to an empty list if shape is null or shape->listnumber_ is null
    ListNumber dims = (shape && shape->listnumber_) ? *shape->listnumber_ : ListNumber{};

    ctx->addSymbol(
        p->variablename_,
        p->elementtype_,
        dims,
        SymbolKind::Input
    );
}

void TypeChecker::visitHiddenDefinition(HiddenDefinition *p) {} // abstract class

void TypeChecker::visitHiddenDef(HiddenDef *p) {
    visitVariableName(p->variablename_);
    p->elementtype_->accept(this);
    p->tensorshape_->accept(this);
    p->elementtype_->accept(this);

    NodeName *onnxName = dynamic_cast<NodeName*>(p->onnxname_);
    std::string onnxNameStr = onnxName->string_;
    auto* shape = dynamic_cast<TensorDims*>(p->tensorshape_);
    ListNumber dims = (shape && shape->listnumber_) ? *shape->listnumber_ : ListNumber{};

    ctx->addSymbol(
        p->variablename_,
        p->elementtype_,
        dims,
        SymbolKind::Hidden,
        onnxNameStr
    );
}

void TypeChecker::visitOutputDefinition(OutputDefinition *p) {} //abstract class

void TypeChecker::visitOutputDef(OutputDef *p) {
    visitVariableName(p->variablename_);
    p->elementtype_->accept(this);
    p->tensorshape_->accept(this);
    p->elementtype_->accept(this);

    auto* shape = dynamic_cast<TensorDims*>(p->tensorshape_);
    ListNumber dims = (shape && shape->listnumber_) ? *shape->listnumber_ : ListNumber{};

    ctx->addSymbol(
        p->variablename_,
        p->elementtype_,
        dims,
        SymbolKind::Output
    );
}

void TypeChecker::visitListInputDefinition(ListInputDefinition *listinputdefinition)
{
    for (auto &inputDef : *listinputdefinition) {
        inputDef->accept(this);
    }
}

void TypeChecker::visitListHiddenDefinition(ListHiddenDefinition *listhiddendefinition)
{
    for (auto &hiddenDef : *listhiddendefinition) {
        hiddenDef->accept(this);
    }
}

void TypeChecker::visitListOutputDefinition(ListOutputDefinition *listoutputdefinition)
{
    for (auto &outputDef : *listoutputdefinition) {
        outputDef->accept(this);
    }
}

void TypeChecker::visitNetworkEquivalence(NetworkEquivalence *p) {} // abstract base class

void TypeChecker::visitIsomorphicTo(IsomorphicTo *p) {
    validateNetworkCongruence(p->variablename_, "isomorphic-to");
}

void TypeChecker::visitEqualTo(EqualTo *p) {
    validateNetworkCongruence(p->variablename_, "equal-to");
}

void TypeChecker::visitListNetworkEquivalence(ListNetworkEquivalence *p) {
    for (auto &networkEquivalence : *p) {
        networkEquivalence->accept(this);
    }
}

void TypeChecker::visitNetworkDefinition(NetworkDefinition *p) {} // abstract base class

void TypeChecker::visitNetworkDef(NetworkDef *p) {
    currentNetworkName = p->variablename_->string_;
    
    visitVariableName(p->variablename_);
    visitListInputDefinition(p->listinputdefinition_);
    visitListHiddenDefinition(p->listhiddendefinition_);
    visitListOutputDefinition(p->listoutputdefinition_);
    
    // Store network information for later validation
    NetworkInfo networkInfo;
    networkInfo.name = currentNetworkName;
    networkInfo.hasEquivalence = !p->listnetworkequivalence_->empty();
    collectNetworkVariables(networkInfo, p->listinputdefinition_, p->listoutputdefinition_);
    networks[currentNetworkName] = networkInfo;
    visitListNetworkEquivalence(p->listnetworkequivalence_);
}

void TypeChecker::visitListNetworkDefinition(ListNetworkDefinition *listnetworkdefinition)
{
    for (auto &networkDef : *listnetworkdefinition) {
        networkDef->accept(this);
    }
}

void TypeChecker::visitVersion(Version *p) {} // abstract base class

void TypeChecker::visitVNNLibVersion(VNNLibVersion *p) {
    std::string ver = p->versiontoken_->string_;
    int major = 0, minor = 0;
    std::sscanf(ver.c_str(), "<%d.%d>", &major, &minor);

    if (major != VNNLIB_MAJOR_VERSION) {
        addDiagnostic(
            Severity::Error,
            static_cast<int>(ErrorCode::MajorVersionMismatch),
            "Incompatible VNNLib version",
            ver,
            string_format("Expected VNNLib version <%d.x>, but found version <%d.%d>.", VNNLIB_MAJOR_VERSION, major, minor),
            p->versiontoken_->integer_
        );
    }

    if (minor != VNNLIB_MINOR_VERSION) {
        addDiagnostic(
            Severity::Warning,
            static_cast<int>(WarningCode::MinorVersionMismatch),
            "Minor version mismatch",
            ver,
            string_format("Expected VNNLib version <%d.%d>, but found version <%d.%d>.", VNNLIB_MAJOR_VERSION, VNNLIB_MINOR_VERSION, major, minor),
            p->versiontoken_->integer_
        );
    }
}

void TypeChecker::visitQuery(Query *p) {} // abstract base class

void TypeChecker::visitVNNLibQuery(VNNLibQuery *p) {
    assert(p->version_); p->version_->accept(this);
    p->listnetworkdefinition_->accept(this);
    assert(p->listassertion_); p->listassertion_->accept(this);
}

// --- Helper Methods for Validation ---

// Helper function to efficiently create strings to represent tensor element access
std::string make_element(std::string_view name, const Indices& indices) {
    std::string out;
    out.reserve(name.size() + 2 + indices.size() * 12);
    out.append(name);
    out.push_back('[');

    bool first = true;
    for (int idx : indices) {
        if (!first) out.push_back(',');
        first = false;

        char buf[24];
        auto [p, ec] = std::to_chars(std::begin(buf), std::end(buf), idx);
        out.append(buf, p);
    }

    out.push_back(']');
    return out;
}

// Checks for valid tensor element access
void TypeChecker::validateTensorIndexing(VariableName *name, Indices indices) {
    const SymbolInfo *symbol = ctx->getSymbol(*name);
    std::string name_str = name->string_;
    std::string element_str = make_element(name_str, indices);

    if (symbol->shape.size() == 0 && !indices.empty()) {
        // For scalars (empty shape), do not allow any indexing
        addDiagnostic(
            Severity::Error,
            static_cast<int>(ErrorCode::InvalidScalarAccess),
            "Invalid indexing for scalar variable.",
            element_str,
            "Scalar variables cannot be indexed.",
            name->integer_
        );
        return;
    }

    for (size_t i = 0; i < indices.size(); ++i) {
        // Check for too many indices
        if (i >= symbol->shape.size()) {
            addDiagnostic(
                Severity::Error,
                static_cast<int>(ErrorCode::TooManyIndices),
                "Too many indices for variable",
                element_str,
                string_format("Expected %zu indices but encountered %zu.",
                    symbol->shape.size(), indices.size()),
                name->integer_
            );
            return;
        }
        // Check that the index is within the variable's declared shape
        if (indices[i] >= symbol->shape[i]) {
            addDiagnostic(
                Severity::Error,
                static_cast<int>(ErrorCode::IndexOutOfBounds),
                "Index out of bounds for variable",
                element_str,
                string_format("Index %d is out of bounds for dimension %zu with size %d.",
                              indices[i], i, symbol->shape[i]),
                name->integer_
            );
            return;
        }
    }

    // Check for not enough indices
    if (indices.size() < symbol->shape.size()) {
        addDiagnostic(
            Severity::Error,
            static_cast<int>(ErrorCode::NotEnoughIndices),
            "Not enough indices for variable",
            element_str,
            string_format("Expected %zu indices but encountered %zu.", symbol->shape.size(), indices.size()),
            name->integer_
        );
        return;
    }
}

// Validate variable use in arithmetic expressions
void TypeChecker::validateVariableAccess(const VariableName* name) {
    const SymbolInfo *symbol = ctx->getSymbol(*name);
    if (!symbol) {
        addDiagnostic(
            Severity::Error,
            static_cast<int>(ErrorCode::UndeclaredVariable),
            "Undeclared variable",
            name->string_,
            "Variable must be declared before use.",
            name->integer_
        );
    }

    TDataType nodeType = symbol->dtype;
    TDataType exprType = ctx->currentDataType;

    if (exprType == TDataType::Unknown) {
        ctx->currentDataType = nodeType;
        ctx->lastScannedVariable = name->string_;
    }
    // if exprType is a constant type, check if nodeType is of the same family. E.g., Float and Float
    else if (isConstant(exprType)) {
        if (sameFamily(nodeType, exprType)) {
            ctx->currentDataType = nodeType;
            ctx->lastScannedVariable = name->string_;
        } else {
            addDiagnostic(
                Severity::Error,
                static_cast<int>(ErrorCode::TypeMismatch),
                "Type mismatch in arithmetic expression",
                name->string_,
                string_format(
                    "Expected a %s type to match constant '%s', but variable '%s' has type '%s'.",
                    dtypeToString(exprType).c_str(),
                    ctx->lastScannedVariable.c_str(),
                    name->string_.c_str(),
                    dtypeToString(nodeType).c_str()
                ),
                name->integer_
            );
        }
    }
    // if exprType is a variable type, check if nodeType is of the same type
    else if (!sameType(exprType, nodeType)) {
        addDiagnostic(
            Severity::Error,
            static_cast<int>(ErrorCode::TypeMismatch),
            "Type mismatch in arithmetic expression",
            name->string_,
            string_format(
                "Expected type '%s' (from variable '%s'), but variable '%s' has type '%s'.",
                dtypeToString(exprType).c_str(), 
                ctx->lastScannedVariable.c_str(),
                name->string_.c_str(),
                dtypeToString(nodeType).c_str()
            ),
            name->integer_
        );
    }
    else {
        ctx->currentDataType = nodeType;
        ctx->lastScannedVariable = name->string_;
    }
}

// Helper method to validate whether two networks are congruent (isometric or equal graphs)
void TypeChecker::validateNetworkCongruence(VariableName* referencedNetworkName, const std::string& statementType) {
    std::string referencedName = referencedNetworkName->string_;
    auto it = networks.find(referencedName);
    if (it == networks.end()) {
        addDiagnostic(Severity::Error, static_cast<int>(ErrorCode::MissingNetwork),
                     string_format("Referenced network '%s' not found for %s statement",
                                   referencedName.c_str(), statementType.c_str()),
                     referencedName,
                     "Ensure the network is declared before this statement",
                     referencedNetworkName->integer_);
        return;
    }
    
    if (!currentNetworkName.empty()) {
        auto currentIt = networks.find(currentNetworkName);
        if (currentIt != networks.end()) {
            const NetworkInfo& referencedNetwork = it->second;
            const NetworkInfo& currentNetwork = currentIt->second;
            
            // Check variables congruence
            if (!areVariablesCongruent(currentNetwork, referencedNetwork, referencedNetworkName->integer_)) {
                return;
            }

            // Check that the referenced network does not have further equivalence constraints
            if (referencedNetwork.hasEquivalence) {
                addDiagnostic(Severity::Error, static_cast<int>(ErrorCode::ChainedEquivalence),
                            string_format("Chained network equivalence is not allowed. Network '%s' already has equivalence constraints.", referencedName.c_str()),
                            referencedName,
                            "Network equivalence cannot be chained.",
                            referencedNetworkName->integer_);
            }
        }
    }
}

// Helper method to collect network input and output variables
void TypeChecker::collectNetworkVariables(NetworkInfo& networkInfo, const ListInputDefinition* inputs, const ListOutputDefinition* outputs) {
    // Collect input variables
    for (auto input : *inputs) {
        auto inputDef = dynamic_cast<InputDef*>(input); 
        SymbolInfo* symbol = ctx->getSymbol(*inputDef->variablename_);
        if (symbol) {
            networkInfo.vars.push_back(symbol);
        }
    }
    
    // Collect output variables
    for (auto output : *outputs) {
        auto outputDef = dynamic_cast<OutputDef*>(output);
        SymbolInfo* symbol = ctx->getSymbol(*outputDef->variablename_);
        if (symbol) {
            networkInfo.vars.push_back(symbol);
        }
    }
}

// Helper method to compare the input/output variables of two networks for congruence
bool TypeChecker::areVariablesCongruent(const NetworkInfo& current, const NetworkInfo& target, int line) {
    // Check if the variable counts match
    if (current.vars.size() != target.vars.size()) {
        addDiagnostic(Severity::Error, static_cast<int>(ErrorCode::VariableCountMismatch),
                    string_format("Number of variables mismatch between networks '%s' and '%s'",
                                  current.name.c_str(), target.name.c_str()),
                    target.name,
                    "Ensure both networks have the same number of input/output variables",
                    line);
        return false;
    }
    
    // Match variables by position
    for (size_t i = 0; i < current.vars.size(); ++i) {
        const SymbolInfo* sym1 = current.vars[i];
        const SymbolInfo* sym2 = target.vars[i];
        if (!validateSymbolCongruence(*sym1, *sym2, current, target, i, line)) {
            return false;
        }
    }

    return true;
}

// Helper method to validate congruence of two symbols (variables)
bool TypeChecker::validateSymbolCongruence(const SymbolInfo& sym1, const SymbolInfo& sym2, 
    const NetworkInfo& current, const NetworkInfo& target, size_t i, int line) {
    // Check kind compatibility (input vs output)
    if (sym1.kind != sym2.kind) {
        addDiagnostic(Severity::Error, static_cast<int>(ErrorCode::VariableKindMismatch),
                    string_format("Variable kind mismatch for variable number %d between networks '%s' and '%s'",
                                i, current.name.c_str(), target.name.c_str()),
                    target.name,
                    string_format("Expected variable number %d to be of kind '%s', but found kind '%s'.",
                                i, kindToString(sym1.kind).c_str(), kindToString(sym2.kind).c_str()),
                    line);
        return false;
    }
    
    if (sym1.dtype != sym2.dtype) {
        addDiagnostic(Severity::Error, static_cast<int>(ErrorCode::TypeMismatch),
                        string_format("Type mismatch for variable number %d between networks '%s' and '%s'",
                                    i, current.name.c_str(), target.name.c_str()),
                        target.name,
                        string_format("Expected variable number %d to be of type '%s', but found type '%s'.",
                                    i, dtypeToString(sym1.dtype).c_str(), dtypeToString(sym2.dtype).c_str()),
                        line);
        return false;
    }

    if (sym1.shape != sym2.shape) {
        addDiagnostic(Severity::Error, static_cast<int>(ErrorCode::VariableShapeMismatch),
                        string_format("Shape mismatch for variable number %d between networks '%s' and '%s'",
                                    i, current.name.c_str(), target.name.c_str()),
                        target.name,
                        "Ensure corresponding variables have the same shape",
                        line);
        return false;
    }
    
    return true;
}

// Visitors for BNFC tokens
void TypeChecker::visitInteger(Integer x) {}
void TypeChecker::visitChar(Char x) {}
void TypeChecker::visitDouble(Double x) {}
void TypeChecker::visitString(String x) {}
void TypeChecker::visitIdent(Ident x) {}
void TypeChecker::visitVariableName(VariableName *p) {}     // Token for variable names

void TypeChecker::visitListNumber(ListNumber *p) {
    for (const auto &num : *p) {
        num->accept(this);
    }
}

void TypeChecker::visitNumber(Number *p) {}                 // Token for number literals

void TypeChecker::visitVersionToken(VersionToken *x) {}     // Token for version