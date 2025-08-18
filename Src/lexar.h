#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_set>

// Token types for lexical analysis
enum class TokenType {
    // Literals
    NUMBER, STRING, BOOLEAN, IDENTIFIER, NULL_TOKEN,
    
    // Keywords - Control Flow
    CLASS, FUNCTION, IF, ELSE, WHILE, FOR, RETURN, VAR, TRUE, FALSE,
    BREAK, CONTINUE, SWITCH, CASE, DEFAULT, TRY, CATCH, FINALLY, THROW,
    
    // Keywords - Access Modifiers
    PUBLIC, PRIVATE, PROTECTED, STATIC, FINAL, ABSTRACT, VIRTUAL,
    
    // Keywords - Data Types
    VOID, INT, LONG, FLOAT, DOUBLE, STRING_TYPE, BOOLEAN_TYPE,
    CHAR, BYTE, SHORT,
    
    // Keywords - ML Specific
    TENSOR, MATRIX, MODEL, TRAIN, PREDICT, LAYER, OPTIMIZER,
    LOSS, METRIC, DATASET, BATCH, EPOCH, LEARNING_RATE,
    
    // Keywords - Advanced ML
    CONVOLUTION, POOLING, LSTM, GRU, ATTENTION, TRANSFORMER,
    EMBEDDING, DROPOUT, BATCH_NORM, ACTIVATION,
    
    // Keywords - Modules
    IMPORT, EXPORT, FROM, AS, PACKAGE, NAMESPACE,
    
    // Operators - Arithmetic
    PLUS, MINUS, MULTIPLY, DIVIDE, MODULO, POWER,
    
    // Operators - Assignment
    ASSIGN, PLUS_ASSIGN, MINUS_ASSIGN, MULTIPLY_ASSIGN, DIVIDE_ASSIGN,
    MODULO_ASSIGN, POWER_ASSIGN,
    
    // Operators - Comparison
    EQUALS, NOT_EQUALS, LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
    
    // Operators - Logical
    AND, OR, NOT, BITWISE_AND, BITWISE_OR, BITWISE_XOR, BITWISE_NOT,
    LEFT_SHIFT, RIGHT_SHIFT,
    
    // Operators - Increment/Decrement
    INCREMENT, DECREMENT,
    
    // Operators - Ternary
    QUESTION, COLON,
    
    // Delimiters
    SEMICOLON, COMMA, DOT, ARROW, DOUBLE_COLON,
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, 
    LEFT_BRACKET, RIGHT_BRACKET,
    
    // Special
    NEWLINE, EOF_TOKEN, COMMENT, MULTILINE_COMMENT,
    
    // ML Operators
    MATRIX_MULTIPLY, TENSOR_DOT, CONVOLUTION_OP,
    
    // Type annotations
    TYPE_ANNOTATION, GENERIC_START, GENERIC_END
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    size_t position;
    
    Token(TokenType t, const std::string& v, int l = 0, int c = 0, size_t p = 0) 
        : type(t), value(v), line(l), column(c), position(p) {}
        
    std::string toString() const;
    bool isKeyword() const;
    bool isOperator() const;
    bool isLiteral() const;
};

class LexerError : public std::exception {
private:
    std::string message;
    int line;
    int column;
    
public:
    LexerError(const std::string& msg, int l, int c) 
        : message(msg), line(l), column(c) {}
        
    const char* what() const noexcept override {
        return message.c_str();
    }
    
    int getLine() const { return line; }
    int getColumn() const { return column; }
    std::string getFullMessage() const;
};

class Lexer {
private:
    std::string source;
    size_t current;
    size_t start;
    int line;
    int column;
    
    // Keywords map for fast lookup
    static std::map<std::string, TokenType> keywords;
    
    // Character classification sets
    static std::unordered_set<char> whitespace;
    static std::unordered_set<char> digits;
    static std::unordered_set<char> alphaChars;
    static std::unordered_set<char> alphaNumeric;
    
public:
    explicit Lexer(const std::string& src);
    
    // Main tokenization method
    std::vector<Token> tokenize();
    
    // Single token methods
    Token nextToken();
    std::vector<Token> tokenizeExpression(const std::string& expression);
    
    // Utility methods
    bool hasMoreTokens() const;
    void reset();
    void reset(const std::string& newSource);
    
    // Error handling
    void reportError(const std::string& message);
    
private:
    // Character navigation
    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    
    // Token creation helpers
    Token makeToken(TokenType type);
    Token makeToken(TokenType type, const std::string& value);
    Token errorToken(const std::string& message);
    
    // Specific token parsers
    Token stringToken();
    Token numberToken();
    Token identifierToken();
    Token characterToken();
    Token commentToken();
    Token multilineCommentToken();
    
    // Character classification
    bool isDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isWhitespace(char c) const;
    bool isHexDigit(char c) const;
    bool isBinaryDigit(char c) const;
    bool isOctalDigit(char c) const;
    
    // Whitespace and comment handling
    void skipWhitespace();
    void skipLineComment();
    void skipBlockComment();
    
    // Number parsing helpers
    Token parseDecimalNumber();
    Token parseHexNumber();
    Token parseBinaryNumber();
    Token parseOctalNumber();
    Token parseFloatingPoint();
    
    // String parsing helpers
    std::string parseStringLiteral(char delimiter);
    std::string parseEscapeSequence();
    std::string parseUnicodeEscape();
    
    // Operator recognition
    TokenType getOperatorType(char first, char second = '\0', char third = '\0');
    bool isOperatorStart(char c) const;
    
    // Keyword recognition
    TokenType getIdentifierType(const std::string& text);
    
    // Position tracking
    void updatePosition(char c);
    void markTokenStart();
    std::string getCurrentLexeme() const;
    
    // Static initialization
    static void initializeKeywords();
    static void initializeCharacterSets();
    
public:
    // Static utility methods
    static std::string tokenTypeToString(TokenType type);
    static bool isAssignmentOperator(TokenType type);
    static bool isBinaryOperator(TokenType type);
    static bool isUnaryOperator(TokenType type);
    static bool isComparisonOperator(TokenType type);
    static bool isLogicalOperator(TokenType type);
    static bool isArithmeticOperator(TokenType type);
    static int getOperatorPrecedence(TokenType type);
    static bool isRightAssociative(TokenType type);
};

// Global utility functions
std::ostream& operator<<(std::ostream& os, const Token& token);
std::ostream& operator<<(std::ostream& os, TokenType type);