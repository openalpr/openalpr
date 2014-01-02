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


#ifndef ALPRIMPL_H
#define ALPRIMPL_H

#include "alpr.h"
#include "config.h"

#include "regiondetector.h"
#include "licenseplatecandidate.h"
#include "stateidentifier.h"
#include "charactersegmenter.h"
#include "ocr.h"

#include "cjson.h"

#include <opencv2/core/core.hpp>


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

#endif // ALPRIMPL_H