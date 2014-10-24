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

#include "alpr_impl.h"

void plateAnalysisThread(void* arg);

using namespace std;
using namespace cv;

AlprImpl::AlprImpl(const std::string country, const std::string configFile, const std::string runtimeDir)
{
  config = new Config(country, configFile, runtimeDir);
  
  // Config file or runtime dir not found.  Don't process any further.
  if (config->loaded == false)
  {
    plateDetector = ALPR_NULL_PTR;
    stateIdentifier = ALPR_NULL_PTR;
    ocr = ALPR_NULL_PTR;
    return;
  }
  
  plateDetector = createDetector(config);
  stateIdentifier = new StateIdentifier(config);
  ocr = new OCR(config);
  setNumThreads(0);
  
  this->detectRegion = DEFAULT_DETECT_REGION;
  this->topN = DEFAULT_TOPN;
  this->defaultRegion = "";
  
}
AlprImpl::~AlprImpl()
{
  delete config;
  
  if (plateDetector != ALPR_NULL_PTR)
    delete plateDetector;
  
  if (stateIdentifier != ALPR_NULL_PTR)
    delete stateIdentifier;
  
  if (ocr != ALPR_NULL_PTR)
    delete ocr;
}

bool AlprImpl::isLoaded()
{
  return config->loaded;
}

AlprFullDetails AlprImpl::recognizeFullDetails(cv::Mat img)
{
  std::vector<cv::Rect> regionsOfInterest;
  regionsOfInterest.push_back(cv::Rect(0, 0, img.cols, img.rows));
  
  return this->recognizeFullDetails(img, regionsOfInterest);
}

AlprFullDetails AlprImpl::recognizeFullDetails(cv::Mat img, std::vector<cv::Rect> regionsOfInterest)
{
  timespec startTime;
  getTime(&startTime);
  
  if (regionsOfInterest.size() == 0)
    regionsOfInterest.push_back(cv::Rect(0, 0, img.cols, img.rows));
  
  AlprFullDetails response;
  
  response.results.epoch_time = getEpochTime();
  response.results.img_width = img.cols;
  response.results.img_height = img.rows;
  
  for (uint i = 0; i < regionsOfInterest.size(); i++)
  {
    response.results.regionsOfInterest.push_back(AlprRegionOfInterest(regionsOfInterest[i].x, regionsOfInterest[i].y, 
            regionsOfInterest[i].width, regionsOfInterest[i].height));
  }
  
  if (!img.data)
  {
    // Invalid image
    if (this->config->debugGeneral)
      std::cerr << "Invalid image" << std::endl;
    
    return response;
  }

  // Find all the candidate regions
  response.plateRegions = plateDetector->detect(img, regionsOfInterest);

  queue<PlateRegion> plateQueue;
  for (uint i = 0; i < response.plateRegions.size(); i++)
    plateQueue.push(response.plateRegions[i]);
  
  while(!plateQueue.empty())
  {
    PlateRegion plateRegion = plateQueue.front();
    plateQueue.pop();

    PipelineData pipeline_data(img, plateRegion.rect, config);

    timespec platestarttime;
    getTime(&platestarttime);

    LicensePlateCandidate lp(&pipeline_data);

    lp.recognize();

    bool plateDetected = false;
    if (pipeline_data.plate_area_confidence > 10)
    {
      AlprPlateResult plateResult;
      plateResult.region = defaultRegion;
      plateResult.regionConfidence = 0;

      for (int pointidx = 0; pointidx < 4; pointidx++)
      {
        plateResult.plate_points[pointidx].x = (int) pipeline_data.plate_corners[pointidx].x;
        plateResult.plate_points[pointidx].y = (int) pipeline_data.plate_corners[pointidx].y;
      }

      if (detectRegion)
      {
        stateIdentifier->recognize(&pipeline_data);
        if (pipeline_data.region_confidence > 0)
        {
          plateResult.region = pipeline_data.region_code;
          plateResult.regionConfidence = (int) pipeline_data.region_confidence;
        }
      }



      ocr->performOCR(&pipeline_data);
      ocr->postProcessor.analyze(plateResult.region, topN);
      const vector<PPResult> ppResults = ocr->postProcessor.getResults();


      int bestPlateIndex = 0;

      for (uint pp = 0; pp < ppResults.size(); pp++)
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

      if (plateResult.topNPlates.size() > 0)
        plateResult.bestPlate = plateResult.topNPlates[bestPlateIndex];

      timespec plateEndTime;
      getTime(&plateEndTime);
      plateResult.processing_time_ms = diffclock(platestarttime, plateEndTime);

      if (plateResult.topNPlates.size() > 0)
      {
        plateDetected = true;
        response.results.plates.push_back(plateResult);
      }
    }
    
    if (!plateDetected)
    {
      // Not a valid plate
      // Check if this plate has any children, if so, send them back up for processing
      for (uint childidx = 0; childidx < plateRegion.children.size(); childidx++)
      {
        plateQueue.push(plateRegion.children[childidx]);
      }
    }
  
      
      
      
  }

  timespec endTime;
  getTime(&endTime);
  response.results.total_processing_time_ms = diffclock(startTime, endTime);
  
  if (config->debugTiming)
  {
    cout << "Total Time to process image: " << diffclock(startTime, endTime) << "ms." << endl;
  }
  
  if (config->debugGeneral && config->debugShowImages)
  {
    for (uint i = 0; i < response.plateRegions.size(); i++)
    {
      rectangle(img, response.plateRegions[i].rect, Scalar(0, 0, 255), 2);
    }
    
    for (uint i = 0; i < response.results.plates.size(); i++)
    {
      for (int z = 0; z < 4; z++)
      {
	AlprCoordinate* coords = response.results.plates[i].plate_points;
	Point p1(coords[z].x, coords[z].y);
	Point p2(coords[(z + 1) % 4].x, coords[(z + 1) % 4].y);
	line(img, p1, p2, Scalar(255,0,255), 2);
      }
    }

    
    displayImage(config, "Main Image", img);
    
    // Sleep 1ms
    sleep_ms(1);
    
  }
  
  
  if (config->debugPauseOnFrame)
  {
    // Pause indefinitely until they press a key
    while ((char) cv::waitKey(50) == -1)
    {}
  }
  
  return response;
}



