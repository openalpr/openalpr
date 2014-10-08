/*
 * Copyright (c) 2014 New Designs Unlimited, LLC
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

#include "characteranalysis2l.h"

using namespace cv;
using namespace std;

CharacterAnalysis2L::CharacterAnalysis2L(PipelineData* pipeline_data) {
  this->pipeline_data = pipeline_data;
}


CharacterAnalysis2L::~CharacterAnalysis2L() {
}

void CharacterAnalysis2L::analyze() {

  pipeline_data->clearThresholds();
  pipeline_data->thresholds = produceThresholds(pipeline_data->crop_gray, pipeline_data->config);

  std::vector<std::vector<std::vector<cv::Point> > > allContours;
  std::vector<std::vector<cv::Vec4i> > allHierarchy;

  timespec startTime;
  getTime(&startTime);

  for (uint i = 0; i < pipeline_data->thresholds.size(); i++)
  {
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    Mat tempThreshold(pipeline_data->thresholds[i].size(), CV_8U);
    pipeline_data->thresholds[i].copyTo(tempThreshold);
    findContours(tempThreshold,
                 contours, // a vector of contours
                 hierarchy,
                 CV_RETR_TREE, // retrieve all contours
                 CV_CHAIN_APPROX_SIMPLE ); // all pixels of each contours

    allContours.push_back(contours);
    allHierarchy.push_back(hierarchy);
  }
}
