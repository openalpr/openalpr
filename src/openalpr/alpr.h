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

#ifndef OPENALPR_ALPR_H
#define OPENALPR_ALPR_H

#include <iostream>
#include <vector>
#include <fstream> 

namespace alpr
{
  
  struct AlprPlate
  {
    std::string characters;
    float overall_confidence;

    bool matches_template;
  };

  struct AlprCoordinate
  {
    int x;
    int y;
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

      int requested_topn;

      AlprPlate bestPlate;
      std::vector<AlprPlate> topNPlates;

      float processing_time_ms;
      AlprCoordinate plate_points[4];

      int regionConfidence;
      std::string region;
  };

  class AlprResults
  {
    public:
      AlprResults() {};
      virtual ~AlprResults() {};

      long epoch_time;
      int img_width;
      int img_height;
      float total_processing_time_ms;

      std::vector<AlprPlateResult> plates;

      std::vector<AlprRegionOfInterest> regionsOfInterest;

  };


  class AlprImpl;
  class Alpr
  {

    public:
      Alpr(const std::string country, const std::string configFile = "", const std::string runtimeDir = "");
      virtual ~Alpr();

      void setDetectRegion(bool detectRegion);
      void setTopN(int topN);
      void setDefaultRegion(std::string region);

      // Recognize from an image on disk
      AlprResults recognize(std::string filepath);

      // Recognize from byte data representing an encoded image (e.g., BMP, PNG, JPG, GIF etc).
      AlprResults recognize(std::vector<char> imageBytes);

      // Recognize from raw pixel data.  
      AlprResults recognize(unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight, std::vector<AlprRegionOfInterest> regionsOfInterest);


      static std::string toJson(const AlprResults results);
      static AlprResults fromJson(std::string json);

      bool isLoaded();

      static std::string getVersion();

    private:
      AlprImpl* impl;
  };

}
#endif // OPENALPR_APLR_H
