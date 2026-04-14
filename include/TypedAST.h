#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <variant>
#include <sstream>
#include <typeinfo>
#include <algorithm>

#include "Absyn.H"
#include "Printer.H"
#include "VNNLibExport.h"


using Shape = std::vector<int64_t>;
using Indices = std::vector<int64_t>;

// Supported Data Types
enum class TDataType {
	Real,
	F16, F32, F64, BF16,
	F8E4M3FN, F8E5M2, F8E4M3FNUZ, F8E5M2FNUZ, F4E2M1,
	I8, I16, I32, I64,
	U8, U16, U32, U64,
	C64, C128, Bool, String, Unknown,
	FloatConstant, NegativeIntConstant, PositiveIntConstant
};

std::string dtypeToString(TDataType dt);
bool isConstant(TDataType dt);
bool sameFamily(TDataType varDt, TDataType constDt);
bool sameType(TDataType a, TDataType b);

// Structure to store symbol information
enum class SymbolKind {Input, Hidden, Output, Network, Unknown};

class VNNLIB_API SymbolInfo final {
public:
	std::string name{};
	std::string onnxName{};
	TDataType dtype{TDataType::Unknown};
	Shape shape{};
	SymbolKind kind{SymbolKind::Unknown};
	std::string networkName{};

	bool isScalar() const;
	size_t rank() const;

	SymbolInfo(std::string name, TDataType dtype, Shape shape, SymbolKind kind, std::string onnxName = "")
        : name(name), onnxName(onnxName), dtype(dtype), shape(std::move(shape)), kind(kind) {}

    bool operator==(const SymbolInfo &other) const;
};

// --- Base Node ---

class VNNLIB_API TNode {
public:
	virtual ~TNode() = default;
	virtual void children(std::vector<const TNode*>& out) const = 0;
	virtual std::string toString() const = 0;

protected:
	TNode() = default;                               
	TNode(const TNode&) = delete;
	TNode& operator=(const TNode&) = delete;
	TNode(TNode&&) noexcept = default;            
	TNode& operator=(TNode&&) noexcept = default;
};

class VNNLIB_API TElementType : public TNode {
friend class TypedBuilder;
public:
	TDataType dtype{TDataType::Unknown};
	virtual ~TElementType() = default;
	void children(std::vector<const TNode*>& out) const override;
	std::string toString() const override;
protected:
	ElementType* src_ElementType{nullptr};
};

// --- Arithmetic Expressions ---

class VNNLIB_API TArithExpr : public TNode {
friend class TypedBuilder;
public:
	TDataType dtype{TDataType::Unknown};
	virtual ~TArithExpr() = default;
	std::string toString() const override;
protected:
	ArithExpr* src_ArithExpr{nullptr};
};

class VNNLIB_API TVarExpr final : public TArithExpr {
public:
	std::shared_ptr<const SymbolInfo> symbol{};
	Indices indices{};
	int line{-1};
	void children(std::vector<const TNode*>& out) const override;
};

class VNNLIB_API TLiteral : public TArithExpr {
public:
	std::string lexeme;
	int line{-1};
	void children(std::vector<const TNode*>& out) const override;
};

class VNNLIB_API TFloat final : public TLiteral {
public:
	double value{};
};

class VNNLIB_API TInt final : public TLiteral {
public:
	int64_t value{};
};

class VNNLIB_API TNegate final : public TArithExpr {
public:
	std::unique_ptr<TArithExpr> expr;
	void children(std::vector<const TNode*>& out) const override;
};

class VNNLIB_API TPlus final : public TArithExpr {
public:
	std::vector<std::unique_ptr<TArithExpr>> args;
	void children(std::vector<const TNode*>& out) const override;
};

class VNNLIB_API TMinus final : public TArithExpr {
public:
	std::unique_ptr<TArithExpr> head;
	std::vector<std::unique_ptr<TArithExpr>> rest;
	void children(std::vector<const TNode*>& out) const override;
};

