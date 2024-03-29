/*
 * Copyright (c) DTAI - KU Leuven – All rights reserved.
 * Proprietary, do not copy or distribute without permission. 
 * Written by Pieter Robberechts, 2019
 */

#include "Bagging.hpp"

using std::make_shared;
using std::shared_ptr;
using std::string;
using boost::timer::cpu_timer;

Bagging::Bagging(DataReader *dr, const int ensembleSize, uint seed) :
  dr_(dr),
  ensembleSize_(ensembleSize),
  learners_({}) {
  random_number_generator.seed(seed);
  buildBag();
}


void Bagging::buildBag() {
  cpu_timer timer;
  std::vector<double> timings;
  for (int i = 0; i < ensembleSize_; i++) {
    timer.start();

    //sampling data and training a tree classifier with sampled data
    std::vector<size_t> samples = sampleData(dr_->trainData().size());
    const DecisionTree decisionTree(dr_, samples);
    learners_.emplace_back(std::move(decisionTree));

    auto nanoseconds = boost::chrono::nanoseconds(timer.elapsed().wall);
    auto seconds = boost::chrono::duration_cast<boost::chrono::seconds>(nanoseconds);
    timings.push_back(seconds.count());
  }
  float avg_timing = Utils::iterators::average(std::begin(timings), std::begin(timings) + std::min(5, ensembleSize_));
  std::cout << "Average timing: " << avg_timing << std::endl;
}

void Bagging::test() const {
  TreeTest t;
  float accuracy = 0;
  for (const auto& row: dr_->testData()) {
    static size_t last = row.size() - 1;
    std::vector<std::string> decisions;
    for (int i = 0; i < ensembleSize_; i++) {
      const std::shared_ptr<Node> root = std::make_shared<Node>(learners_.at(i).root_);
      const auto& classification = t.classify(row, root);
      decisions.push_back(Utils::tree::getMax(classification));
    }
    std::string prediction = Utils::iterators::mostCommon(decisions.begin(), decisions.end());
    if (prediction == row[last])
      accuracy += 1;
  }
  std::cout << "Total accuracy: " << (accuracy / dr_->testData().size()) << std::endl;
}

/**
 * Method that takes a sample of the original data. The data is randomly sampled.
 *
 * @param data - data that you want to sample
 * @return - vector of data indexes
 */
const std::vector<size_t> Bagging::sampleData(int size) const {
    std::uniform_int_distribution<size_t> distribution(0, size-1);
    std::vector<size_t> sampleData;

    for (int i = 0; i < size; i++){
        sampleData.push_back(distribution(const_cast<std::mt19937_64&>(random_number_generator)));
    }

    return sampleData;
}


