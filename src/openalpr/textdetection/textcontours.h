/* 
 * File:   textcontours.h
 * Author: mhill
 *
 * Created on October 9, 2014, 7:40 PM
 */

#ifndef TEXTCONTOURS_H
#define	TEXTCONTOURS_H

#include <vector>
#include "opencv2/imgproc/imgproc.hpp"

class TextContours {
public:
  TextContours();
  TextContours(cv::Mat threshold);
  virtual ~TextContours();
  
  void load(cv::Mat threshold);
  
  std::vector<bool> goodIndices;
  std::vector<std::vector<cv::Point> > contours;
  std::vector<cv::Vec4i> hierarchy;
  
  uint size();
  int getGoodIndicesCount();
  
  std::vector<bool> getIndicesCopy();
  void setIndices(std::vector<bool> newIndices);
  
private:
  

};

#endif	/* TEXTCONTOURS_H */

