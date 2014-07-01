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

#include "licenseplatecandidate.h"

using namespace std;
using namespace cv;

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

  int expandX = round(this->pipeline_data->regionOfInterest.width * 0.20);
  int expandY = round(this->pipeline_data->regionOfInterest.height * 0.15);
  // expand box by 15% in all directions
  Rect expandedRegion = expandRect( this->pipeline_data->regionOfInterest, expandX, expandY, this->pipeline_data->grayImg.cols, this->pipeline_data->grayImg.rows) ;

  pipeline_data->crop_gray = Mat(this->pipeline_data->grayImg, expandedRegion);
  resize(pipeline_data->crop_gray, pipeline_data->crop_gray, Size(config->templateWidthPx, config->templateHeightPx));
  
  
  CharacterRegion charRegion(pipeline_data);

  if (charRegion.confidence > 10)
  {
    PlateLines plateLines(config);

    plateLines.processImage(pipeline_data->plate_mask, &charRegion, 1.10);
    plateLines.processImage(pipeline_data->crop_gray, &charRegion, 0.9);

    PlateCorners cornerFinder(pipeline_data->crop_gray, &plateLines, &charRegion, config);
    vector<Point> smallPlateCorners = cornerFinder.findPlateCorners();

    if (cornerFinder.confidence > 0)
    {
      pipeline_data->plate_corners = transformPointsToOriginalImage(this->pipeline_data->grayImg, pipeline_data->crop_gray, expandedRegion, smallPlateCorners);

      pipeline_data->crop_gray = deSkewPlate(this->pipeline_data->grayImg, pipeline_data->plate_corners);

      charSegmenter = new CharacterSegmenter(pipeline_data);

      //this->recognizedText = ocr->recognizedText;
      //strcpy(this->recognizedText, ocr.recognizedText);

      pipeline_data->plate_area_confidence = 100;
    }
    charRegion.confidence = 0;
  }
}

// Re-maps the coordinates from the smallImage to the coordinate space of the bigImage.
vector<Point2f> LicensePlateCandidate::transformPointsToOriginalImage(Mat bigImage, Mat smallImage, Rect region, vector<Point> corners)
{
  vector<Point2f> cornerPoints;
  for (int i = 0; i < corners.size(); i++)
  {
    float bigX = (corners[i].x * ((float) region.width / smallImage.cols));
    float bigY = (corners[i].y * ((float) region.height / smallImage.rows));

    bigX = bigX + region.x;
    bigY = bigY + region.y;

    cornerPoints.push_back(Point2f(bigX, bigY));
  }

  return cornerPoints;
}

Mat LicensePlateCandidate::deSkewPlate(Mat inputImage, vector<Point2f> corners)
{
  // Figure out the appoximate width/height of the license plate region, so we can maintain the aspect ratio.
  LineSegment leftEdge(round(corners[3].x), round(corners[3].y), round(corners[0].x), round(corners[0].y));
  LineSegment rightEdge(round(corners[2].x), round(corners[2].y), round(corners[1].x), round(corners[1].y));
  LineSegment topEdge(round(corners[0].x), round(corners[0].y), round(corners[1].x), round(corners[1].y));
  LineSegment bottomEdge(round(corners[3].x), round(corners[3].y), round(corners[2].x), round(corners[2].y));

  float w = distanceBetweenPoints(leftEdge.midpoint(), rightEdge.midpoint());
  float h = distanceBetweenPoints(bottomEdge.midpoint(), topEdge.midpoint());
  float aspect = w/h;

  int width = config->ocrImageWidthPx;
  int height = round(((float) width) / aspect);
  if (height > config->ocrImageHeightPx)
  {
    height = config->ocrImageHeightPx;
    width = round(((float) height) * aspect);
  }

  Mat deskewed(height, width, this->pipeline_data->grayImg.type());

  // Corners of the destination image
  vector<Point2f> quad_pts;
  quad_pts.push_back(Point2f(0, 0));
  quad_pts.push_back(Point2f(deskewed.cols, 0));
  quad_pts.push_back(Point2f(deskewed.cols, deskewed.rows));
  quad_pts.push_back(Point2f(0, deskewed.rows));

  // Get transformation matrix
  Mat transmtx = getPerspectiveTransform(corners, quad_pts);

  // Apply perspective transformation
  warpPerspective(inputImage, deskewed, transmtx, deskewed.size());

  if (this->config->debugGeneral)
    displayImage(config, "quadrilateral", deskewed);

  return deskewed;
}

