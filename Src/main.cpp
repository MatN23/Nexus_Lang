#include "interpreter.h"
#include "version.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

void printUsage(const char* programName) {
    std::cout << "NEXUS Programming Language v" << NEXUS_VERSION << std::endl;
    std::cout << "Usage: " << programName << " [options] [file.nx]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help     Show this help message" << std::endl;
    std::cout << "  -v, --version  Show version information" << std::endl;
    std::cout << "  -i, --interactive  Start interactive REPL" << std::endl;
    std::cout << "  -d, --debug    Enable debug mode" << std::endl;
    std::cout << "  -p, --profile  Enable profiling" << std::endl;
    std::cout << "  -o, --output   Specify output file" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " hello.nx" << std::endl;
    std::cout << "  " << programName << " -i" << std::endl;
    std::cout << "  " << programName << " -d program.nx" << std::endl;
}

void printVersion() {
    std::cout << "NEXUS Programming Language" << std::endl;
    std::cout << "Version: " << NEXUS_VERSION << std::endl;
    std::cout << "Build Date: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "Compiler: " << NEXUS_COMPILER << std::endl;
    std::cout << "Platform: " << NEXUS_PLATFORM << std::endl;
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    
    return content;
}

void runInteractive() {
    std::cout << "NEXUS Interactive Shell v" << NEXUS_VERSION << std::endl;
    std::cout << "Type 'exit' to quit, 'help' for commands" << std::endl;
    std::cout << std::endl;
    
    NexusInterpreter interpreter;
    std::string input;
    int lineNumber = 1;
    
    while (true) {
        std::cout << "nexus:" << lineNumber << "> ";
        std::getline(std::cin, input);
        
        if (input == "exit" || input == "quit") {
            break;
        }
        
        if (input == "help") {
            std::cout << "Interactive Commands:" << std::endl;
            std::cout << "  help     - Show this help" << std::endl;
            std::cout << "  exit     - Exit the shell" << std::endl;
            std::cout << "  clear    - Clear variables" << std::endl;
            std::cout << "  vars     - Show all variables" << std::endl;
            std::cout << "  models   - Show all models" << std::endl;
            continue;
        }
        
        if (input == "clear") {
            interpreter.clearEnvironment();
            std::cout << "Environment cleared." << std::endl;
            continue;
        }
        
        if (input == "vars") {
            interpreter.printVariables();
            continue;
        }
        
        if (input == "models") {
            interpreter.printModels();
            continue;
        }
        
        if (!input.empty()) {
            try {
                interpreter.execute(input);
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
        
        lineNumber++;
    }
    
    std::cout << "Goodbye!" << std::endl;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> args(argv, argv + argc);
    
    bool debugMode = false;
    bool profileMode = false;
    bool interactive = false;
    std::string outputFile;
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
        else if (args[i] == "-p" || args[i] == "--profile") {
            profileMode = true;
        }
        else if (args[i] == "-o" || args[i] == "--output") {
            if (i + 1 < args.size()) {
                outputFile = args[++i];
            } else {
                std::cerr << "Error: -o requires an output file" << std::endl;
                return 1;
            }
        }
        else if (args[i][0] != '-') {
            inputFile = args[i];
        }
        else {
            std::cerr << "Unknown option: " << args[i] << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Start interactive mode if requested or no file provided
    if (interactive || (inputFile.empty() && !interactive)) {
        if (inputFile.empty() && !interactive) {
            std::cout << "No input file provided. Starting interactive mode..." << std::endl;
        }
        runInteractive();
        return 0;
    }
    
    // Execute file
    try {
        std::string source = readFile(inputFile);
        
        NexusInterpreter interpreter;
        if (debugMode) {
            interpreter.enableDebug();
        }
        if (profileMode) {
            interpreter.enableProfiling();
        }
        
        auto start = std::chrono::high_resolution_clock::now();
        interpreter.execute(source);
        auto end = std::chrono::high_resolution_clock::now();
        
        if (profileMode) {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "\nExecution time: " << duration.count() << "ms" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}