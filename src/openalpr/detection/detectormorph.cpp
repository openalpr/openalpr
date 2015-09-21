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

#include "detectormorph.h"

using namespace cv;
using namespace std;

namespace alpr {

  DetectorMorph::DetectorMorph(Config* config) : Detector(config) {

    this->loaded = true;
  }

  DetectorMorph::~DetectorMorph() {
  }

  bool  DetectorMorph::ValidateCharAspect(Rect& r0, float idealAspect) {
		if ((r0.width < 5 || r0.width > 20)
			|| (r0.height < 15 || r0.height > 40)) return false;

		float aspectChar = ((float)r0.width / (float)r0.height);

		float deltaChar = fabs(idealAspect - aspectChar);

		if (deltaChar < 0.25)
			return true;
		else
			return false;

	}
	
  vector<PlateRegion> DetectorMorph::detect(Mat frame, std::vector<cv::Rect> regionsOfInterest) {

    Mat frame_gray,frame_gray_cp;

    if (frame.channels() > 2)
    {
      cvtColor( frame, frame_gray, CV_BGR2GRAY );
    }
    else
    {
      frame.copyTo(frame_gray);
    }

    frame_gray.copyTo(frame_gray_cp);
    blur(frame_gray, frame_gray, Size(5, 5));

    vector<PlateRegion> detectedRegions;
    for (int i = 0; i < regionsOfInterest.size(); i++) {
      Mat img_open, img_result;
      Mat element = getStructuringElement(MORPH_RECT, Size(30, 4));
      morphologyEx(frame_gray, img_open, CV_MOP_OPEN, element, cv::Point(-1, -1));

      img_result = frame_gray - img_open;

      if (config->debugDetector && config->debugShowImages) {
        imshow("Opening", img_result);
      }

      //threshold image using otsu thresholding
      Mat img_threshold, img_open2;
      threshold(img_result, img_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);

      if (config->debugDetector && config->debugShowImages) {
        imshow("Threshold Detector", img_threshold);
      }

      Mat diamond(5, 5, CV_8U, cv::Scalar(1));

	diamond.at<uchar>(0, 0) = 0;
	diamond.at<uchar>(0, 1) = 0;
	diamond.at<uchar>(1, 0) = 0;
	diamond.at<uchar>(4, 4) = 0;
	diamond.at<uchar>(3, 4) = 0;
	diamond.at<uchar>(4, 3) = 0;
	diamond.at<uchar>(4, 0) = 0;
	diamond.at<uchar>(4, 1) = 0;
	diamond.at<uchar>(3, 0) = 0;
	diamond.at<uchar>(0, 4) = 0;
	diamond.at<uchar>(0, 3) = 0;
	diamond.at<uchar>(1, 4) = 0;
			
      morphologyEx(img_threshold, img_open2, CV_MOP_OPEN, diamond, cv::Point(-1, -1));
      Mat rectElement = getStructuringElement(cv::MORPH_RECT, Size(13, 4));
      morphologyEx(img_open2, img_threshold, CV_MOP_CLOSE, rectElement, cv::Point(-1, -1));

      if (config->debugDetector && config->debugShowImages) {
        imshow("Close", img_threshold);
        waitKey(0);
      }

      //Find contours of possibles plates
      vector< vector< Point> > contours;
      findContours(img_threshold,
              contours, // a vector of contours
              CV_RETR_EXTERNAL, // retrieve the external contours
              CV_CHAIN_APPROX_NONE); // all pixels of each contours

      //Start to iterate to each contour founded
      vector<vector<Point> >::iterator itc = contours.begin();
      vector<RotatedRect> rects;

      //Remove patch that are no inside limits of aspect ratio and area.    
      while (itc != contours.end()) {
        //Create bounding rect of object
        RotatedRect mr = minAreaRect(Mat(*itc));
        
        if (mr.angle < -45.) {
					mr.angle += 90.0;
					swap(mr.size.width, mr.size.height);
				}  
        
        if (!CheckSizes(mr))
          itc = contours.erase(itc);
        else {
          ++itc;
					rects.push_back(mr);
        }
      }

     //Now prunning based on checking all candidate plates for a min/max number of blobsc
Mat img_crop, img_crop_b, img_crop_th, img_crop_th_inv;
vector< vector< Point> > plateBlobs;
vector< vector< Point> > plateBlobsInv;
double thresholds[] = { 10, 40, 80, 120, 160, 200, 240 };
const int num_thresholds = 7;
int numValidChars = 0;
Mat rotated;
for (int i = 0; i < rects.size(); i++) {
	numValidChars = 0;
	RotatedRect PlateRect = rects[i];
	Size rect_size = PlateRect.size;

	// get the rotation matrix
	Mat M = getRotationMatrix2D(PlateRect.center, PlateRect.angle, 1.0);
	// perform the affine transformation
	warpAffine(frame_gray_cp, rotated, M, frame_gray_cp.size(), INTER_CUBIC);
	//Crop area around candidate plate
	getRectSubPix(rotated, rect_size, PlateRect.center, img_crop);

	 if (config->debugDetector && config->debugShowImages) {
		imshow("Tilt Correction", img_crop);
		waitKey(0);
	}

	for (int z = 0; z < num_thresholds; z++) {

		cv::threshold(img_crop, img_crop_th, thresholds[z], 255, cv::THRESH_BINARY);
		cv::threshold(img_crop, img_crop_th_inv, thresholds[z], 255, cv::THRESH_BINARY_INV);

		findContours(img_crop_th,
			plateBlobs, // a vector of contours
			CV_RETR_LIST, // retrieve the contour list
			CV_CHAIN_APPROX_NONE); // all pixels of each contours

		findContours(img_crop_th_inv,
			plateBlobsInv, // a vector of contours
			CV_RETR_LIST, // retrieve the contour list
			CV_CHAIN_APPROX_NONE); // all pixels of each contours

		int numBlobs = plateBlobs.size();
		int numBlobsInv = plateBlobsInv.size();
	
		float idealAspect = config->avgCharWidthMM / config->avgCharHeightMM;
		for (int j = 0; j < numBlobs; j++) {
			cv::Rect r0 = cv::boundingRect(cv::Mat(plateBlobs[j]));
			
			if (ValidateCharAspect(r0, idealAspect))
				numValidChars++;
		}

		for (int j = 0; j < numBlobsInv; j++) {
			cv::Rect r0 = cv::boundingRect(cv::Mat(plateBlobsInv[j]));
			if (ValidateCharAspect(r0, idealAspect))
				numValidChars++;
		}

	}
	//If too much or too lcittle might not be a true plate
	//if (numBlobs < 3 || numBlobs > 50) continue;
	if (numValidChars < 4  || numValidChars > 50) continue;

        PlateRegion PlateReg;

        // Ensure that the rectangle isn't < 0 or > maxWidth/Height
        Rect bounding_rect = PlateRect.boundingRect();
        PlateReg.rect = expandRect(bounding_rect, 0, 0, frame.cols, frame.rows);
        
        
        detectedRegions.push_back(PlateReg);

      }

    }
    
    return detectedRegions;
  }

  bool DetectorMorph::CheckSizes(RotatedRect& mr) {

    float error = 1.2;

    float aspect = config->plateWidthMM / config->plateHeightMM;
    //Set a min and max area. All other patchs are discarded
    int min = 10 * aspect * 10; // minimum area
    int max = 100 * aspect * 100; // maximum area
    //Get only patchs that match to a respect ratio.
    float rmin = 3.0;
    float rmax = 7.0;

    int area = mr.size.height * mr.size.width;
    float r = (float) mr.size.width / (float) mr.size.height;
    if (r < 1)
      r = (float) mr.size.height / (float) mr.size.width;

    if ( (area < min || area > max) || (r < rmin || r > rmax) ||
            (mr.size.width < 70 || mr.size.width > 300) || (mr.size.height < 10 || mr.size.height > 80) )
      return false;
    else
      return true;

  }


}
