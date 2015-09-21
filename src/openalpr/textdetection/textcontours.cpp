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

#include "textcontours.h"

using namespace std;
using namespace cv;

namespace alpr
{

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

    for (unsigned int i = 0; i < contours.size(); i++)
      goodIndices.push_back(true);

    this->width = threshold.cols;
    this->height = threshold.rows;
  }


  unsigned int TextContours::size() {
    return contours.size();
  }



  int TextContours::getGoodIndicesCount()
  {
    int count = 0;
    for (unsigned int i = 0; i < goodIndices.size(); i++)
    {
      if (goodIndices[i])
        count++;
    }

    return count;
  }


  std::vector<bool> TextContours::getIndicesCopy()
  {
    vector<bool> copyArray;
    for (unsigned int i = 0; i < goodIndices.size(); i++)
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
      for (unsigned int i = 0; i < newIndices.size(); i++)
        goodIndices[i] = newIndices[i];
    }
    else
    {
      assert("Invalid set operation on indices");
    }
  }

  Mat TextContours::drawDebugImage() const {

      Mat img_contours = Mat::zeros(Size(width, height), CV_8U);

      return drawDebugImage(img_contours);
  }

  Mat TextContours::drawDebugImage(Mat baseImage) const {
      Mat img_contours(baseImage.size(), CV_8U);
      baseImage.copyTo(img_contours);

      cvtColor(img_contours, img_contours, CV_GRAY2RGB);

      vector<vector<Point> > allowedContours;
      for (unsigned int i = 0; i < this->contours.size(); i++)
      {
        if (this->goodIndices[i])
          allowedContours.push_back(this->contours[i]);
      }

      drawContours(img_contours, this->contours,
                   -1, // draw all contours
                   cv::Scalar(255,0,0), // in blue
                   1); // with a thickness of 1

      drawContours(img_contours, allowedContours,
                   -1, // draw all contours
                   cv::Scalar(0,255,0), // in green
                   1); // with a thickness of 1


      return img_contours;
  }

}

