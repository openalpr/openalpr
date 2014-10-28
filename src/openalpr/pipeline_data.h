
#ifndef OPENALPR_PIPELINEDATA_H
#define OPENALPR_PIPELINEDATA_H

#include "opencv2/imgproc/imgproc.hpp"
#include "utility.h"
#include "config.h"
#include "textdetection/textline.h"

namespace alpr
{

  class PipelineData
  {

    public:
      PipelineData(cv::Mat colorImage, cv::Rect regionOfInterest, Config* config);
      virtual ~PipelineData();

      void clearThresholds();

      // Inputs
      Config* config;

      cv::Mat colorImg;
      cv::Mat grayImg;
      cv::Rect regionOfInterest;

      bool isMultiline;

      cv::Mat crop_gray;

      bool hasPlateBorder;
      cv::Mat plateBorderMask;    
      std::vector<TextLine> textLines;

      std::vector<cv::Mat> thresholds;

      std::vector<cv::Point2f> plate_corners;


      // Outputs
      bool plate_inverted;

      std::string region_code;
      float region_confidence;


      float plate_area_confidence;

      std::vector<cv::Rect> charRegions;




      // OCR

  };

}

#endif // OPENALPR_PIPELINEDATA_H