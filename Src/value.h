#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include <functional>

// Forward declarations
class NexusInterpreter;
class Environment;

// Value types enumeration
enum class ValueType {
    NIL,
    BOOLEAN,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT,
    FUNCTION,
    CLASS,
    INSTANCE,
    TENSOR,
    MODEL,
    DATASET
};

// Function signature for callable objects
using NativeFunction = std::function<class Value(const std::vector<class Value>&)>;

// Tensor class for ML operations
class Tensor {
private:
    std::vector<double> data;
    std::vector<size_t> shape;
    size_t totalSize;
    
public:
    Tensor();
    Tensor(const std::vector<size_t>& shape);
    Tensor(const std::vector<size_t>& shape, const std::vector<double>& data);
    Tensor(const std::vector<std::vector<double>>& matrix);
    
    // Shape operations
    const std::vector<size_t>& getShape() const { return shape; }
    size_t getDimensions() const { return shape.size(); }
    size_t getSize() const { return totalSize; }
    void reshape(const std::vector<size_t>& newShape);
    
    // Data access
    double& operator[](size_t index) { return data[index]; }
    const double& operator[](size_t index) const { return data[index]; }
    double& at(const std::vector<size_t>& indices);
    const double& at(const std::vector<size_t>& indices) const;
    
    // Mathematical operations
    Tensor operator+(const Tensor& other) const;
    Tensor operator-(const Tensor& other) const;
    Tensor operator*(const Tensor& other) const;  // Element-wise
    Tensor operator/(const Tensor& other) const;
    Tensor matmul(const Tensor& other) const;     // Matrix multiplication
    Tensor transpose() const;
    Tensor sum(int axis = -1) const;
    Tensor mean(int axis = -1) const;
    double norm() const;
    
    // Utility methods
    void fill(double value);
    void randomize(double min = -1.0, double max = 1.0);
    void zero();
    void ones();
    std::string toString() const;
    
private:
    size_t calculateIndex(const std::vector<size_t>& indices) const;
    void calculateTotalSize();
};

// Callable interface for functions and methods
class Callable {
public:
    virtual ~Callable() = default;
    virtual Value call(NexusInterpreter& interpreter, const std::vector<Value>& arguments) = 0;
    virtual std::string toString() const = 0;
    virtual size_t arity() const = 0;
};

// Native function wrapper
class NativeCallable : public Callable {
private:
    NativeFunction function;
    size_t paramCount;
    std::string name;
    
public:
    NativeCallable(const std::string& n, size_t params, NativeFunction fn)
        : function(fn), paramCount(params), name(n) {}
        
    Value call(NexusInterpreter& interpreter, const std::vector<Value>& arguments) override;
    std::string toString() const override { return "<native fn " + name + ">"; }
    size_t arity() const override { return paramCount; }
};

// Main Value class with optimized storage
class Value {
private:
    ValueType type_;
    
    // Optimized storage using variant
    std::variant<
        std::nullptr_t,           // NIL
        bool,                     // BOOLEAN
        double,                   // NUMBER
        std::string,              // STRING
        std::vector<Value>,       // ARRAY
        std::map<std::string, Value>, // OBJECT
        std::shared_ptr<Callable>, // FUNCTION
        std::shared_ptr<Tensor>   // TENSOR
    > data_;
    
public:
    // Constructors
    Value();
    Value(std::nullptr_t);
    Value(bool value);
    Value(int value);
    Value(double value);
    Value(const std::string& value);
    Value(const char* value);
    Value(const std::vector<Value>& value);
    Value(const std::map<std::string, Value>& value);
    Value(std::shared_ptr<Callable> value);
    Value(std::shared_ptr<Tensor> value);
    Value(const Tensor& value);
    
    // Copy and move constructors
    Value(const Value& other);
    Value(Value&& other) noexcept;
    
    // Assignment operators
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;
    
    // Destructor
    ~Value() = default;
    
