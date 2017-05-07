/*
 * Copyright (c) 2016 OpenALPR Technology, Inc.
 * Open source Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenALPR.
 *
 * OpenALPR is free software: you can redistribute it and/or modify
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

#include "alpr_c.h"
#include <alpr.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
 
OPENALPRC_DLL_EXPORT OPENALPR* openalpr_init(const char* country, const char* configFile, const char* runtimeDir)
{
  alpr::Alpr* alpr_inst = new alpr::Alpr(country, configFile, runtimeDir);
  
  return (OPENALPR*) alpr_inst;
}

OPENALPRC_DLL_EXPORT int openalpr_is_loaded(OPENALPR* instance)
{
  return (int) ((alpr::Alpr*) instance)->isLoaded();
}

// Set the country used for plate recognition
OPENALPRC_DLL_EXPORT void openalpr_set_country(OPENALPR* instance, const char* country)
{
  ((alpr::Alpr*) instance)->setCountry(country);
}

// Update the prewarp setting without reloading the library
OPENALPRC_DLL_EXPORT void openalpr_set_prewarp(OPENALPR* instance, const char* prewarp_config)
{
  ((alpr::Alpr*) instance)->setPrewarp(prewarp_config);
}

// Update the detection mask without reloading the library
OPENALPRC_DLL_EXPORT void openalpr_set_mask(OPENALPR* instance, unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight)
{
  ((alpr::Alpr*) instance)->setMask(pixelData, bytesPerPixel, imgWidth, imgHeight);
}

// Enable/disable region detection.  Pass a 0 or 1
OPENALPRC_DLL_EXPORT void openalpr_set_detect_region(OPENALPR* instance, int detectRegion)
{
  ((alpr::Alpr*) instance)->setDetectRegion(detectRegion);
}

OPENALPRC_DLL_EXPORT void openalpr_set_topn(OPENALPR* instance, int topN)
{
  ((alpr::Alpr*) instance)->setTopN(topN);
}

OPENALPRC_DLL_EXPORT void openalpr_set_default_region(OPENALPR* instance, const char* region)
{
  ((alpr::Alpr*) instance)->setDefaultRegion(region);
}
     


// Recognizes the provided image and responds with JSON.  
// Caller must call free() on the returned object
OPENALPRC_DLL_EXPORT char* openalpr_recognize_rawimage(OPENALPR* instance, unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight, AlprCRegionOfInterest roi)
{
  std::vector<alpr::AlprRegionOfInterest> rois;
  alpr::AlprRegionOfInterest cpproi(roi.x, roi.y, roi.width, roi.height);
  rois.push_back(cpproi);
  
  alpr::AlprResults results = ((alpr::Alpr*) instance)->recognize(pixelData,bytesPerPixel, imgWidth, imgHeight, rois);
  std::string json_string = alpr::Alpr::toJson(results);
  
  char* result_obj = strdup(json_string.c_str());
  
  return result_obj;
  
}

OPENALPRC_DLL_EXPORT char* openalpr_recognize_encodedimage(OPENALPR* instance, unsigned char* bytes, long long length, AlprCRegionOfInterest roi)
{
  std::vector<alpr::AlprRegionOfInterest> rois;
  alpr::AlprRegionOfInterest cpproi(roi.x, roi.y, roi.width, roi.height);
  rois.push_back(cpproi);
  
  std::vector<char> byte_vector(length);
  memcpy(&byte_vector[0], bytes, length*sizeof(char));
  
  alpr::AlprResults results = ((alpr::Alpr*) instance)->recognize(byte_vector, rois);
  std::string json_string = alpr::Alpr::toJson(results);
  
  char* result_obj = strdup(json_string.c_str());
  
  return result_obj;
}


OPENALPRC_DLL_EXPORT void openalpr_free_response_string(char* response)
{
  free(response);
}

OPENALPRC_DLL_EXPORT void openalpr_cleanup(OPENALPR* instance)
{
  delete ((alpr::Alpr*) instance);
}