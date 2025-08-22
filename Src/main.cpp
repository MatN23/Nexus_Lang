#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <chrono>
#include <algorithm>
#include <thread>
#include <cstdlib>

// Version information
#define NEXUS_VERSION "1.3.0"
#define NEXUS_BUILD_DATE __DATE__ " " __TIME__
#define NEXUS_PLATFORM "macOS ARM64"

// ANSI color codes for better output
namespace Colors {
    const std::string RESET = "\033[0m";
    const std::string BOLD = "\033[1m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string BLUE = "\033[34m";
    const std::string YELLOW = "\033[33m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
}

// Simple lexer for basic tokenization (stub)
enum class TokenType {
    IDENTIFIER, NUMBER, STRING, KEYWORD, OPERATOR, DELIMITER, NEWLINE, EOF_TOKEN
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    int column;
    
    Token(TokenType t, const std::string& v, int l = 0, int c = 0) 
        : type(t), value(v), line(l), column(c) {}
};

class SimpleTokenizer {
private:
    std::string source;
    size_t current = 0;
    int line = 1;
    int column = 1;
    
    std::map<std::string, bool> keywords = {
        {"var", true}, {"int", true}, {"double", true}, {"string", true}, {"bool", true},
        {"if", true}, {"else", true}, {"while", true}, {"for", true}, {"function", true},
        {"class", true}, {"return", true}, {"true", true}, {"false", true}, {"null", true},
        {"model", true}, {"train", true}, {"predict", true}, {"tensor", true}, {"import", true}
    };
    
public:
    explicit SimpleTokenizer(const std::string& src) : source(src) {}
    
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        
        while (!isAtEnd()) {
            skipWhitespace();
            if (isAtEnd()) break;
            
            char c = advance();
            
            if (std::isalpha(c) || c == '_') {
                tokens.push_back(identifier());
            } else if (std::isdigit(c)) {
                tokens.push_back(number());
            } else if (c == '"') {
                tokens.push_back(string());
            } else if (c == '\n') {
                tokens.push_back(Token(TokenType::NEWLINE, "\\n", line++, column));
                column = 1;
            } else {
                tokens.push_back(Token(TokenType::OPERATOR, std::string(1, c), line, column++));
            }
        }
        
        tokens.push_back(Token(TokenType::EOF_TOKEN, "", line, column));
        return tokens;
    }
    
private:
    bool isAtEnd() const { return current >= source.length(); }
    
    char advance() {
        if (isAtEnd()) return '\0';
        column++;
        return source[current++];
    }
    
    char peek() const {
        if (isAtEnd()) return '\0';
        return source[current];
    }
    
    void skipWhitespace() {
        while (!isAtEnd() && std::isspace(peek()) && peek() != '\n') {
            advance();
        }
    }
    
    Token identifier() {
        size_t start = current - 1;
        while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
            advance();
        }
        
        std::string text = source.substr(start, current - start);
        TokenType type = keywords.count(text) ? TokenType::KEYWORD : TokenType::IDENTIFIER;
        return Token(type, text, line, column - text.length());
    }
    
    Token number() {
        size_t start = current - 1;
        while (!isAtEnd() && (std::isdigit(peek()) || peek() == '.')) {
            advance();
        }
        
        std::string text = source.substr(start, current - start);
        return Token(TokenType::NUMBER, text, line, column - text.length());
    }
    
    Token string() {
        size_t start = current;
        while (!isAtEnd() && peek() != '"') {
            if (peek() == '\n') line++;
            advance();
        }
        
        if (isAtEnd()) {
            return Token(TokenType::STRING, "Unterminated string", line, column);
        }
        
        std::string text = source.substr(start, current - start);
        advance(); // Closing "
        return Token(TokenType::STRING, text, line, column - text.length());
    }
};

// Simple interpreter (stub implementation)
class NexusInterpreter {
private:
    std::map<std::string, std::string> variables;
    bool debugMode = false;
    
public:
    void setDebugMode(bool debug) { debugMode = debug; }
    
    void execute(const std::string& source) {
        if (source.empty()) return;
        
        SimpleTokenizer tokenizer(source);
        auto tokens = tokenizer.tokenize();
        
        if (debugMode) {
            std::cout << Colors::CYAN << "=== Debug: Tokens ===" << Colors::RESET << std::endl;
            for (const auto& token : tokens) {
                std::cout << "  " << tokenTypeToString(token.type) 
                         << ": '" << token.value << "'" << std::endl;
            }
            std::cout << std::endl;
        }
        
        executeTokens(tokens);
    }
    
private:
    void executeTokens(const std::vector<Token>& tokens) {
        for (size_t i = 0; i < tokens.size(); ++i) {
            const auto& token = tokens[i];
            
            if (token.type == TokenType::KEYWORD) {
                if (token.value == "var" && i + 2 < tokens.size()) {
                    handleVariableDeclaration(tokens, i);
                } else if (token.value == "model") {
                    handleModelDeclaration(tokens, i);
                } else if (token.value == "train") {
                    handleTrainStatement(tokens, i);
                } else if (token.value == "print") {
                    handlePrintStatement(tokens, i);
                }
            } else if (token.type == TokenType::IDENTIFIER) {
                if (token.value == "print" && i + 1 < tokens.size()) {
                    handlePrintStatement(tokens, i);
                }
            }
        }
    }
    
