#pragma once

#include "lexer.h"
#include "parser.h"
#include "environment.h"
#include "ml/neural_network.h"
#include <memory>
#include <map>
#include <chrono>

class NexusInterpreter {
private:
    std::shared_ptr<Environment> globals;
    std::shared_ptr<Environment> environment;
    std::map<std::string, std::shared_ptr<NeuralNetwork>> models;
    std::map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> profileTimers;
    bool debugMode;
    bool profilingMode;
    
public:
    NexusInterpreter();
    ~NexusInterpreter();
    
    // Core execution methods
    void execute(const std::string& source);
    void executeFile(const std::string& filename);
    Value evaluateExpression(const std::string& expression);
    
    // Environment management
    void clearEnvironment();
    void printVariables() const;
    void printModels() const;
    
    // Configuration
    void enableDebug() { debugMode = true; }
    void disableDebug() { debugMode = false; }
    void enableProfiling() { profilingMode = true; }
    void disableProfiling() { profilingMode = false; }
    
    // Built-in functions
    void setupBuiltins();
    Value callBuiltinFunction(const std::string& name, const std::vector<Value>& args);
    
    // ML operations
    void createModel(const std::string& name, const std::vector<int>& architecture);
    void trainModel(const std::string& name, const std::map<std::string, Value>& params = {});
    Value predictModel(const std::string& name, const Value& input = Value());
    void saveModel(const std::string& name, const std::string& filepath);
    void loadModel(const std::string& name, const std::string& filepath);
    
    // Profiling
    void startTimer(const std::string& name);
    double endTimer(const std::string& name);
    
private:
    // Statement execution
    void executeStatements(const std::vector<Token>& tokens);
    size_t executeStatement(const std::vector<Token>& tokens, size_t start);
    size_t executeVariableDeclaration(const std::vector<Token>& tokens, size_t start);
    size_t executeAssignment(const std::vector<Token>& tokens, size_t start);
    size_t executeIfStatement(const std::vector<Token>& tokens, size_t start);
    size_t executeWhileStatement(const std::vector<Token>& tokens, size_t start);
    size_t executeForStatement(const std::vector<Token>& tokens, size_t start);
    size_t executeModelDeclaration(const std::vector<Token>& tokens, size_t start);
    size_t executeTrainStatement(const std::vector<Token>& tokens, size_t start);
    size_t executeExpressionStatement(const std::vector<Token>& tokens, size_t start);
    size_t executeReturnStatement(const std::vector<Token>& tokens, size_t start);
    size_t executeImportStatement(const std::vector<Token>& tokens, size_t start);
    
    // Expression evaluation
    Value evaluateExpression(const std::vector<Token>& tokens, size_t& pos);
    Value evaluateLogicalOr(const std::vector<Token>& tokens, size_t& pos);
    Value evaluateLogicalAnd(const std::vector<Token>& tokens, size_t& pos);
    Value evaluateEquality(const std::vector<Token>& tokens, size_t& pos);
    Value evaluateComparison(const std::vector<Token>& tokens, size_t& pos);
    Value evaluateTerm(const std::vector<Token>& tokens, size_t& pos);
    Value evaluateFactor(const std::vector<Token>& tokens, size_t& pos);
    Value evaluateUnary(const std::vector<Token>& tokens, size_t& pos);
    Value evaluatePrimary(const std::vector<Token>& tokens, size_t& pos);
    Value evaluateCall(const std::vector<Token>& tokens, size_t& pos, Value callee);
    
    // Utility methods
    bool isAtEnd(const std::vector<Token>& tokens, size_t pos) const;
    bool check(const std::vector<Token>& tokens, size_t pos, TokenType type) const;
    bool match(const std::vector<Token>& tokens, size_t& pos, TokenType type);
    bool match(const std::vector<Token>& tokens, size_t& pos, const std::vector<TokenType>& types);
    Token advance(const std::vector<Token>& tokens, size_t& pos);
    Token peek(const std::vector<Token>& tokens, size_t pos) const;
    Token previous(const std::vector<Token>& tokens, size_t pos) const;
    void consume(const std::vector<Token>& tokens, size_t& pos, TokenType type, const std::string& message);
    
    // Block execution
    size_t executeBlock(const std::vector<Token>& tokens, size_t start);
    size_t findBlockEnd(const std::vector<Token>& tokens, size_t start);
    
    // Debug utilities
    void debugPrint(const std::string& message) const;
    void debugPrintTokens(const std::vector<Token>& tokens) const;
    void debugPrintEnvironment() const;
    
    // Error handling
    void runtimeError(const std::string& message);
    void runtimeError(const Token& token, const std::string& message);
};