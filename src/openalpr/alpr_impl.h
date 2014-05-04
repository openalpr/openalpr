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

#include "regiondetector.h"
#include "licenseplatecandidate.h"
#include "stateidentifier.h"
#include "charactersegmenter.h"
#include "ocr.h"

#include "constants.h"

#include "cjson.h"

#include <opencv2/core/core.hpp>
#include "opencv2/ocl/ocl.hpp"
   

#include "tinythread/tinythread.h"

#define DEFAULT_TOPN 25
#define DEFAULT_DETECT_REGION false

class AlprImpl
{

  public:
    AlprImpl(const std::string country, const std::string runtimeDir = "");
    virtual ~AlprImpl();

    std::vector<AlprResult> recognize(cv::Mat img);
    
    void applyRegionTemplate(AlprResult* result, std::string region);
    
    void setDetectRegion(bool detectRegion);
    void setTopN(int topn);
    void setDefaultRegion(string region);
    
    std::string toJson(const vector<AlprResult> results);
    static std::string getVersion();
    
    Config* config;
    
  private:
    
    RegionDetector* plateDetector;
    StateIdentifier* stateIdentifier;
    OCR* ocr;
  
    int topN;
    bool detectRegion;
    std::string defaultRegion;
    
    cJSON* createJsonObj(const AlprResult* result);
};

class PlateDispatcher
{
  public:
    PlateDispatcher(vector<PlateRegion> plateRegions, cv::Mat* image, 
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

      Mat img(this->frame->size(), this->frame->type());
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
    
    vector<AlprResult> getRecognitionResults()
    {
      return recognitionResults;
    }
    

    StateIdentifier* stateIdentifier;
    OCR* ocr;
    Config* config;
  
    int topN;
    bool detectRegion;
    std::string defaultRegion;
    
  private:
    
    tthread::mutex mMutex;
    cv::Mat* frame;
    vector<PlateRegion> plateRegions;
    vector<AlprResult> recognitionResults;

};

#endif // OPENALPR_ALPRIMPL_H