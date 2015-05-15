
#ifndef OPENALPR_MOTIONDETECTOR_H
#define OPENALPR_MOTIONDETECTOR_H

#include "opencv2/opencv.hpp"
#include "utility.h"

namespace alpr
{
  class MotionDetector
  {
      private: cv::Ptr<cv::BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
      private: cv::Mat fgMaskMOG2;
      public:
          MotionDetector();
          virtual ~MotionDetector();

          void ResetMotionDetection(cv::Mat* frame);
          cv::Rect MotionDetect(cv::Mat* frame);
  };
}

#endif // OPENALPR_MOTIONDETECTOR_H