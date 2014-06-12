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

#ifndef OPENALPR_ALPR_H
#define OPENALPR_ALPR_H

#include <iostream>
#include <vector>


struct AlprPlate
{
  std::string characters;
  float overall_confidence;

  bool matches_template;
  //int char_confidence[];
};

struct AlprCoordinate
{
  int x;
  int y;
};

class AlprResult
{
  public:
    AlprResult();
    virtual ~AlprResult();

    int requested_topn;
    int result_count;

    AlprPlate bestPlate;
    std::vector<AlprPlate> topNPlates;

    float processing_time_ms;
    AlprCoordinate plate_points[4];

    int regionConfidence;
    std::string region;
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

    std::vector<AlprResult> recognize(std::string filepath);
    std::vector<AlprResult> recognize(std::vector<unsigned char> imageBuffer);

    std::string toJson(const std::vector<AlprResult> results, double processing_time_ms = -1);

    bool isLoaded();
    
    static std::string getVersion();

  private:
    AlprImpl* impl;
};

#endif // OPENALPR_APLR_H