    void handleVariableDeclaration(const std::vector<Token>& tokens, size_t& i) {
        if (i + 4 < tokens.size() && tokens[i + 2].value == "=") {
            std::string varName = tokens[i + 1].value;
            std::string value = tokens[i + 3].value;
            
            variables[varName] = value;
            std::cout << Colors::GREEN << "✓ Variable '" << varName 
                     << "' = " << value << Colors::RESET << std::endl;
            i += 4;
        }
    }
    
    void handleModelDeclaration(const std::vector<Token>& tokens, size_t& i) {
        if (i + 2 < tokens.size()) {
            std::string modelName = tokens[i + 1].value;
            std::cout << Colors::MAGENTA << "🧠 Created model '" << modelName 
                     << "'" << Colors::RESET << std::endl;
            i += 2;
        }
    }
    
    void handleTrainStatement(const std::vector<Token>& tokens, size_t& i) {
        if (i + 1 < tokens.size()) {
            std::string modelName = tokens[i + 1].value;
            std::cout << Colors::YELLOW << "🚀 Training model '" << modelName 
                     << "'..." << Colors::RESET << std::endl;
            // Simulate training time
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::cout << Colors::GREEN << "✅ Training completed!" << Colors::RESET << std::endl;
            i += 1;
        }
    }
    
    void handlePrintStatement(const std::vector<Token>& tokens, size_t& i) {
        if (i + 1 < tokens.size()) {
            std::string value = tokens[i + 1].value;
            if (value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            } else if (variables.count(value)) {
                value = variables[value];
            }
            std::cout << value << std::endl;
            i += 1;
        }
    }
    
    std::string tokenTypeToString(TokenType type) {
        switch (type) {
            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::NUMBER: return "NUMBER";
            case TokenType::STRING: return "STRING";
            case TokenType::KEYWORD: return "KEYWORD";
            case TokenType::OPERATOR: return "OPERATOR";
            case TokenType::DELIMITER: return "DELIMITER";
            case TokenType::NEWLINE: return "NEWLINE";
            case TokenType::EOF_TOKEN: return "EOF";
            default: return "UNKNOWN";
        }
    }
};

// Utility functions
void printBanner() {
    std::cout << Colors::BOLD << Colors::BLUE;
    std::cout << "╔══════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  🧠 NEXUS Programming Language                           ║" << std::endl;
    std::cout << "║     ML-First Language with Familiar Syntax              ║" << std::endl;
    std::cout << "║     Version " << NEXUS_VERSION << "                                      ║" << std::endl;
    std::cout << "╚══════════════════════════════════════════════════════════╝" << std::endl;
    std::cout << Colors::RESET << std::endl;
}

void printUsage(const char* programName) {
    std::cout << Colors::BOLD << "NEXUS Programming Language v" << NEXUS_VERSION << Colors::RESET << std::endl;
    std::cout << "Usage: " << Colors::CYAN << programName << " [options] [file.nx]" << Colors::RESET << std::endl;
    std::cout << std::endl;
    std::cout << Colors::YELLOW << "Options:" << Colors::RESET << std::endl;
    std::cout << "  -h, --help        Show this help message" << std::endl;
    std::cout << "  -v, --version     Show version information" << std::endl;
    std::cout << "  -i, --interactive Start interactive REPL" << std::endl;
    std::cout << "  -d, --debug       Enable debug mode" << std::endl;
    std::cout << "  -e, --eval        Evaluate expression directly" << std::endl;
    std::cout << "  --ast             Show Abstract Syntax Tree" << std::endl;
    std::cout << "  --tokens          Show tokenization output" << std::endl;
    std::cout << std::endl;
    std::cout << Colors::YELLOW << "Examples:" << Colors::RESET << std::endl;
    std::cout << "  " << programName << " hello.nx" << std::endl;
    std::cout << "  " << programName << " -i" << std::endl;
    std::cout << "  " << programName << " -d program.nx" << std::endl;
    std::cout << "  " << programName << " -e \"var x = 42; print(x);\"" << std::endl;
}

