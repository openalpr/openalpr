/*
 * Copyright (c) 2015 New Designs Unlimited, LLC
 * Opensource Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenAlpr.
 *
 * OpenAlpr is free software: you can redistribute it and/or modify
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

#ifndef OPENALPR_SCOREKEEPER_H
#define	OPENALPR_SCOREKEEPER_H

#include <string>
#include <iostream>
#include <iomanip>

namespace alpr
{

  class ScoreKeeper {
  public:
    ScoreKeeper();
    virtual ~ScoreKeeper();

    void setScore(std::string weight_id, float score, float weight);

    float getTotal();

    void printDebugScores();

  private:

    std::vector<std::string> weight_ids;
    std::vector<float> weights;

    std::vector<float> scores;

  };

}

#endif	/* OPENALPR_SCOREKEEPER_H */

