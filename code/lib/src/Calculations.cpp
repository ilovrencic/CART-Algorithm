/*
 * Copyright (c) DTAI - KU Leuven – All rights reserved.
 * Proprietary, do not copy or distribute without permission. 
 * Written by Pieter Robberechts, 2019
 */

#include <cmath>
#include <algorithm>
#include <iterator>
#include "Calculations.hpp"
#include "Utils.hpp"
#include <future>

using std::tuple;
using std::pair;
using std::forward_as_tuple;
using std::vector;
using std::string;
using std::unordered_map;

tuple<const std::vector<size_t>, const std::vector<size_t>> Calculations::partition(const Data& data, const Question& q, const std::vector<size_t>& indexes) {
  std::vector<size_t> true_indexes;
  std::vector<size_t> false_indexes;
  
  for (const auto &index : indexes) {
    auto row = data[index];
    if (q.solve(row))
      true_indexes.push_back(index);
    else
      false_indexes.push_back(index);
  }

  return forward_as_tuple(true_indexes, false_indexes);
}

tuple<const double, const Question> Calculations::find_best_split(const Data& rows, const MetaData& meta, const std::vector<size_t>& indexes) {
  double best_gain = 0.0;  // keep track of the best information gain
  auto best_question = Question();  //keep track of the feature / value that produced it

  //there has to be at least two datapoints to make a split
  if (rows.size() <= 1){
      return forward_as_tuple(best_gain, best_question);
  }

  //find current current class distribution
  ClassCounter current_node_classes = classCounts(rows, indexes);

  //find current gini index
  double best_gini = gini(current_node_classes, indexes.size());

  //going through all features and for each feature finding the best threshold and loss
  for (int column = 0; column < meta.labels.size()-1; column++){
      //presorting the list of indexes, based on the data values
      std::vector<size_t> current_indexes = indexes;

      if (meta.labelMap.find(meta.labels[column])->second == "NUMERIC"){
          std::sort(current_indexes.begin(), current_indexes.end(), NumericComparator(column, rows));
      } else {
          std::sort(current_indexes.begin(), current_indexes.end(), StringComparator(column, rows));
      }

      //determining best threshold to split the data
      tuple<std::string, double> result = determine_best_threshold(rows, column, current_indexes, current_node_classes);
      std::vector<size_t>().swap(current_indexes);

      std::string threshold = std::get<0>(result);
      double gini_index = std::get<1>(result);

      if ((best_gini-gini_index) > best_gain){
          best_gain = best_gini-gini_index;
          best_question = Question(column, threshold);
      }
  }

  return forward_as_tuple(best_gain, best_question);
}

const double Calculations::gini(const ClassCounter& counts, double N) {
  double impurity = 1.0;

  //computing the gini index
  for (auto& it : counts){
      double pk = std::pow((it.second/N), 2);
      impurity -= pk;
  }
  return impurity;
}

tuple<std::string, double> Calculations::determine_best_threshold(const Data& data, int col, const std::vector<size_t>& indexes, const ClassCounter& counter) {
  std::string best_thresh;
  double best_loss = std::numeric_limits<float>::infinity();

  //left and right branch
  ClassCounter left_branch = empty(counter);
  ClassCounter right_branch = copy(counter);

  //going linear through whole dataset, trying to find the best threshold to split the dataset
  for (int row = 1; row < indexes.size(); row++){
      size_t index = indexes[row-1];
      auto current_class = data[index][data[index].size()-1];// getting the class

      //updating the class counters as we go through the dataset
      left_branch[current_class] += 1;
      right_branch[current_class] -= 1;

      //skipping over the updating the loss, until we find a datapoint of different value
      size_t current_index = indexes[row];
      if (data[index][col] == data[current_index][col]) continue;

      double left_gini_index = gini(left_branch, row);
      double right_gini_index = gini(right_branch, indexes.size()-row);

      double current_gini = ((row+1)*left_gini_index + (indexes.size()-row-1)*right_gini_index)/indexes.size();

      if (current_gini < best_loss){
          best_loss = current_gini;
          best_thresh = data[current_index][col];
      }
  }

  return forward_as_tuple(best_thresh, best_loss);
}

const ClassCounter Calculations::classCounts(const Data& data, const std::vector<size_t>& indexes) {
  ClassCounter counter;
  for (const auto& index: indexes) {
    auto rows = data[index];
    const string decision = *std::rbegin(rows);
    if (counter.find(decision) != std::end(counter)) {
      counter.at(decision)++;
    } else {
      counter[decision] += 1;
    }
  }
  return counter;
}

const ClassCounter Calculations::copy(const ClassCounter &current) {
    ClassCounter counter;
    for (auto &it : current){
        counter[it.first] = it.second;
    }
    return counter;
}

const ClassCounter Calculations::empty(const ClassCounter &current) {
    ClassCounter counter;
    for (auto &it : current){
        counter[it.first] = 0;
    }
    return counter;
}
