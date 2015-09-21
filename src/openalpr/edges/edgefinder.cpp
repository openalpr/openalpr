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

#include "edgefinder.h"
#include "textlinecollection.h"

using namespace std;
using namespace cv;

namespace alpr
{

  EdgeFinder::EdgeFinder(PipelineData* pipeline_data) {

    this->pipeline_data = pipeline_data;

    // First re-crop the area from the original picture knowing the text position

  }


  EdgeFinder::~EdgeFinder() {
  }

  std::vector<cv::Point2f> EdgeFinder::findEdgeCorners() {

    TextLineCollection tlc(pipeline_data->textLines);

    vector<Point> corners;

    // If the character segment is especially small, just expand the existing box
    // If it's a nice, long segment, then guess the correct box based on character height/position
    if (tlc.longerSegment.length > tlc.charHeight * 3)
    {

      float charHeightToPlateWidthRatio = pipeline_data->config->plateWidthMM / pipeline_data->config->avgCharHeightMM;
      float idealPixelWidth = tlc.charHeight *  (charHeightToPlateWidthRatio * 1.03);	// Add 3% so we don't clip any characters

      float charHeightToPlateHeightRatio = pipeline_data->config->plateHeightMM / pipeline_data->config->avgCharHeightMM;
      float idealPixelHeight = tlc.charHeight *  charHeightToPlateHeightRatio;


      float verticalOffset = (idealPixelHeight * 1.5 / 2);
      float horizontalOffset = (idealPixelWidth * 1.25 / 2);
      LineSegment topLine = tlc.centerHorizontalLine.getParallelLine(verticalOffset);
      LineSegment bottomLine = tlc.centerHorizontalLine.getParallelLine(-1 * verticalOffset);

      LineSegment leftLine = tlc.centerVerticalLine.getParallelLine(-1 * horizontalOffset);
      LineSegment rightLine = tlc.centerVerticalLine.getParallelLine(horizontalOffset);

      Point topLeft = topLine.intersection(leftLine);
      Point topRight = topLine.intersection(rightLine);
      Point botRight = bottomLine.intersection(rightLine);
      Point botLeft = bottomLine.intersection(leftLine);

      corners.push_back(topLeft);
      corners.push_back(topRight);
      corners.push_back(botRight);
      corners.push_back(botLeft);
    }
    else
    {

      int expandX = (int) ((float) pipeline_data->crop_gray.cols) * 0.15f;
      int expandY = (int) ((float) pipeline_data->crop_gray.rows) * 0.15f;
      int w = pipeline_data->crop_gray.cols;
      int h = pipeline_data->crop_gray.rows;

      corners.push_back(Point(-1 * expandX, -1 * expandY));
      corners.push_back(Point(expandX + w, -1 * expandY));
      corners.push_back(Point(expandX + w, expandY + h));
      corners.push_back(Point(-1 * expandX, expandY + h));

  //    for (int i = 0; i < 4; i++)
  //    {
  //      std::cout << "CORNER: " << corners[i].x << " - " << corners[i].y << std::endl;
  //    }
    }

    // Re-crop an image (from the original image) using the new coordinates
    Transformation imgTransform(pipeline_data->grayImg, pipeline_data->crop_gray, pipeline_data->regionOfInterest);
    vector<Point2f> remappedCorners = imgTransform.transformSmallPointsToBigImage(corners);

    Size cropSize = imgTransform.getCropSize(remappedCorners, 
            Size(pipeline_data->config->templateWidthPx, pipeline_data->config->templateHeightPx));

    Mat transmtx = imgTransform.getTransformationMatrix(remappedCorners, cropSize);
    Mat newCrop = imgTransform.crop(cropSize, transmtx);

    // Re-map the textline coordinates to the new crop  
    vector<TextLine> newLines;
    for (unsigned int i = 0; i < pipeline_data->textLines.size(); i++)
    {        
      vector<Point2f> textArea = imgTransform.transformSmallPointsToBigImage(pipeline_data->textLines[i].textArea);
      vector<Point2f> linePolygon = imgTransform.transformSmallPointsToBigImage(pipeline_data->textLines[i].linePolygon);

      vector<Point2f> textAreaRemapped;
      vector<Point2f> linePolygonRemapped;

      textAreaRemapped = imgTransform.remapSmallPointstoCrop(textArea, transmtx);
      linePolygonRemapped = imgTransform.remapSmallPointstoCrop(linePolygon, transmtx);

      newLines.push_back(TextLine(textAreaRemapped, linePolygonRemapped, newCrop.size()));
    }

    // Find the PlateLines for this crop
    PlateLines plateLines(pipeline_data);
    plateLines.processImage(newCrop, newLines, 1.05);

    // Get the best corners
    PlateCorners cornerFinder(newCrop, &plateLines, pipeline_data, newLines);
    vector<Point> smallPlateCorners = cornerFinder.findPlateCorners();


    // Transform the best corner points back to the original image
    std::vector<Point2f> imgArea;
    imgArea.push_back(Point2f(0, 0));
    imgArea.push_back(Point2f(newCrop.cols, 0));
    imgArea.push_back(Point2f(newCrop.cols, newCrop.rows));
    imgArea.push_back(Point2f(0, newCrop.rows));
    Mat newCropTransmtx = imgTransform.getTransformationMatrix(imgArea, remappedCorners);

    vector<Point2f> cornersInOriginalImg = imgTransform.remapSmallPointstoCrop(smallPlateCorners, newCropTransmtx);

    return cornersInOriginalImg;

  }

}