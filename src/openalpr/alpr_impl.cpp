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

#include "alpr_impl.h"


AlprImpl::AlprImpl(const std::string country, const std::string runtimeDir)
{
  config = new Config(country, runtimeDir);
  plateDetector = new RegionDetector(config);
  stateIdentifier = new StateIdentifier(config);
  ocr = new OCR(config);
  
  this->detectRegion = DEFAULT_DETECT_REGION;
  this->topN = DEFAULT_TOPN;
  this->defaultRegion = "";
  
  if (config->opencl_enabled)
  {
    
    cv::ocl::PlatformsInfo platinfo;
    cv::ocl::getOpenCLPlatforms(platinfo);
    
    for (int i = 0; i < platinfo.size(); i++)
    {
	std::cout << platinfo[i]->platformName << std::endl;
    }
    
    cv::ocl::DevicesInfo devices;
    cv::ocl::getOpenCLDevices(devices, cv::ocl::CVCL_DEVICE_TYPE_CPU);
    
    for (int i = 0; i < devices.size(); i++)
      std:: cout << devices[i]->deviceName << std::endl;
    
    if (devices.size() > 0)
    {
      cv::ocl::setDevice(devices[0]);
    
      cout << "Using OpenCL Device: " << devices[0]->deviceName << endl;
    }
    else
    {
      cout << "OpenCL initialization failed.  Runtime drivers may not be installed." << endl;
    }
  }
}
AlprImpl::~AlprImpl()
{
  delete config;
  delete plateDetector;
  delete stateIdentifier;
  delete ocr;
}


std::vector<AlprResult> AlprImpl::recognize(cv::Mat img)
{
  timespec startTime;
  getTime(&startTime);
  
  vector<AlprResult> response;
  

  vector<Rect> plateRegions = plateDetector->detect(img);


  // Recognize.

  for (int i = 0; i < plateRegions.size(); i++)
  {
      timespec platestarttime;
      getTime(&platestarttime);
      
      LicensePlateCandidate lp(img, plateRegions[i], config);
      
      lp.recognize();

      
      if (lp.confidence > 10)
      {
	AlprResult plateResult;
	plateResult.region = defaultRegion;
	plateResult.regionConfidence = 0;
	
	for (int pointidx = 0; pointidx < 4; pointidx++)
	{
	  plateResult.plate_points[pointidx].x = (int) lp.plateCorners[pointidx].x;
	  plateResult.plate_points[pointidx].y = (int) lp.plateCorners[pointidx].y;
	}
	
	if (detectRegion)
	{
	  char statecode[4];
	  plateResult.regionConfidence = stateIdentifier->recognize(img, plateRegions[i], statecode);
	  if (plateResult.regionConfidence > 0)
	  {
	    plateResult.region = statecode;
	  }
	}
    
    
	ocr->performOCR(lp.charSegmenter->getThresholds(), lp.charSegmenter->characters);
	
	ocr->postProcessor->analyze(plateResult.region, topN);

	//plateResult.characters = ocr->postProcessor->bestChars;
	const vector<PPResult> ppResults = ocr->postProcessor->getResults();
	
	int bestPlateIndex = 0;
	
	for (int pp = 0; pp < ppResults.size(); pp++)
	{
	  if (pp >= topN)
	    break;
	  
	  // Set our "best plate" match to either the first entry, or the first entry with a postprocessor template match
	  if (bestPlateIndex == 0 && ppResults[pp].matchesTemplate)
	    bestPlateIndex = pp;
	  
	  if (ppResults[pp].letters.size() >= config->postProcessMinCharacters &&
	    ppResults[pp].letters.size() <= config->postProcessMaxCharacters)
	  {
	    AlprPlate aplate;
	    aplate.characters = ppResults[pp].letters;
	    aplate.overall_confidence = ppResults[pp].totalscore;
	    aplate.matches_template = ppResults[pp].matchesTemplate;
	    plateResult.topNPlates.push_back(aplate);
	  }
	}
	plateResult.result_count = plateResult.topNPlates.size();
	
	if (plateResult.topNPlates.size() > 0)
	  plateResult.bestPlate = plateResult.topNPlates[bestPlateIndex];
	
	timespec plateEndTime;
	getTime(&plateEndTime);
	plateResult.processing_time_ms = diffclock(platestarttime, plateEndTime);
	
	if (plateResult.result_count > 0)
	  response.push_back(plateResult);
	
	if (config->debugGeneral)
	{
	  rectangle(img, plateRegions[i], Scalar(0, 255, 0), 2);
	  for (int z = 0; z < 4; z++)
	    line(img, lp.plateCorners[z], lp.plateCorners[(z + 1) % 4], Scalar(255,0,255), 2);
	}
	
	
      }
      else
      {
	if (config->debugGeneral)
	  rectangle(img, plateRegions[i], Scalar(0, 0, 255), 2);
      }
      
  }

  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "Total Time to process image: " << diffclock(startTime, endTime) << "ms." << endl;
  }
  
  if (config->debugGeneral && config->debugShowImages)
  {
    displayImage(config, "Main Image", img);
    // Pause indefinitely until they press a key
    while ((char) cv::waitKey(50) == -1)
      {}
  }
  return response;
}

string AlprImpl::toJson(const vector< AlprResult > results)
{
  cJSON *root = cJSON_CreateArray();	
  
  for (int i = 0; i < results.size(); i++)
  {
    cJSON *resultObj = createJsonObj( &results[i] );
    cJSON_AddItemToArray(root, resultObj);
  }
  
  // Print the JSON object to a string and return
  char *out;
  out=cJSON_PrintUnformatted(root);
  cJSON_Delete(root);
  
  string response(out);
  
  free(out);
  return response;
}



cJSON* AlprImpl::createJsonObj(const AlprResult* result)
{
  cJSON *root, *coords;
  
  root=cJSON_CreateObject();	
  
  cJSON_AddStringToObject(root,"plate",		result->bestPlate.characters.c_str());
  cJSON_AddNumberToObject(root,"confidence",		result->bestPlate.overall_confidence);
  cJSON_AddNumberToObject(root,"matches_template",	result->bestPlate.matches_template);
  
  cJSON_AddStringToObject(root,"region",		result->region.c_str());
  cJSON_AddNumberToObject(root,"region_confidence",	result->regionConfidence);
  
  cJSON_AddNumberToObject(root,"processing_time_ms",	result->processing_time_ms);
  
  cJSON_AddItemToObject(root, "coordinates", 		coords=cJSON_CreateArray());
  for (int i=0;i<4;i++)
  {
    cJSON *coords_object;
    coords_object = cJSON_CreateObject();
    cJSON_AddNumberToObject(coords_object, "x",  result->plate_points[i].x);
    cJSON_AddNumberToObject(coords_object, "y",  result->plate_points[i].y);

    cJSON_AddItemToArray(coords, coords_object);
  }
  
  return root;
}


void AlprImpl::setDetectRegion(bool detectRegion)
{
  this->detectRegion = detectRegion;
}
void AlprImpl::setTopN(int topn)
{
  this->topN = topn;
}
void AlprImpl::setDefaultRegion(string region)
{
  this->defaultRegion = region;
}

