#pragma once

#include "value.h"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>

class EnvironmentError : public std::exception {
private:
    std::string message;
    
public:
    explicit EnvironmentError(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class Environment {
private:
    std::map<std::string, Value> variables;
    std::shared_ptr<Environment> parent;
    std::unordered_set<std::string> constants;  // Variables that cannot be reassigned
    std::string scopeName;
    int depth;
    
public:
    explicit Environment(std::shared_ptr<Environment> enclosing = nullptr, 
                        const std::string& name = "global");
    
    // Variable operations
    void define(const std::string& name, const Value& value, bool isConstant = false);
    void defineConstant(const std::string& name, const Value& value);
    Value get(const std::string& name) const;
    void assign(const std::string& name, const Value& value);
    bool exists(const std::string& name) const;
    bool existsInCurrentScope(const std::string& name) const;
    void remove(const std::string& name);
    
    // Scope management
    std::shared_ptr<Environment> getParent() const { return parent; }
    const std::string& getScopeName() const { return scopeName; }
    int getDepth() const { return depth; }
    void setScopeName(const std::string& name) { scopeName = name; }
    
    // Utility methods
    std::vector<std::string> getAllVariableNames() const;
    std::vector<std::string> getCurrentScopeVariables() const;
    size_t getVariableCount() const;
    size_t getTotalVariableCount() const;
    void clear();
    
    // Debug and inspection
    void printVariables() const;
    void printScope() const;
    void printAllScopes() const;
    std::string toString() const;
    
    // Constant checking
    bool isConstant(const std::string& name) const;
    
    // Serialization (for debugging/persistence)
    std::map<std::string, Value> exportVariables() const;
    void importVariables(const std::map<std::string, Value>& vars);
    
private:
    // Helper methods
    Environment* findEnvironmentWithVariable(const std::string& name) const;
    void validateAssignment(const std::string& name) const;
    std::string getFullScopePath() const;
};

// Global environment manager for the entire interpreter
class GlobalEnvironmentManager {
private:
    static std::shared_ptr<Environment> globalEnv;
    static std::vector<std::shared_ptr<Environment>> environmentStack;
    
public:
    // Global environment access
    static std::shared_ptr<Environment> getGlobal();
    static void setGlobal(std::shared_ptr<Environment> env);
    static void resetGlobal();
    
    // Environment stack management
    static void pushEnvironment(std::shared_ptr<Environment> env);
    static std::shared_ptr<Environment> popEnvironment();
    static std::shared_ptr<Environment> getCurrentEnvironment();
    static size_t getStackDepth();
    
    // Utility
    static void clearAll();
    static void printStack();
};