    // Type checking
    ValueType getType() const { return type_; }
    bool isNil() const { return type_ == ValueType::NIL; }
    bool isBoolean() const { return type_ == ValueType::BOOLEAN; }
    bool isNumber() const { return type_ == ValueType::NUMBER; }
    bool isString() const { return type_ == ValueType::STRING; }
    bool isArray() const { return type_ == ValueType::ARRAY; }
    bool isObject() const { return type_ == ValueType::OBJECT; }
    bool isFunction() const { return type_ == ValueType::FUNCTION; }
    bool isTensor() const { return type_ == ValueType::TENSOR; }
    bool isCallable() const { return isFunction(); }
    
    // Type conversion
    bool asBool() const;
    double asNumber() const;
    std::string asString() const;
    const std::vector<Value>& asArray() const;
    std::vector<Value>& asArray();
    const std::map<std::string, Value>& asObject() const;
    std::map<std::string, Value>& asObject();
    std::shared_ptr<Callable> asCallable() const;
    std::shared_ptr<Tensor> asTensor() const;
    
    // Safe conversion with default values
    bool toBool(bool defaultValue = false) const;
    double toNumber(double defaultValue = 0.0) const;
    std::string toString(const std::string& defaultValue = "") const;
    
    // Arithmetic operations
    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator%(const Value& other) const;
    Value operator-() const;  // Unary minus
    
    // Comparison operations
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
    bool operator<(const Value& other) const;
    bool operator<=(const Value& other) const;
    bool operator>(const Value& other) const;
    bool operator>=(const Value& other) const;
    
    // Logical operations
    Value operator&&(const Value& other) const;
    Value operator||(const Value& other) const;
    Value operator!() const;
    
    // Array/Object operations
    Value& operator[](const Value& key);
    const Value& operator[](const Value& key) const;
    Value get(const Value& key) const;
    void set(const Value& key, const Value& value);
    bool has(const Value& key) const;
    size_t length() const;
    
    // Utility methods
    std::string getTypeString() const;
    bool isTruthy() const;
    bool isFalsy() const;
    Value deepCopy() const;
    size_t hash() const;
    
    // ML-specific operations
    Value dot(const Value& other) const;      // Tensor dot product
    Value matmul(const Value& other) const;   // Matrix multiplication
    Value transpose() const;                  // Matrix transpose
    Value reshape(const std::vector<size_t>& shape) const;
    
    // Iteration support
    class Iterator {
    private:
        const Value* value_;
        size_t index_;
        
    public:
        Iterator(const Value* v, size_t idx) : value_(v), index_(idx) {}
        
        Value operator*() const;
        Iterator& operator++();
        bool operator!=(const Iterator& other) const;
    };
    
    Iterator begin() const;
    Iterator end() const;
    
    // Static factory methods
    static Value nil();
    static Value boolean(bool value);
    static Value number(double value);
    static Value string(const std::string& value);
    static Value array(const std::vector<Value>& elements = {});
    static Value object(const std::map<std::string, Value>& properties = {});
    static Value tensor(const std::vector<size_t>& shape);
    static Value tensor(const std::vector<std::vector<double>>& matrix);
    
    // Serialization
    std::string serialize() const;
    static Value deserialize(const std::string& data);
    
private:
    // Helper methods
    void setType(ValueType type);
    void validateType(ValueType expected) const;
    std::string formatValue() const;
    
    // Type-specific helpers
    double compareNumbers(const Value& other) const;
    std::string concatenateStrings(const Value& other) const;
    Value performArithmetic(const Value& other, char op) const;
};

// Global utility functions
std::ostream& operator<<(std::ostream& os, const Value& value);
std::ostream& operator<<(std::ostream& os, ValueType type);

// Hash function for Value (for use in containers)
namespace std {
    template<>
    struct hash<Value> {
        std::size_t operator()(const Value& v) const {
            return v.hash();
        }
    };
}

// Value comparison functors
struct ValueEqual {
    bool operator()(const Value& lhs, const Value& rhs) const {
        return lhs == rhs;
    }
};

struct ValueLess {
    bool operator()(const Value& lhs, const Value& rhs) const {
        return lhs < rhs;
    }
};