void printVersion() {
    std::cout << Colors::BOLD << "NEXUS Programming Language" << Colors::RESET << std::endl;
    std::cout << "Version: " << Colors::GREEN << NEXUS_VERSION << Colors::RESET << std::endl;
    std::cout << "Build Date: " << NEXUS_BUILD_DATE << std::endl;
    std::cout << "Platform: " << NEXUS_PLATFORM << std::endl;
    std::cout << "Compiler: " << __VERSION__ << std::endl;
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void runInteractive() {
    printBanner();
    std::cout << Colors::CYAN << "Interactive REPL Mode" << Colors::RESET << std::endl;
    std::cout << "Type 'exit' to quit, 'help' for commands" << std::endl;
    std::cout << std::endl;
    
    NexusInterpreter interpreter;
    std::string input;
    int lineNumber = 1;
    
    while (true) {
        std::cout << Colors::BLUE << "nexus:" << lineNumber << "> " << Colors::RESET;
        std::getline(std::cin, input);
        
        if (input == "exit" || input == "quit") {
            break;
        }
        
        if (input == "help") {
            std::cout << Colors::YELLOW << "Interactive Commands:" << Colors::RESET << std::endl;
            std::cout << "  help        - Show this help" << std::endl;
            std::cout << "  exit/quit   - Exit the REPL" << std::endl;
            std::cout << "  version     - Show version info" << std::endl;
            std::cout << "  clear       - Clear screen" << std::endl;
            std::cout << std::endl;
            std::cout << Colors::YELLOW << "NEXUS Examples:" << Colors::RESET << std::endl;
            std::cout << "  var message = \"Hello NEXUS!\";" << std::endl;
            std::cout << "  model net = [784, 256, 10];" << std::endl;
            std::cout << "  train net;" << std::endl;
            std::cout << "  print(\"Hello World!\");" << std::endl;
            continue;
        }
        
        if (input == "version") {
            printVersion();
            continue;
        }
        
        if (input == "clear") {
            std::system("clear");
            continue;
        }
        
        if (!input.empty()) {
            try {
                auto start = std::chrono::high_resolution_clock::now();
                interpreter.execute(input);
                auto end = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                std::cout << Colors::CYAN << "⏱️  " << duration.count() << "μs" << Colors::RESET << std::endl;
                
            } catch (const std::exception& e) {
                std::cout << Colors::RED << "Error: " << e.what() << Colors::RESET << std::endl;
            }
        }
        
        lineNumber++;
    }
    
    std::cout << Colors::GREEN << "👋 Goodbye! Thanks for using NEXUS!" << Colors::RESET << std::endl;
}

void runExample() {
    std::cout << Colors::YELLOW << "🚀 Running NEXUS example..." << Colors::RESET << std::endl;
    std::cout << std::endl;
    
    std::string exampleCode = R"(
var message = "Hello NEXUS!";
var number = 42;
print(message);
print(number);

model neuralNet = [784, 256, 128, 10];
train neuralNet;

var result = "Neural network trained successfully!";
print(result);
)";
    
    NexusInterpreter interpreter;
    interpreter.setDebugMode(false);
    interpreter.execute(exampleCode);
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);
    
    bool debugMode = false;
    bool interactive = false;
    bool showTokens = false;
    bool showAST = false;
    std::string evalExpression;
    std::string inputFile;
    
    // Parse command line arguments
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "-h" || args[i] == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else if (args[i] == "-v" || args[i] == "--version") {
            printVersion();
            return 0;
        }
        else if (args[i] == "-i" || args[i] == "--interactive") {
            interactive = true;
        }
        else if (args[i] == "-d" || args[i] == "--debug") {
            debugMode = true;
        }
        else if (args[i] == "--tokens") {
            showTokens = true;
            debugMode = true;
        }
        else if (args[i] == "--ast") {
            showAST = true;
        }
        else if (args[i] == "-e" || args[i] == "--eval") {
            if (i + 1 < args.size()) {
                evalExpression = args[++i];
            } else {
                std::cerr << Colors::RED << "Error: -e requires an expression" << Colors::RESET << std::endl;
                return 1;
            }
        }
        else if (args[i] == "--example") {
            runExample();
            return 0;
        }
        else if (args[i][0] != '-') {
            inputFile = args[i];
        }
        else {
            std::cerr << Colors::RED << "Unknown option: " << args[i] << Colors::RESET << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Execute based on options
    try {
        NexusInterpreter interpreter;
        interpreter.setDebugMode(debugMode);
        
        if (!evalExpression.empty()) {
            // Direct evaluation
            std::cout << Colors::YELLOW << "Evaluating: " << evalExpression << Colors::RESET << std::endl;
            interpreter.execute(evalExpression);
        }
        else if (interactive || inputFile.empty()) {
            // Interactive mode
            runInteractive();
        }
        else {
            // Execute file
            printBanner();
            std::cout << Colors::CYAN << "Executing: " << inputFile << Colors::RESET << std::endl;
            std::cout << std::endl;
            
            std::string source = readFile(inputFile);
            auto start = std::chrono::high_resolution_clock::now();
            interpreter.execute(source);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << std::endl << Colors::CYAN << "⏱️  Execution time: " 
                     << duration.count() << "ms" << Colors::RESET << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << Colors::RED << "Error: " << e.what() << Colors::RESET << std::endl;
        return 1;
    }
    
    return 0;
}