AlprResults AlprImpl::recognize( std::vector<char> imageBytes, std::vector<AlprRegionOfInterest> regionsOfInterest )
{  
  cv::Mat img = cv::imdecode(cv::Mat(imageBytes), 1);
  
  return this->recognize(img, this->convertRects(regionsOfInterest));
}

AlprResults AlprImpl::recognize( unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight, std::vector<AlprRegionOfInterest> regionsOfInterest)
{
  
  int arraySize = imgWidth * imgHeight * bytesPerPixel;
  cv::Mat imgData = cv::Mat(arraySize, 1, CV_8U, pixelData);
  cv::Mat img = imgData.reshape(bytesPerPixel, imgHeight);
  
  if (regionsOfInterest.size() == 0)
  {
    AlprRegionOfInterest fullFrame(0,0, img.cols, img.rows);

    regionsOfInterest.push_back(fullFrame);
  }
  
  return this->recognize(img, this->convertRects(regionsOfInterest));
}

AlprResults AlprImpl::recognize(cv::Mat img, std::vector<cv::Rect> regionsOfInterest)
{
  
  AlprFullDetails fullDetails = recognizeFullDetails(img, regionsOfInterest);
  return fullDetails.results;
}


 std::vector<cv::Rect> AlprImpl::convertRects(std::vector<AlprRegionOfInterest> regionsOfInterest)
 {
   std::vector<cv::Rect> rectRegions;
   for (uint i = 0; i < regionsOfInterest.size(); i++)
   {
     rectRegions.push_back(cv::Rect(regionsOfInterest[i].x, regionsOfInterest[i].y, regionsOfInterest[i].width, regionsOfInterest[i].height));
   }
   
   return rectRegions;
 }

string AlprImpl::toJson( const AlprResults results )
{
  cJSON *root, *jsonResults;
  root = cJSON_CreateObject();
  
  
  cJSON_AddNumberToObject(root,"epoch_time",	results.epoch_time	  );
  cJSON_AddNumberToObject(root,"version",	2	  );
  cJSON_AddNumberToObject(root,"img_width",	results.img_width	  );
  cJSON_AddNumberToObject(root,"img_height",	results.img_height	  );
  cJSON_AddNumberToObject(root,"processing_time_ms", results.total_processing_time_ms );

  // Add the regions of interest to the JSON
  cJSON *rois;
  cJSON_AddItemToObject(root, "regions_of_interest", 		rois=cJSON_CreateArray());
  for (uint i=0;i<results.regionsOfInterest.size();i++)
  {
    cJSON *roi_object;
    roi_object = cJSON_CreateObject();
    cJSON_AddNumberToObject(roi_object, "x",  results.regionsOfInterest[i].x);
    cJSON_AddNumberToObject(roi_object, "y",  results.regionsOfInterest[i].y);
    cJSON_AddNumberToObject(roi_object, "width",  results.regionsOfInterest[i].width);
    cJSON_AddNumberToObject(roi_object, "height",  results.regionsOfInterest[i].height);

    cJSON_AddItemToArray(rois, roi_object);
  }
  
  
  cJSON_AddItemToObject(root, "results", 		jsonResults=cJSON_CreateArray());
  for (uint i = 0; i < results.plates.size(); i++)
  {
    cJSON *resultObj = createJsonObj( &results.plates[i] );
    cJSON_AddItemToArray(jsonResults, resultObj);
  }
  
  // Print the JSON object to a string and return
  char *out;
  out=cJSON_PrintUnformatted(root);
  
  cJSON_Delete(root);
  
  string response(out);
  
  free(out);
  return response;
}



