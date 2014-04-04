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

#include "regiondetector.h"

RegionDetector::RegionDetector(Config* config)
{
  this->config = config;
  // Don't scale.  Can change this in the future (i.e., maximum resolution preference, or some such).
  this->scale_factor = 1.0f;

  // Load either the regular or OpenCL version of the cascade classifier
  if (config->opencl_enabled)
  {
    this->plate_cascade = new ocl::OclCascadeClassifier();
  }
  else
  {
    this->plate_cascade = new CascadeClassifier();
  }

  if( this->plate_cascade->load( config->getCascadeRuntimeDir() + config->country + ".xml" ) )
  {
    this->loaded = true;
  }
  else
  {
    this->loaded = false;
    printf("--(!)Error loading classifier\n");
  }

}

RegionDetector::~RegionDetector()
{
  delete this->plate_cascade;
}

bool RegionDetector::isLoaded()
{
  return this->loaded;
}

vector<PlateRegion> RegionDetector::detect(Mat frame)
{

  Mat frame_gray;
  cvtColor( frame, frame_gray, CV_BGR2GRAY );

  vector<PlateRegion> regionsOfInterest = doCascade(frame_gray);

  return regionsOfInterest;
}

/** @function detectAndDisplay */
vector<PlateRegion> RegionDetector::doCascade(Mat frame)
{
  //float scale_factor = 1;
  int w = frame.size().width;
  int h = frame.size().height;

  vector<Rect> plates;

  equalizeHist( frame, frame );
  resize(frame, frame, Size(w * this->scale_factor, h * this->scale_factor));

  //-- Detect plates
  timespec startTime;
  getTime(&startTime);

  Size minSize(config->minPlateSizeWidthPx * this->scale_factor, config->minPlateSizeHeightPx * this->scale_factor);
  Size maxSize(w * config->maxPlateWidthPercent * this->scale_factor, h * config->maxPlateHeightPercent * this->scale_factor);

  if (config->opencl_enabled)
  {
    ocl::oclMat openclFrame(frame);
    ((ocl::OclCascadeClassifier*) plate_cascade)->detectMultiScale(openclFrame, plates, config->detection_iteration_increase, 3, 0, minSize, maxSize);
  }
  else
  {

    plate_cascade->detectMultiScale( frame, plates, config->detection_iteration_increase, 3,
                                     0,
                                     //0|CV_HAAR_SCALE_IMAGE,
                                     minSize, maxSize );
  }

  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "LBP Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }

  for( int i = 0; i < plates.size(); i++ )
  {
    plates[i].x = plates[i].x / scale_factor;
    plates[i].y = plates[i].y / scale_factor;
    plates[i].width = plates[i].width / scale_factor;
    plates[i].height = plates[i].height / scale_factor;
  }

  vector<PlateRegion> orderedRegions = aggregateRegions(plates);
  
  return orderedRegions;

}

vector<PlateRegion> RegionDetector::aggregateRegions(vector<Rect> regions)
{
  // Combines overlapping regions into a parent->child order.
  // The largest regions will be parents, and they will have children if they are within them.
  // This way, when processing regions later, we can process the parents first, and only delve into the children
  // If there was no plate match.  Otherwise, we would process everything and that would be wasteful.
  
  vector<PlateRegion> orderedRegions;
  
  // For now, just return a full list with no children, so I can get the plumbing sorted.
  for (int i = 0; i < regions.size(); i++)
  {
    PlateRegion newRegion;
    newRegion.rect = regions[i];
    orderedRegions.push_back(newRegion);
  }
  
  return orderedRegions;
}
