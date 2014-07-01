
#ifndef OPENALPR_PIPELINEDATA_H
#define OPENALPR_PIPELINEDATA_H

#include "segmentation/segment.h"
#include "segmentation/segmentationgroup.h"

class PipelineData
{

  public:
    PipelineData(cv::Mat colorImage, cv::Rect regionOfInterest, Config* config);
    virtual ~PipelineData();

    // Inputs
    Config* config;
    
    cv::Mat colorImg;
    cv::Mat grayImg;
    cv::Rect regionOfInterest;
    
    
    cv::Mat crop_gray;
    cv::Mat plate_mask;    
    
    // Outputs
    std::string region_code;
    float region_confidence;
    
    float overall_confidence;
    
    std::vector<cv::Mat> thresholds;
    
    
    // Plate Lines
    std::vector<LineSegment> horizontalLines;
    std::vector<LineSegment> verticalLines;

    // Segmentation
    std::vector<Segment> segments;
    std::vector<SegmentationGroup> segmentGroups;

    // OCR
    
};


#endif // OPENALPR_PIPELINEDATA_H