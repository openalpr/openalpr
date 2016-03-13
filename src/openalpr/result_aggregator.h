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

#ifndef OPENALPR_RESULTAGGREGATOR_H
#define OPENALPR_RESULTAGGREGATOR_H


#include "alpr_impl.h"
#include "prewarp.h"

// Runs the analysis for multiple training sets, and aggregates the results into the best matches

struct PlateShapeInfo
{
  cv::Point2f center;
  float area;
  int max_width;
  int max_height;
};

namespace alpr
{

  enum ResultMergeStrategy
  {
    MERGE_COMBINE,  // Used when running an analysis multiple times for accuracy improvement.  Merges results together
    MERGE_PICK_BEST // Used when analyzing multiple countries.  Chooses results from one country or the other
  };
  
  struct ResultPlateScore
  {
    AlprPlate plate;
    float score_total;
    int count;
  };
  
  struct ResultRegionScore
  {
    std::string region;
    float confidence;
    
  };
  
  class ResultAggregator
  {
  public:
    ResultAggregator(ResultMergeStrategy merge_strategy, int topn, Config* config);

    virtual ~ResultAggregator();

    void addResults(AlprFullDetails full_results);

    AlprFullDetails getAggregateResults();

    cv::Mat applyImperceptibleChange(cv::Mat image, int index);
    
  private:
    
    int topn;
    PreWarp* prewarp;
    Config* config;
    
    std::vector<AlprFullDetails> all_results;

    PlateShapeInfo getShapeInfo(AlprPlateResult plate);

    ResultMergeStrategy merge_strategy;
    
    ResultRegionScore findBestRegion(std::vector<AlprPlateResult> cluster);
    
    std::vector<std::vector<AlprPlateResult> > findClusters();
    int overlaps(AlprPlateResult plate, std::vector<std::vector<AlprPlateResult> > clusters);
  };

}

#endif //OPENALPR_RESULTAGGREGATOR_H
