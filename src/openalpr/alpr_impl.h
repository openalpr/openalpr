/*
 * Copyright (c) 2013 New Designs Unlimited, LLC
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


#ifndef OPENALPR_ALPRIMPL_H
#define OPENALPR_ALPRIMPL_H

#include <list>
#include <sstream>

#include "alpr.h"
#include "config.h"

#include "detection/detector.h"
#include "detection/detectorfactory.h"

#include "licenseplatecandidate.h"
#include "stateidentifier.h"
#include "segmentation/charactersegmenter.h"
#include "ocr.h"

#include "constants.h"

#include "cjson.h"

#include "pipeline_data.h"

#include <opencv2/core/core.hpp>
   

#include "support/tinythread.h"
#include "support/platform.h"

#define DEFAULT_TOPN 25
#define DEFAULT_DETECT_REGION false

#define ALPR_NULL_PTR 0


struct AlprFullDetails
{
  std::vector<PlateRegion> plateRegions;
  std::vector<AlprResult> results;
};

class AlprImpl
{

  public:
    AlprImpl(const std::string country, const std::string configFile = "", const std::string runtimeDir = "");
    virtual ~AlprImpl();

    AlprFullDetails recognizeFullDetails(cv::Mat img);
    AlprFullDetails recognizeFullDetails(cv::Mat img, std::vector<cv::Rect> regionsOfInterest);
    
    std::vector<AlprResult> recognize(std::string filepath, std::vector<AlprRegionOfInterest> regionsOfInterest);
    std::vector<AlprResult> recognize(std::vector<unsigned char> imageBuffer, std::vector<AlprRegionOfInterest> regionsOfInterest);
    std::vector<AlprResult> recognize(cv::Mat img, std::vector<cv::Rect> regionsOfInterest);
    
    void applyRegionTemplate(AlprResult* result, std::string region);
    
    void setDetectRegion(bool detectRegion);
    void setTopN(int topn);
    void setDefaultRegion(std::string region);
    
    std::string toJson(const std::vector<AlprResult> results, double processing_time_ms = -1);
    static std::string getVersion();
    
    Config* config;
    
    bool isLoaded();
    
  private:
    
    Detector* plateDetector;
    StateIdentifier* stateIdentifier;
    OCR* ocr;
  
    int topN;
    bool detectRegion;
    std::string defaultRegion;
    
    std::vector<cv::Rect> convertRects(std::vector<AlprRegionOfInterest> regionsOfInterest);
    
    cJSON* createJsonObj(const AlprResult* result);
};

class PlateDispatcher
{
  public:
    PlateDispatcher(std::vector<PlateRegion> plateRegions, cv::Mat* image, 
		    Config* config,
		    StateIdentifier* stateIdentifier,
		    OCR* ocr,
		    int topN, bool detectRegion, std::string defaultRegion) 
    {
      this->plateRegions = plateRegions;
      this->frame = image;
      
      this->config = config;
      this->stateIdentifier = stateIdentifier;
      this->ocr = ocr;
      this->topN = topN;
      this->detectRegion = detectRegion;
      this->defaultRegion = defaultRegion;
    }

    cv::Mat getImageCopy()
    {
      tthread::lock_guard<tthread::mutex> guard(mMutex);

      cv::Mat img(this->frame->size(), this->frame->type());
      this->frame->copyTo(img);
      
      return img;
    }

    
    bool nextPlate(PlateRegion* plateRegion)
    {
      tthread::lock_guard<tthread::mutex> guard(mMutex);
      
      if (plateRegions.size() == 0)
	return false;
      
      *plateRegion = plateRegions[plateRegions.size() - 1];
      plateRegions.pop_back();
      
      return true;
    }
    
    void appendPlate(PlateRegion plate)
    {
      tthread::lock_guard<tthread::mutex> guard(mMutex);
      
      plateRegions.push_back(plate);
    }
    
    void addResult(AlprResult recognitionResult)
    {
      tthread::lock_guard<tthread::mutex> guard(mMutex);
      recognitionResults.push_back(recognitionResult);
    }
    
    std::vector<AlprResult> getRecognitionResults()
    {
      return recognitionResults;
    }
    
    StateIdentifier* stateIdentifier;
    OCR* ocr;
    Config* config;
  
    uint topN;
    bool detectRegion;
    std::string defaultRegion;
    
    tthread::mutex ocrMutex;
    
  private:
    
    tthread::mutex mMutex;
    
    cv::Mat* frame;
    std::vector<PlateRegion> plateRegions;
    std::vector<AlprResult> recognitionResults;

};

#endif // OPENALPR_ALPRIMPL_H