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

#include <opencv2/core/core.hpp>

#include "licenseplatecandidate.h"
#include "edges/edgefinder.h"
#include "transformation.h"

using namespace std;
using namespace cv;

namespace alpr
{

  LicensePlateCandidate::LicensePlateCandidate(PipelineData* pipeline_data)
  {
    this->pipeline_data = pipeline_data;
    this->config = pipeline_data->config;

  }

  LicensePlateCandidate::~LicensePlateCandidate()
  {
    delete charSegmenter;
  }

  // Must delete this pointer in parent class
  void LicensePlateCandidate::recognize()
  {
    charSegmenter = NULL;

    pipeline_data->plate_area_confidence = 0;
    pipeline_data->isMultiline = config->multiline;


    Rect expandedRegion = this->pipeline_data->regionOfInterest;

    pipeline_data->crop_gray = Mat(this->pipeline_data->grayImg, expandedRegion);
    resize(pipeline_data->crop_gray, pipeline_data->crop_gray, Size(config->templateWidthPx, config->templateHeightPx));


    CharacterAnalysis textAnalysis(pipeline_data);

    if (textAnalysis.confidence > 10)
    {

      EdgeFinder edgeFinder(pipeline_data);

      pipeline_data->plate_corners = edgeFinder.findEdgeCorners();

      if (edgeFinder.confidence > 0)
      {

        timespec startTime;
        getTime(&startTime);


        Mat originalCrop = pipeline_data->crop_gray;

        Transformation imgTransform(this->pipeline_data->grayImg, pipeline_data->crop_gray, expandedRegion);

        Size cropSize = imgTransform.getCropSize(pipeline_data->plate_corners, 
                Size(pipeline_data->config->ocrImageWidthPx, pipeline_data->config->ocrImageHeightPx));
        Mat transmtx = imgTransform.getTransformationMatrix(pipeline_data->plate_corners, cropSize);
        pipeline_data->crop_gray = imgTransform.crop(cropSize, transmtx);


        if (this->config->debugGeneral)
          displayImage(config, "quadrilateral", pipeline_data->crop_gray);



        // Apply a perspective transformation to the TextLine objects
        // to match the newly deskewed license plate crop
        vector<TextLine> newLines;
        for (unsigned int i = 0; i < pipeline_data->textLines.size(); i++)
        {        
          vector<Point2f> textArea = imgTransform.transformSmallPointsToBigImage(pipeline_data->textLines[i].textArea);
          vector<Point2f> linePolygon = imgTransform.transformSmallPointsToBigImage(pipeline_data->textLines[i].linePolygon);

          vector<Point2f> textAreaRemapped;
          vector<Point2f> linePolygonRemapped;

          textAreaRemapped = imgTransform.remapSmallPointstoCrop(textArea, transmtx);
          linePolygonRemapped = imgTransform.remapSmallPointstoCrop(linePolygon, transmtx);

          newLines.push_back(TextLine(textAreaRemapped, linePolygonRemapped));
        }

        pipeline_data->textLines.clear();
        for (unsigned int i = 0; i < newLines.size(); i++)
          pipeline_data->textLines.push_back(newLines[i]);



        if (config->debugTiming)
        {
          timespec endTime;
          getTime(&endTime);
          cout << "deskew Time: " << diffclock(startTime, endTime) << "ms." << endl;
        }

        charSegmenter = new CharacterSegmenter(pipeline_data);


        pipeline_data->plate_area_confidence = 100;
      }

    }
  }

}
