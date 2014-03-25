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

LicensePlateCandidate::LicensePlateCandidate(Mat frame, Rect regionOfInterest, Config* config)
{
  this->config = config;

  this->frame = frame;
  this->plateRegion = regionOfInterest;
}

LicensePlateCandidate::~LicensePlateCandidate()
{
  delete charSegmenter;
}

// Must delete this pointer in parent class
void LicensePlateCandidate::recognize()
{
  charSegmenter = NULL;

  this->confidence = 0;

  int expandX = round(this->plateRegion.width * 0.15);
  int expandY = round(this->plateRegion.height * 0.10);
  // expand box by 15% in all directions
  Rect expandedRegion = expandRect( this->plateRegion, expandX, expandY, frame.cols, frame.rows) ;

  Mat plate_bgr = Mat(frame, expandedRegion);
  resize(plate_bgr, plate_bgr, Size(config->templateWidthPx, config->templateHeightPx));

  Mat plate_bgr_cleaned = Mat(plate_bgr.size(), plate_bgr.type());
  this->cleanupColors(plate_bgr, plate_bgr_cleaned);

  CharacterRegion charRegion(plate_bgr, config);

  if (charRegion.confidence > 10)
  {
    PlateLines plateLines(config);
    //Mat boogedy = charRegion.getPlateMask();

    plateLines.processImage(charRegion.getPlateMask(), 1.15);
    plateLines.processImage(plate_bgr_cleaned, 0.9);

    PlateCorners cornerFinder(plate_bgr, &plateLines, &charRegion, config);
    vector<Point> smallPlateCorners = cornerFinder.findPlateCorners();

    if (cornerFinder.confidence > 0)
    {
      this->plateCorners = transformPointsToOriginalImage(frame, plate_bgr, expandedRegion, smallPlateCorners);

      this->deskewed = deSkewPlate(frame, this->plateCorners);

      charSegmenter = new CharacterSegmenter(deskewed, charRegion.thresholdsInverted(), config);

      //this->recognizedText = ocr->recognizedText;
      //strcpy(this->recognizedText, ocr.recognizedText);

      this->confidence = 100;
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

  Mat deskewed(height, width, frame.type());

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

void LicensePlateCandidate::cleanupColors(Mat inputImage, Mat outputImage)
{
  if (this->config->debugGeneral)
    cout << "LicensePlate::cleanupColors" << endl;

  //Mat normalized(inputImage.size(), inputImage.type());

  Mat intermediate(inputImage.size(), inputImage.type());

  normalize(inputImage, intermediate, 0, 255, CV_MINMAX );

  // Equalize intensity:
  if(intermediate.channels() >= 3)
  {
    Mat ycrcb;

    cvtColor(intermediate,ycrcb,CV_BGR2YCrCb);

    vector<Mat> channels;
    split(ycrcb,channels);

    equalizeHist(channels[0], channels[0]);

    merge(channels,ycrcb);

    cvtColor(ycrcb,intermediate,CV_YCrCb2BGR);

    //ycrcb.release();
  }

  bilateralFilter(intermediate, outputImage, 3, 25, 35);

  if (this->config->debugGeneral)
  {
    displayImage(config, "After cleanup", outputImage);
  }
}