cJSON* AlprImpl::createJsonObj(const AlprPlateResult* result)
{
  cJSON *root, *coords, *candidates;
  
  root=cJSON_CreateObject();	
  
  cJSON_AddStringToObject(root,"plate",		result->bestPlate.characters.c_str());
  cJSON_AddNumberToObject(root,"confidence",		result->bestPlate.overall_confidence);
  cJSON_AddNumberToObject(root,"matches_template",	result->bestPlate.matches_template);
  
  cJSON_AddStringToObject(root,"region",		result->region.c_str());
  cJSON_AddNumberToObject(root,"region_confidence",	result->regionConfidence);
  
  cJSON_AddNumberToObject(root,"processing_time_ms",	result->processing_time_ms);
  cJSON_AddNumberToObject(root,"requested_topn",	result->requested_topn);
  
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
  for (uint i = 0; i < result->topNPlates.size(); i++)
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

AlprResults AlprImpl::fromJson(std::string json) {
  AlprResults allResults;
  
  cJSON* root = cJSON_Parse(json.c_str());
  
  int version = cJSON_GetObjectItem(root, "version")->valueint;
  allResults.epoch_time = (long) cJSON_GetObjectItem(root, "epoch_time")->valuedouble; 
  allResults.img_width = cJSON_GetObjectItem(root, "img_width")->valueint;
  allResults.img_height = cJSON_GetObjectItem(root, "img_height")->valueint;
  allResults.total_processing_time_ms = cJSON_GetObjectItem(root, "processing_time_ms")->valueint;

  
  cJSON* rois = cJSON_GetObjectItem(root,"regions_of_interest");
  int numRois = cJSON_GetArraySize(rois);
  for (int c = 0; c < numRois; c++)
  {
    cJSON* roi = cJSON_GetArrayItem(rois, c);
    int x = cJSON_GetObjectItem(roi, "x")->valueint;
    int y = cJSON_GetObjectItem(roi, "y")->valueint;
    int width = cJSON_GetObjectItem(roi, "width")->valueint;
    int height = cJSON_GetObjectItem(roi, "height")->valueint;

    AlprRegionOfInterest alprRegion(x,y,width,height);
    allResults.regionsOfInterest.push_back(alprRegion);
  }
  
  cJSON* resultsArray = cJSON_GetObjectItem(root,"results");
  int resultsSize = cJSON_GetArraySize(resultsArray);
  
  for (int i = 0; i < resultsSize; i++)
  {
    cJSON* item = cJSON_GetArrayItem(resultsArray, i);
    AlprPlateResult plate;
    
    //plate.bestPlate = cJSON_GetObjectItem(item, "plate")->valuestring; 
    plate.processing_time_ms = cJSON_GetObjectItem(item, "processing_time_ms")->valuedouble;
    plate.region = cJSON_GetObjectItem(item, "region")->valuestring;
    plate.regionConfidence = cJSON_GetObjectItem(item, "region_confidence")->valueint;
    plate.requested_topn = cJSON_GetObjectItem(item, "requested_topn")->valueint;
    
    
    cJSON* coordinates = cJSON_GetObjectItem(item,"coordinates");
    for (int c = 0; c < 4; c++)
    {
      cJSON* coordinate = cJSON_GetArrayItem(coordinates, c);
      AlprCoordinate alprcoord;
      alprcoord.x = cJSON_GetObjectItem(coordinate, "x")->valueint;
      alprcoord.y = cJSON_GetObjectItem(coordinate, "y")->valueint;
      
      plate.plate_points[c] = alprcoord;
    }
    
    cJSON* candidates = cJSON_GetObjectItem(item,"candidates");
    int numCandidates = cJSON_GetArraySize(candidates);
    for (int c = 0; c < numCandidates; c++)
    {
      cJSON* candidate = cJSON_GetArrayItem(candidates, c);
      AlprPlate plateCandidate;
      plateCandidate.characters = cJSON_GetObjectItem(candidate, "plate")->valuestring;
      plateCandidate.overall_confidence = cJSON_GetObjectItem(candidate, "confidence")->valuedouble;
      plateCandidate.matches_template = (cJSON_GetObjectItem(candidate, "matches_template")->valueint) != 0;

      plate.topNPlates.push_back(plateCandidate);
      
      if (c == 0)
      {
        plate.bestPlate = plateCandidate;
      }
    }
    
    allResults.plates.push_back(plate);
  }
  
  
  cJSON_Delete(root);
  
  
  return allResults;
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

std::string AlprImpl::getVersion()
{
  std::stringstream ss;
  
  ss << OPENALPR_MAJOR_VERSION << "." << OPENALPR_MINOR_VERSION << "." << OPENALPR_PATCH_VERSION;
  return ss.str();
}

