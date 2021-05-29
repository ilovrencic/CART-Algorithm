/*
 * Copyright (c) DTAI - KU Leuven – All rights reserved.
 * Proprietary, do not copy or distribute without permission. 
 * Written by Pieter Robberechts, 2019
 */

#include "DecisionTree.hpp"
#include <future>

using std::make_shared;
using std::shared_ptr;
using std::string;
using boost::timer::cpu_timer;

DecisionTree::DecisionTree(DataReader* dr) : root_(Node()), dr_(dr) {
  std::cout << "Start building tree." << std::endl; cpu_timer timer;
  root_ = buildTree(dr_->trainData(), dr_->metaData(), createIndexes(dr->trainData()));
  std::cout << "Done. " << timer.format() << std::endl;
}

DecisionTree::DecisionTree(DataReader* dr, const std::vector<size_t>& samples) : root_(Node()), dr_(dr) {
    std::cout << "Start building tree." << std::endl; cpu_timer timer;
    root_ = buildTree(dr_->trainData(), dr_->metaData(), samples);
    std::cout << "Done. " << timer.format() << std::endl;
}

const Node DecisionTree::buildTree(const Data& rows, const MetaData& meta, const std::vector<size_t>& indexes) {
    auto const& [gain, question] = Calculations::find_best_split(rows,meta,indexes);

    if (gain == 0) {
        return Node(Leaf(Calculations::classCounts(rows, indexes)));
    }

    //partitioning the data indexes, instead of the data
    const auto partitions{Calculations::partition(rows, question, indexes)} ;
    const std::vector<size_t>& true_branch{ std::get<0>(partitions) };
    const std::vector<size_t>& false_branch{ std::get<1>(partitions) };

    //starting two different async calls that are going to build tree in parallel
    std::future<const Node> future1{std::async(&DecisionTree::buildTree, this, std::cref(rows), std::cref(meta), std::cref(true_branch))};
    std::future<const Node> future2{std::async(&DecisionTree::buildTree, this, std::cref(rows), std::cref(meta), std::cref(false_branch))};

    Node left_node { future1.get() };
    Node right_node { future2.get() };

    return Node(left_node, right_node, question);
}

void DecisionTree::print() const {
  print(make_shared<Node>(root_));
}

void DecisionTree::print(const shared_ptr<Node> root, string spacing) const {
  if (bool is_leaf = root->leaf() != nullptr; is_leaf) {
    const auto &leaf = root->leaf();
    std::cout << spacing + "Predict: "; Utils::print::print_map(leaf->predictions());
    return;
  }
  std::cout << spacing << root->question().toString(dr_->metaData().labels) << "\n";

  std::cout << spacing << "--> True: " << "\n";
  print(root->trueBranch(), spacing + "   ");

  std::cout << spacing << "--> False: " << "\n";
  print(root->falseBranch(), spacing + "   ");
}

void DecisionTree::test() const {
  TreeTest t(dr_->testData(), dr_->metaData(), root_);
}

const std::vector<size_t> DecisionTree::createIndexes(const Data& data) {
    std::vector<size_t> indexes(data.size());
    std::iota(indexes.begin(), indexes.end(), 0);
    return indexes;
}
