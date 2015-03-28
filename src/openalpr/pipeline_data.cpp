#include "pipeline_data.h"

using namespace cv;
using namespace std;

namespace alpr
{

  PipelineData::PipelineData(Mat colorImage, Rect regionOfInterest, Config* config)
  {
    Mat grayImg;
    if (colorImage.channels() > 2)
    {
      cvtColor(this->colorImg, grayImg, CV_BGR2GRAY);
    }
    else
    {
      grayImg = colorImage;
    }
    
    PipelineData(colorImage, grayImg, regionOfInterest, config);
  }
  
  PipelineData::PipelineData(Mat colorImage, Mat grayImg, Rect regionOfInterest, Config* config)
  {
    this->colorImg = colorImage;
    this->grayImg = grayImg;

    this->regionOfInterest = regionOfInterest;
    this->config = config;

    this->region_confidence = 0;

    plate_inverted = false;
  }

  PipelineData::~PipelineData()
  {
    clearThresholds();
  }

  void PipelineData::clearThresholds()
  {
    for (unsigned int i = 0; i < thresholds.size(); i++)
    {
      thresholds[i].release();
    }
    thresholds.clear();
  }

}
