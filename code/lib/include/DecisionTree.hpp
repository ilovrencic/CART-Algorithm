/*
 * Copyright (c) DTAI - KU Leuven – All rights reserved.
 * Proprietary, do not copy or distribute without permission. 
 * Written by Pieter Robberechts, 2019
 */

#ifndef DECISIONTREE_DECISIONTREE_HPP
#define DECISIONTREE_DECISIONTREE_HPP

#include "Calculations.hpp"
#include "DataReader.hpp"
#include "Node.hpp"
#include "TreeTest.hpp"
#include "Utils.hpp"

class DecisionTree {
  public:
    DecisionTree() = delete;
    explicit DecisionTree(DataReader* dr);
    explicit DecisionTree(DataReader* dr, const std::vector<size_t>& samples);

    void print() const;
    void test() const;

    inline Data testData() { return dr_->testData(); }
    inline std::shared_ptr<Node> root() { return std::make_shared<Node>(root_); }

    Node root_;
  private:
    DataReader* dr_; //changed to pointer to reduce memory overhead

    const Node buildTree(const Data& rows, const MetaData &meta, const std::vector<size_t>& indexes);
    void print(const std::shared_ptr<Node> root, std::string spacing="") const;
    const std::vector<size_t> createIndexes(const Data& data);

};

#endif //DECISIONTREE_DECISIONTREE_HPP
