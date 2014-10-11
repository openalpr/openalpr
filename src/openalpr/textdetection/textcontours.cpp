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

#include "textcontours.h"

using namespace std;
using namespace cv;


TextContours::TextContours() {
  

}


TextContours::TextContours(cv::Mat threshold) {
  
  load(threshold);
}


TextContours::~TextContours() {
}

void TextContours::load(cv::Mat threshold) {

  Mat tempThreshold(threshold.size(), CV_8U);
  threshold.copyTo(tempThreshold);
  findContours(tempThreshold,
               contours, // a vector of contours
               hierarchy,
               CV_RETR_TREE, // retrieve all contours
               CV_CHAIN_APPROX_SIMPLE ); // all pixels of each contours

  for (uint i = 0; i < contours.size(); i++)
    goodIndices.push_back(true);
}


uint TextContours::size() {
  return contours.size();
}



int TextContours::getGoodIndicesCount()
{
  int count = 0;
  for (uint i = 0; i < goodIndices.size(); i++)
  {
    if (goodIndices[i])
      count++;
  }

  return count;
}


std::vector<bool> TextContours::getIndicesCopy()
{
  vector<bool> copyArray;
  for (uint i = 0; i < goodIndices.size(); i++)
  {
    bool val = goodIndices[i];
    copyArray.push_back(goodIndices[i]);
  }
  
  return copyArray;
}

void TextContours::setIndices(std::vector<bool> newIndices)
{
  if (newIndices.size() == goodIndices.size())
  {
    for (uint i = 0; i < newIndices.size(); i++)
      goodIndices[i] = newIndices[i];
  }
  else
  {
    assert("Invalid set operation on indices");
  }
}