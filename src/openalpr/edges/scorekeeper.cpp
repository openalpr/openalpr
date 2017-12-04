/*
 * Copyright (c) 2015 OpenALPR Technology, Inc.
 * Open source Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenALPR.
 *
 * OpenALPR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License
 * version 3 as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iomanip>
#include <iostream>

#include "scorekeeper.h"

namespace alpr
{

  ScoreKeeper::ScoreKeeper() {
  }


  ScoreKeeper::~ScoreKeeper() {
  }

  void ScoreKeeper::setScore(std::string weight_id, float score, float weight) {

    // Assume that we never set this value twice
    weight_ids.push_back(weight_id);
    scores.push_back(score);
    weights.push_back(weight);
  }


  float ScoreKeeper::getTotal() {

    float score = 0;

    for (unsigned int i = 0; i < weights.size(); i++)
    {
      score += scores[i] * weights[i];
    }

    return score;
  }
  
  
  int ScoreKeeper::size() {
    return weight_ids.size();
  }


  void ScoreKeeper::printDebugScores() {

    int longest_weight_id = 0;
    for (unsigned int i = 0; i < weight_ids.size(); i++)
    {
      if (weight_ids[i].length() > longest_weight_id)
        longest_weight_id = weight_ids[i].length();
    }

    float total = getTotal();

    std::cout << "--------------------" << std::endl;
    std::cout << "Total: " << total << std::endl;
    for (unsigned int i = 0; i < weight_ids.size(); i++)
    {
      float percent_of_total = (scores[i] * weights[i]) / total * 100;

      std::cout << "   - " << std::setw(longest_weight_id + 1) << std::left << weight_ids[i] << 
              " Weighted Score: " << std::setw(10) << std::left << (scores[i] * weights[i]) << 
              " Orig Score: " << std::setw(10) << std::left << scores[i] << 
              " (" << percent_of_total << "% of total)" << std::endl;
    }

    std::cout << "--------------------" << std::endl;
  }
  
}