class VNNLIB_API TMultiply final : public TArithExpr {
public:
	std::vector<std::unique_ptr<TArithExpr>> args;
	void children(std::vector<const TNode*>& out) const override;
};

// --- Boolean Expressions ---

class VNNLIB_API TBoolExpr : public TNode {
friend class TypedBuilder;
public:
	virtual ~TBoolExpr() = default;
	std::string toString() const override;
protected:
	BoolExpr* src_BoolExpr{nullptr};
};

class VNNLIB_API TCompare : public TBoolExpr {
public:
	std::unique_ptr<TArithExpr> lhs, rhs;
	void children(std::vector<const TNode*>& out) const override;
};

class VNNLIB_API TGreaterThan final : public TCompare {};
class VNNLIB_API TLessThan final : public TCompare {};
class VNNLIB_API TGreaterEqual final : public TCompare {};
class VNNLIB_API TLessEqual final : public TCompare {};
class VNNLIB_API TEqual final : public TCompare {};
class VNNLIB_API TNotEqual final : public TCompare {};

class VNNLIB_API TConnective : public TBoolExpr {
public:
	std::vector<std::unique_ptr<TBoolExpr>> args;
	void children(std::vector<const TNode*>& out) const override;
};

class VNNLIB_API TAnd final : public TConnective {};
class VNNLIB_API TOr final : public TConnective {};

// --- Assertion ---

class VNNLIB_API TAssertion final : public TNode {
friend class TypedBuilder;
public:
	std::unique_ptr<TBoolExpr> cond;
	void children(std::vector<const TNode*>& out) const override;
	std::string toString() const override;
protected:
	Assertion* src_Assertion{nullptr};
};

// --- Definitions ---

class VNNLIB_API TInputDefinition final : public TNode {
friend class TypedBuilder;
public:
	std::shared_ptr<const SymbolInfo> symbol{};
	void children(std::vector<const TNode*>& out) const override;
	std::string toString() const override;
	bool initialized;
protected:
	InputDefinition* src_InputDefinition{nullptr};
};

class VNNLIB_API THiddenDefinition final : public TNode {
friend class TypedBuilder;
public:
	std::shared_ptr<const SymbolInfo> symbol{};
	void children(std::vector<const TNode*>& out) const override;
	std::string toString() const override;
protected:
	HiddenDefinition* src_HiddenDefinition{nullptr};
};

class VNNLIB_API TOutputDefinition final : public TNode {
friend class TypedBuilder;
public:
	std::shared_ptr<const SymbolInfo> symbol{};
	void children(std::vector<const TNode*>& out) const override;
	std::string toString() const override;
protected:
	OutputDefinition* src_OutputDefinition{nullptr};
};

// --- Network ---

class VNNLIB_API TNetworkDefinition final : public TNode {
friend class TypedBuilder;
public:
	std::string isometricTo{};
	std::string equalTo{};
	std::string networkName{};
	std::vector<std::unique_ptr<TInputDefinition>> inputs{};
	std::vector<std::unique_ptr<THiddenDefinition>> hidden{};
	std::vector<std::unique_ptr<TOutputDefinition>> outputs{};
	void children(std::vector<const TNode*>& out) const override;
	std::string toString() const override;
protected:
	NetworkDefinition* src_NetworkDefinition{nullptr};
};

// --- Version ---

class VNNLIB_API TVersion final : public TNode {
friend class TypedBuilder;
public:
	int major{0};
  	int minor{0};
	void children(std::vector<const TNode*>& out) const override;
	std::string toString() const override;
protected:
	Version* src_Version{nullptr};
};

// --- Query ---

class VNNLIB_API TQuery final : public TNode {
friend class TypedBuilder;
public:
	std::unique_ptr<TVersion> version{};
	std::vector<std::unique_ptr<TNetworkDefinition>> networks{};
	std::vector<std::unique_ptr<TAssertion>> assertions{};
	void children(std::vector<const TNode*>& out) const override;
	std::string toString() const override;
protected:
	Query* src_Query{nullptr};
};









