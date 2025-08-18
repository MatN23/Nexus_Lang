#pragma once

#include "../value.h"
#include "tensor.h"
#include "layers.h"
#include "optimizers.h"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <random>
#include <functional>

// Forward declarations
class Layer;
class Optimizer;
class LossFunction;
class Metric;

// Training configuration
struct TrainingConfig {
    int epochs = 100;
    int batchSize = 32;
    double learningRate = 0.001;
    double validationSplit = 0.0;
    bool shuffle = true;
    bool verbose = true;
    int verboseLevel = 1;
    std::string optimizer = "adam";
    std::string loss = "mse";
    std::vector<std::string> metrics = {"accuracy"};
    
    // Early stopping
    bool earlyStoppingEnabled = false;
    int patience = 10;
    double minDelta = 0.0001;
    
    // Learning rate scheduling
    bool learningRateScheduling = false;
    double learningRateDecay = 0.9;
    int learningRateDecaySteps = 100;
    
    // Regularization
    double l1Regularization = 0.0;
    double l2Regularization = 0.0;
    double dropout = 0.0;
};

// Training history for tracking progress
struct TrainingHistory {
    std::vector<double> loss;
    std::vector<double> accuracy;
    std::vector<double> validationLoss;
    std::vector<double> validationAccuracy;
    std::map<std::string, std::vector<double>> customMetrics;
    
    void clear();
    void addEpoch(double trainLoss, double trainAcc, double valLoss = 0.0, double valAcc = 0.0);
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    std::string toString() const;
};

// Main Neural Network class
class NeuralNetwork {
private:
    std::vector<std::shared_ptr<Layer>> layers;
    std::shared_ptr<Optimizer> optimizer;
    std::shared_ptr<LossFunction> lossFunction;
    std::vector<std::shared_ptr<Metric>> metrics;
    
    TrainingConfig config;
    TrainingHistory history;
    
    bool compiled;
    bool trained;
    
    std::mt19937 randomEngine;
    
    // Model architecture info
    std::vector<size_t> inputShape;
    std::vector<size_t> outputShape;
    size_t totalParameters;
    
public:
    NeuralNetwork();
    explicit NeuralNetwork(const std::vector<int>& architecture);
    ~NeuralNetwork();
    
    // Model building
    void addLayer(std::shared_ptr<Layer> layer);
    void addDenseLayer(size_t units, const std::string& activation = "relu");
    void addConv2DLayer(size_t filters, size_t kernelSize, const std::string& activation = "relu");
    void addLSTMLayer(size_t units, bool returnSequences = false);
    void addDropoutLayer(double rate);
    void addBatchNormLayer();
    
    // Model compilation
    void compile(const std::string& optimizerName = "adam",
                const std::string& lossName = "mse",
                const std::vector<std::string>& metricNames = {"accuracy"});
    void compile(std::shared_ptr<Optimizer> opt,
                std::shared_ptr<LossFunction> loss,
                const std::vector<std::shared_ptr<Metric>>& metrics = {});
    
    // Training
    void train(const Tensor& inputs, const Tensor& targets);
    void train(const Tensor& inputs, const Tensor& targets, const TrainingConfig& config);
    void train(const std::vector<Tensor>& batchInputs, const std::vector<Tensor>& batchTargets);
    
    // Prediction
    Tensor predict(const Tensor& input);
    std::vector<Tensor> predict(const std::vector<Tensor>& inputs);
    Tensor predictBatch(const Tensor& batchInput);
    
    // Evaluation
    double evaluate(const Tensor& inputs, const Tensor& targets);
    std::map<std::string, double> evaluateMetrics(const Tensor& inputs, const Tensor& targets);
    
    // Model persistence
    void save(const std::string& filepath);
    void load(const std::string& filepath);
    void saveWeights(const std::string& filepath);
    void loadWeights(const std::string& filepath);
    
    // Model inspection
    void summary() const;
    std::string getArchitecture() const;
    size_t getParameterCount() const;
    std::vector<size_t> getLayerSizes() const;
    
    // Configuration
    void setTrainingConfig(const TrainingConfig& config) { this->config = config; }
    TrainingConfig& getTrainingConfig() { return config; }
    const TrainingConfig& getTrainingConfig() const { return config; }
    
    // History
    const TrainingHistory& getHistory() const { return history; }
    void clearHistory() { history.clear(); }
    
    // State
    bool isCompiled() const { return compiled; }
    bool isTrained() const { return trained; }
    
    // Advanced features
    void freeze();
    void unfreeze();
    void freezeLayer(size_t layerIndex);
    void unfreezeLayer(size_t layerIndex);
    
    // Transfer learning
    void transferWeights(const NeuralNetwork& sourceModel, size_t numLayers = 0);
    std::shared_ptr<NeuralNetwork> extractFeatureExtractor(size_t numLayers);
    
    // Model optimization
    void quantize(int bits = 8);
    void prune(double threshold = 0.01);
    void optimize();
    
    // Ensemble methods
    static std::shared_ptr<NeuralNetwork> createEnsemble(
        const std::vector<std::shared_ptr<NeuralNetwork>>& models);
    
private:
    // Forward and backward pass
    Tensor forward(const Tensor& input, bool training = false);
    void backward(const Tensor& loss);
    
    // Training utilities
    std::pair<std::vector<Tensor>, std::vector<Tensor>> createBatches(
        const Tensor& inputs, const Tensor& targets, int batchSize, bool shuffle = true);
    double calculateLoss(const Tensor& predictions, const Tensor& targets);
    std::map<std::string, double> calculateMetrics(const Tensor& predictions, const Tensor& targets);
    
    // Validation
    std::pair<Tensor, Tensor> splitValidation(const Tensor& inputs, const Tensor& targets, double ratio);
    
    // Initialization
    void initializeWeights();
    void initializeOptimizer();
    void initializeLoss();
    void initializeMetrics();
    
    // Architecture validation
    void validateArchitecture();
    void calculateTotalParameters();
    
    // Utility methods
    void shuffleData(std::vector<Tensor>& inputs, std::vector<Tensor>& targets);
    std::string formatTrainingProgress(int epoch, int totalEpochs, double loss, double acc) const;
};

// Factory functions for creating common architectures
namespace ModelFactory {
    std::shared_ptr<NeuralNetwork> createMLP(const std::vector<int>& layers, 
                                           const std::string& activation = "relu");
    std::shared_ptr<NeuralNetwork> createCNN(const std::vector<int>& convLayers,
                                           const std::vector<int>& denseLayers,
                                           const std::string& activation = "relu");
    std::shared_ptr<NeuralNetwork> createRNN(int inputSize, int hiddenSize, int outputSize,
                                           const std::string& cellType = "lstm");
    std::shared_ptr<NeuralNetwork> createAutoencoder(const std::vector<int>& encoderLayers);
    std::shared_ptr<NeuralNetwork> createGAN(int inputDim, int outputDim, 
                                           const std::vector<int>& generatorLayers,
                                           const std::vector<int>& discriminatorLayers);
}

// Pre-trained model loader
namespace PretrainedModels {
    std::shared_ptr<NeuralNetwork> loadResNet50(bool includeTop = true);
    std::shared_ptr<NeuralNetwork> loadVGG16(bool includeTop = true);
    std::shared_ptr<NeuralNetwork> loadBERT(const std::string& variant = "base");
    std::shared_ptr<NeuralNetwork> loadGPT2(const std::string& variant = "small");
}