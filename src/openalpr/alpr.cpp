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

#include "alpr.h"
#include "alpr_impl.h"

// ALPR code

Alpr::Alpr(const std::string country, const std::string configFile, const std::string runtimeDir)
{
  impl = new AlprImpl(country, configFile, runtimeDir);
}

Alpr::~Alpr()
{
  delete impl;
}

std::vector<AlprResult> Alpr::recognize(std::string filepath)
{
  cv::Mat img = cv::imread(filepath, CV_LOAD_IMAGE_COLOR);
  return impl->recognize(img);
}

std::vector<AlprResult> Alpr::recognize(std::vector<unsigned char> imageBuffer)
{
  // Not sure if this actually works
  cv::Mat img = cv::imdecode(cv::Mat(imageBuffer), 1);

  return impl->recognize(img);
}

std::string Alpr::toJson(const std::vector< AlprResult > results, double processing_time_ms)
{
  return impl->toJson(results, processing_time_ms);
}

void Alpr::setDetectRegion(bool detectRegion)
{
  impl->setDetectRegion(detectRegion);
}

void Alpr::setTopN(int topN)
{
  impl->setTopN(topN);
}

void Alpr::setDefaultRegion(std::string region)
{
  impl->setDefaultRegion(region);
}

bool Alpr::isLoaded()
{
  return impl->isLoaded();
}

std::string Alpr::getVersion()
{
  return AlprImpl::getVersion();
}

// Results code

AlprResult::AlprResult()
{
}

AlprResult::~AlprResult()
{
}
