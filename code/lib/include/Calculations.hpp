/*
 * Copyright (c) DTAI - KU Leuven – All rights reserved.
 * Proprietary, do not copy or distribute without permission. 
 * Written by Pieter Robberechts, 2019
 */

#ifndef DECISIONTREE_CALCULATIONS_HPP
#define DECISIONTREE_CALCULATIONS_HPP

#include <tuple>
#include <vector>
#include <string>
#include <unordered_map>
#include <boost/timer/timer.hpp>
#include "Question.hpp"
#include "Utils.hpp"

using ClassCounter = std::unordered_map<std::string, int>;

namespace Calculations {

std::tuple<const std::vector<size_t>, const std::vector<size_t>> partition(const Data &data, const Question &q, const std::vector<size_t>& indexes); // changed so that it partitions indexes instead the data

const double gini(const ClassCounter& counts, double N);

std::tuple<const double, const Question> find_best_split(const Data &rows, const MetaData &meta, const std::vector<size_t>& indexes);

std::tuple<std::string, double> determine_best_threshold(const Data &data, int col, const std::vector<size_t>& indexes, const ClassCounter& counter);

const ClassCounter copy(const ClassCounter &counter); //used to make a copy of class counter

const ClassCounter empty(const ClassCounter &counter); // used to make an empty copy of class counter

const ClassCounter classCounts(const Data &data, const std::vector<size_t>& indexes);

} // namespace Calculations

#endif //DECISIONTREE_CALCULATIONS_HPP
