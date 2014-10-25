/* 
 * File:   textlinecollection.h
 * Author: mhill
 *
 * Created on October 25, 2014, 4:06 PM
 */

#ifndef OPENALPR_TEXTLINECOLLECTION_H
#define	OPENALPR_TEXTLINECOLLECTION_H

#include "utility.h"

#include "opencv2/imgproc/imgproc.hpp"
#include "textdetection/textline.h"


class TextLineCollection
{
public:
  TextLineCollection(std::vector<TextLine> textLines);
  
  int isLeftOfText(LineSegment line);
  int isAboveText(LineSegment line);
  
  LineSegment centerHorizontalLine;
  LineSegment centerVerticalLine;

  float charHeight;
  float charAngle;
  
  cv::Mat getDebugImage(cv::Size imageSize);
  
private:
  
  LineSegment topCharArea;
  LineSegment bottomCharArea;
  
  LineSegment longerSegment;
  LineSegment shorterSegment;
    
  cv::Mat textMask;
  
  void findCenterHorizontal();
  void findCenterVertical();
};

#endif	/* OPENALPR_TEXTLINECOLLECTION_H */

