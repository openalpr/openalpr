/* 
 * File:   textcontours.cpp
 * Author: mhill
 * 
 * Created on October 9, 2014, 7:40 PM
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