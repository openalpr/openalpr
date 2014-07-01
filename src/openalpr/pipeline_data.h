
#ifndef OPENALPR_PIPELINEDATA_H
#define OPENALPR_PIPELINEDATA_H

#include "opencv2/imgproc/imgproc.hpp"
#include "utility.h"
#include "config.h"

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
    
    cv::Mat crop_gray;
    cv::Mat plate_mask;    
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


#endif // OPENALPR_PIPELINEDATA_H