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

void plateAnalysisThread(void* arg);

AlprImpl::AlprImpl(const std::string country, const std::string runtimeDir)
{
  config = new Config(country, runtimeDir);
  plateDetector = new RegionDetector(config);
  stateIdentifier = new StateIdentifier(config);
  ocr = new OCR(config);
  setNumThreads(0);
  
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
  


  // Find all the candidate regions
  vector<Rect> plateRegions = plateDetector->detect(img);

  // Get the number of threads specified and make sure the value is sane (cannot be greater than CPU cores or less than 1)
  int numThreads = config->multithreading_cores;
  if (numThreads > tthread::thread::hardware_concurrency())
    numThreads = tthread::thread::hardware_concurrency();
  if (numThreads <= 0)
    numThreads = 1;


  PlateDispatcher dispatcher(plateRegions, &img, 
			     config, stateIdentifier, ocr, 
			     topN, detectRegion, defaultRegion);
    
  // Spawn n threads to process all of the candidate regions and recognize
  list<tthread::thread*> threads;
  for (int i = 0; i < numThreads; i++)
  {
    tthread::thread * t = new tthread::thread(plateAnalysisThread, (void *) &dispatcher);
    threads.push_back(t);
  }
  
  // Wait for all threads to finish
  for(list<tthread::thread *>::iterator i = threads.begin(); i != threads.end(); ++ i)
  {
    tthread::thread* t = *i;
    t->join();
    delete t;
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
  
  //	if (config->debugGeneral)
//	{
//	  rectangle(img, plateRegion, Scalar(0, 255, 0), 2);
//	  for (int z = 0; z < 4; z++)
//	    line(img, lp.plateCorners[z], lp.plateCorners[(z + 1) % 4], Scalar(255,0,255), 2);
//	}

//	if (config->debugGeneral)
//	  rectangle(img, plateRegion, Scalar(0, 0, 255), 2);
  
  return dispatcher.getRecognitionResults();
}

void plateAnalysisThread(void* arg)
{
  PlateDispatcher* dispatcher = (PlateDispatcher*) arg;
  if (dispatcher->config->debugGeneral)
    cout << "Thread: " << tthread::this_thread::get_id() << " Initialized" << endl;
  
  int loop_count = 0;
  while (true)
  {
    
    if (dispatcher->hasPlate() == false)
      break;
    
    // Synchronized section
    if (dispatcher->config->debugGeneral)
      cout << "Thread: " << tthread::this_thread::get_id() << " loop " << ++loop_count << endl;
    
    // Get a single plate region from the queue
    Rect plateRegion = dispatcher->nextPlate();
    
    Mat img = dispatcher->getImageCopy();

  
    // Parallel section
      timespec platestarttime;
      getTime(&platestarttime);
      
      LicensePlateCandidate lp(img, plateRegion, dispatcher->config);
      
      lp.recognize();

      
      if (lp.confidence > 10)
      {
	AlprResult plateResult;
	plateResult.region = dispatcher->defaultRegion;
	plateResult.regionConfidence = 0;
	
	for (int pointidx = 0; pointidx < 4; pointidx++)
	{
	  plateResult.plate_points[pointidx].x = (int) lp.plateCorners[pointidx].x;
	  plateResult.plate_points[pointidx].y = (int) lp.plateCorners[pointidx].y;
	}
	
	if (dispatcher->detectRegion)
	{
	  char statecode[4];
	  plateResult.regionConfidence = dispatcher->stateIdentifier->recognize(img, plateRegion, statecode);
	  if (plateResult.regionConfidence > 0)
	  {
	    plateResult.region = statecode;
	  }
	}
    
    
	dispatcher->ocr->performOCR(lp.charSegmenter->getThresholds(), lp.charSegmenter->characters);
	
	dispatcher->ocr->postProcessor->analyze(plateResult.region, dispatcher->topN);

	//plateResult.characters = ocr->postProcessor->bestChars;
	const vector<PPResult> ppResults = dispatcher->ocr->postProcessor->getResults();
	
	int bestPlateIndex = 0;
	
	for (int pp = 0; pp < ppResults.size(); pp++)
	{
	  if (pp >= dispatcher->topN)
	    break;
	  
	  // Set our "best plate" match to either the first entry, or the first entry with a postprocessor template match
	  if (bestPlateIndex == 0 && ppResults[pp].matchesTemplate)
	    bestPlateIndex = pp;
	  
	  if (ppResults[pp].letters.size() >= dispatcher->config->postProcessMinCharacters &&
	    ppResults[pp].letters.size() <= dispatcher->config->postProcessMaxCharacters)
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
	{
	  // Synchronized section
	  dispatcher->addResult(plateResult);
	  
	}
	
	
      }
      
      if (dispatcher->config->debugTiming)
      {
	timespec plateEndTime;
	getTime(&plateEndTime);
	cout << "Thread: " << tthread::this_thread::get_id() << " Finished loop " << loop_count << " in " << diffclock(platestarttime, plateEndTime) << "ms." << endl;
      }
      
  }

  if (dispatcher->config->debugGeneral)
    cout << "Thread: " << tthread::this_thread::get_id() << " Complete" << endl;
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
  cJSON *root, *coords, *candidates;
  
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
  
  
  cJSON_AddItemToObject(root, "candidates", 		candidates=cJSON_CreateArray());
  for (int i = 0; i < result->topNPlates.size(); i++)
  {
    cJSON *candidate_object;
    candidate_object = cJSON_CreateObject();
    cJSON_AddStringToObject(candidate_object, "plate",  result->topNPlates[i].characters.c_str());
    cJSON_AddNumberToObject(candidate_object, "confidence",  result->topNPlates[i].overall_confidence);
    cJSON_AddNumberToObject(candidate_object, "matches_template",  result->topNPlates[i].matches_template);

    cJSON_AddItemToArray(candidates, candidate_object);
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

