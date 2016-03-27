/*
 * Copyright (c) 2015 OpenALPR Technology, Inc.
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

#ifndef OPENALPR_ALPR_H
#define OPENALPR_ALPR_H

#include <iostream>
#include <vector>
#include <fstream> 
#include <stdint.h>

#ifdef WIN32
  #define OPENALPR_DLL_EXPORT __declspec( dllexport )
#else
  #define OPENALPR_DLL_EXPORT 
#endif

namespace alpr
{
  
  struct AlprCoordinate
  {
    int x;
    int y;
  };
  
  struct AlprChar
  {
    AlprCoordinate corners[4];
    float confidence;
    std::string character;
  };
  
  struct AlprPlate
  {
    std::string characters;
    float overall_confidence;

    std::vector<AlprChar> character_details;
    bool matches_template;
  };
  

  class AlprRegionOfInterest
  {
  public:
    AlprRegionOfInterest();
    AlprRegionOfInterest(int x, int y, int width, int height)
    {
      this->x = x;
      this->y = y;
      this->width = width;
      this->height = height;
    };

    int x;
    int y;
    int width;
    int height;
  };

  class AlprPlateResult
  {
    public:
      AlprPlateResult() {};
      virtual ~AlprPlateResult() {};

      // The number requested is always >= the topNPlates count
      int requested_topn;

      // The country (training data code) that was used to recognize the plate
      std::string country;
      
      // the best plate is the topNPlate with the highest confidence
      AlprPlate bestPlate;
      
      // A list of possible plate number permutations
      std::vector<AlprPlate> topNPlates;

      // The processing time for this plate
      float processing_time_ms;
      
      // the X/Y coordinates of the corners of the plate (clock-wise from top-left)
      AlprCoordinate plate_points[4];

      // The index of the plate if there were multiple plates returned
      int plate_index;
      
      // When region detection is enabled, this returns the region.  Region detection is experimental
      int regionConfidence;
      std::string region;
  };

  class AlprResults
  {
    public:
      AlprResults() {
        frame_number = -1;
      };
      virtual ~AlprResults() {};

      int64_t epoch_time;
      int64_t frame_number;
      int img_width;
      int img_height;
      float total_processing_time_ms;

      std::vector<AlprPlateResult> plates;

      std::vector<AlprRegionOfInterest> regionsOfInterest;

  };


  class Config;
  class AlprImpl;
  class OPENALPR_DLL_EXPORT Alpr
  {

    public:
      Alpr(const std::string country, const std::string configFile = "", const std::string runtimeDir = "");
      virtual ~Alpr();

      // Set the country used for plate recognition
      void setCountry(std::string country);
      
      // Update the prewarp setting without reloading the library
      void setPrewarp(std::string prewarp_config);
      // Update the detection mask without reloading the library
      void setMask(unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight);
      
      void setDetectRegion(bool detectRegion);
      void setTopN(int topN);
      void setDefaultRegion(std::string region);

      // Recognize from an image on disk
      AlprResults recognize(std::string filepath);

	  // Recognize from byte data representing an encoded image (e.g., BMP, PNG, JPG, GIF etc).
	  AlprResults recognize(std::vector<char> imageBytes);

	  // Recognize from byte data representing an encoded image (e.g., BMP, PNG, JPG, GIF etc).
	  AlprResults recognize(std::vector<char> imageBytes, std::vector<AlprRegionOfInterest> regionsOfInterest);

      // Recognize from raw pixel data.  
      AlprResults recognize(unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight, std::vector<AlprRegionOfInterest> regionsOfInterest);


      static std::string toJson(const AlprResults results);
      static AlprResults fromJson(std::string json);

      bool isLoaded();

      static std::string getVersion();

      Config* getConfig();

    private:
      AlprImpl* impl;
  };

}
#endif // OPENALPR_APLR_H
