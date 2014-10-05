/*
 * Copyright (c) 2014 New Designs Unlimited, LLC
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
#include <vector>
#include <queue>

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
    
    std::string toJson(const std::vector<AlprResult> results, double processing_time_ms = -1, long epoch_time = -1);
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



#endif // OPENALPR_ALPRIMPL_H