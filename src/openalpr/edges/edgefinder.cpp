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
#include "support/timing.h"

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

    bool high_contrast = is_high_contrast(pipeline_data->crop_gray);
    
    vector<Point2f> returnPoints;
    
    if (high_contrast)
    {
      // Try a high-contrast pass first.  If it doesn't return anything, try a normal detection
      returnPoints = detection(true);
    }
    
    if (!high_contrast || returnPoints.size() == 0)
    {
      returnPoints = detection(false);
    }
    
    return returnPoints;
    
  }
  
  std::vector<cv::Point2f> EdgeFinder::detection(bool high_contrast) {
    TextLineCollection tlc(pipeline_data->textLines);

    vector<Point> corners;

    
    // If the character segment is especially small, just expand the existing box
    // If it's a nice, long segment, then guess the correct box based on character height/position
    if (high_contrast)
    {
      int expandX = (int) ((float) pipeline_data->crop_gray.cols) * 0.5f;
      int expandY = (int) ((float) pipeline_data->crop_gray.rows) * 0.5f;
      int w = pipeline_data->crop_gray.cols;
      int h = pipeline_data->crop_gray.rows;

      corners.push_back(Point(-1 * expandX, -1 * expandY));
      corners.push_back(Point(expandX + w, -1 * expandY));
      corners.push_back(Point(expandX + w, expandY + h));
      corners.push_back(Point(-1 * expandX, expandY + h));
    }
    else if (tlc.longerSegment.length > tlc.charHeight * 3)
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

    vector<Point> smallPlateCorners;
    
    if (high_contrast)
    {
      smallPlateCorners = highContrastDetection(newCrop, newLines);
    }
    else
    {
      smallPlateCorners = normalDetection(newCrop, newLines);
    }

    // Transform the best corner points back to the original image
    std::vector<Point2f> imgArea;
    imgArea.push_back(Point2f(0, 0));
    imgArea.push_back(Point2f(newCrop.cols, 0));
    imgArea.push_back(Point2f(newCrop.cols, newCrop.rows));
    imgArea.push_back(Point2f(0, newCrop.rows));
    Mat newCropTransmtx = imgTransform.getTransformationMatrix(imgArea, remappedCorners);

    vector<Point2f> cornersInOriginalImg;
    
    if (smallPlateCorners.size() > 0)
      cornersInOriginalImg = imgTransform.remapSmallPointstoCrop(smallPlateCorners, newCropTransmtx);

    return cornersInOriginalImg;
  }


  vector<cv::Point> EdgeFinder::normalDetection(Mat newCrop, vector<TextLine> newLines)
  {
    // Find the PlateLines for this crop
    PlateLines plateLines(pipeline_data);
    plateLines.processImage(newCrop, newLines, 1.05);

    // Get the best corners
    PlateCorners cornerFinder(newCrop, &plateLines, pipeline_data, newLines);
    return cornerFinder.findPlateCorners();
  }
  
  vector<cv::Point> EdgeFinder::highContrastDetection(Mat newCrop, vector<TextLine> newLines) {
    
    
    vector<Point> smallPlateCorners;
    
    if (pipeline_data->config->debugGeneral)
      std::cout << "Performing high-contrast edge detection" << std::endl;
    
    // Do a morphology operation.  Find the biggest white rectangle that fit most of the char area.

    int morph_size = 3;
    Mat closureElement = getStructuringElement( 2, // 0 Rect, 1 cross, 2 ellipse
                         Size( 2 * morph_size + 1, 2* morph_size + 1 ),
                         Point( morph_size, morph_size ) );

    morphologyEx(newCrop, newCrop, MORPH_CLOSE, closureElement);
    morphologyEx(newCrop, newCrop, MORPH_OPEN, closureElement);

    Mat thresholded_crop;
    threshold(newCrop, thresholded_crop, 80, 255, cv::THRESH_OTSU);

    vector<vector<Point> > contours;
    findContours(thresholded_crop, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

    float MIN_AREA = 0.05 * newCrop.cols * newCrop.rows;
    for (unsigned int i = 0; i < contours.size(); i++)
    {
      if (contourArea(contours[i]) < MIN_AREA)
        continue;

      vector<Point> smoothedPoints;
      approxPolyDP(contours[i], smoothedPoints, 1, true);

      RotatedRect rrect = minAreaRect(smoothedPoints);

      Point2f rect_points[4];
      rrect.points(rect_points);

      vector<Point> sorted_polygon_points = sortPolygonPoints(rect_points, newCrop.size());

      float polygon_width = (distanceBetweenPoints(sorted_polygon_points[0], sorted_polygon_points[1]) + 
                          distanceBetweenPoints(sorted_polygon_points[3], sorted_polygon_points[2])) / 2;
      float polygon_height = (distanceBetweenPoints(sorted_polygon_points[2], sorted_polygon_points[1]) + 
                          distanceBetweenPoints(sorted_polygon_points[3], sorted_polygon_points[0])) / 2;
      // If it touches the edges, disqualify it

      // Create an inner rect, and test to make sure all the points are within it
      int x_offset = newCrop.cols * 0.1;
      int y_offset = newCrop.rows * 0.1;
      Rect insideRect(Point(x_offset, y_offset), Point(newCrop.cols - x_offset, newCrop.rows - y_offset));

      bool isoutside = false;
      for (unsigned int ptidx = 0; ptidx < sorted_polygon_points.size(); ptidx++)
      {
        if (!insideRect.contains(sorted_polygon_points[ptidx]))
          isoutside = true;
      }
      if (isoutside)
        continue;

      // If the center is not centered, disqualify it
      float MAX_CLOSENESS_TO_EDGE_PERCENT = 0.2;
      if (rrect.center.x < (newCrop.cols * MAX_CLOSENESS_TO_EDGE_PERCENT) || 
              rrect.center.x > (newCrop.cols - (newCrop.cols * MAX_CLOSENESS_TO_EDGE_PERCENT)) ||
              rrect.center.y < (newCrop.rows * MAX_CLOSENESS_TO_EDGE_PERCENT) || 
              rrect.center.y > (newCrop.rows - (newCrop.rows * MAX_CLOSENESS_TO_EDGE_PERCENT)))
      {
        continue;
      }

      // Make sure the aspect ratio is somewhat close to a license plate.
      float aspect_ratio = polygon_width / polygon_height;
      float ideal_aspect_ratio = pipeline_data->config->plateWidthMM / pipeline_data->config->plateHeightMM;

      float ratio = ideal_aspect_ratio / aspect_ratio;

      if (ratio > 2 || ratio < 0.5)
        continue;

      // Make sure that the text line(s) are contained within it

      Rect rect_cover = rrect.boundingRect();
      for (unsigned int linenum = 0; linenum < newLines.size(); linenum++)
      {
        for (unsigned int r = 0; r < newLines[linenum].textArea.size(); r++)
        {
          if (!rect_cover.contains(newLines[linenum].textArea[r]))
          {
            isoutside = true;
            break;
          }
        }
      }
      if (isoutside)
        continue;


      for (int ridx = 0; ridx < 4; ridx++)
        smallPlateCorners.push_back(sorted_polygon_points[ridx]);


    }


    
    return smallPlateCorners;
  }

  
  
  
  bool EdgeFinder::is_high_contrast(const cv::Mat crop) {

    int stride = 2;
    
    int rows = crop.rows;
    int cols = crop.cols / stride;
    
    timespec startTime;
    getTimeMonotonic(&startTime);
    // Calculate pixel intensity
    float avg_intensity = 0;
    for (unsigned int y = 0; y < rows; y++)
    {
      for (unsigned int x = 0; x < crop.cols; x += stride)
      {
        avg_intensity = avg_intensity + crop.at<uchar>(y,x);
      }
    }
    avg_intensity = avg_intensity / (float) (rows * cols * 255);
    
    // Calculate RMS contrast
    float contrast = 0;
    for (unsigned int y = 0; y < rows; y++)
    {
      for (unsigned int x = 0; x < crop.cols; x += stride)
      {
        contrast += pow( ((crop.at<unsigned char>(y,x) / 255.0) - avg_intensity), 2.0);
      }
    }
    contrast /= ((float) rows) * ((float)cols);
    
    contrast = pow(contrast, 0.5f);
    
    if (pipeline_data->config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
      cout << "High Contrast Detection Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }
    
    return contrast > pipeline_data->config->contrastDetectionThreshold;
  }

}