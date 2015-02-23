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

#include "detectormorph.h"

using namespace cv;
using namespace std;

namespace alpr
{


	DetectorMorph::DetectorMorph(Config* config) : Detector(config) {

		this->loaded = true;
	}


	DetectorMorph::~DetectorMorph() {
	}

	vector<PlateRegion> DetectorMorph::detect(Mat frame, std::vector<cv::Rect> regionsOfInterest)
	{

		Mat frame_gray;
		cvtColor(frame, frame_gray, CV_BGR2GRAY);

		vector<PlateRegion> detectedRegions;
		for (int i = 0; i < regionsOfInterest.size(); i++)
		{
			Mat img_sobel, edges, img_open, img_result;
			Mat element = getStructuringElement(MORPH_RECT, Size(30, 4));
			morphologyEx(frame_gray, img_open, CV_MOP_OPEN, element, cv::Point(-1, -1));

			img_result = frame_gray - img_open;

			if (config->debugShowImages)
			{
				imshow("Opening", img_result);
			}
			
			//threshold image using otsu thresholding
			Mat img_threshold, img_open2;
			threshold(img_result, img_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);

			if (config->debugShowImages)
			{
				imshow("Threshold Detector", img_threshold);
			}

			Mat circularElement = getStructuringElement(cv::MORPH_ELLIPSE, Size(5, 5));
			morphologyEx(img_threshold, img_open2, CV_MOP_OPEN, circularElement, cv::Point(-1, -1));
			Mat rectElement = getStructuringElement(cv::MORPH_RECT, Size(20, 4));
			morphologyEx(img_open2, img_threshold, CV_MOP_CLOSE, rectElement, cv::Point(-1, -1));

			if (config->debugShowImages)
			{
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
			while (itc != contours.end()) 
			{
				//Create bounding rect of object
				RotatedRect mr = minAreaRect(Mat(*itc));
				if (!CheckSizes(mr))
					itc = contours.erase(itc);
				else
				{
					++itc;
					if (itc == contours.end()) continue;	
					rects.push_back(mr);
				}
			}

			//Now prunning based on checking all candidate plates for a min/max number of blobs
			Mat img_crop, img_crop_b;
			vector< vector< Point> > plateBlobs;
			for (int i = 0; i < rects.size(); i++)
			{
				RotatedRect PlateRect = rects[i];
				Size rect_size = PlateRect.size;
				
				//Crop area around candidate plate
				getRectSubPix(frame_gray, rect_size, PlateRect.center, img_crop);
				//Thresholding 
				threshold(img_crop, img_crop_b, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);

				findContours(img_crop_b,
					plateBlobs, // a vector of contours
					CV_RETR_EXTERNAL, // retrieve the external contours
					CV_CHAIN_APPROX_NONE); // all pixels of each contours

				int numBlobs = plateBlobs.size();

				//If too much or too little might not be a true plate
				if (numBlobs < 4 || numBlobs > 20) continue;

				PlateRegion PlateReg;

				PlateReg.rect = PlateRect.boundingRect();

				detectedRegions.push_back(PlateReg);

			}
			
		}

		return detectedRegions;
	}

	bool DetectorMorph::CheckSizes(RotatedRect& mr)
	{

		float error = 1.2; 
		
		float aspect = config->plateWidthMM / config->plateHeightMM;
		//Set a min and max area. All other patchs are discarded
		int min = 10 * aspect * 10; // minimum area
		int max = 100 * aspect * 100; // maximum area
		//Get only patchs that match to a respect ratio.
		float rmin = 3.0;
		float rmax = 6.0;

		int area = mr.size.height * mr.size.width;
		float r = (float)mr.size.width / (float)mr.size.height;
		if (r<1)
			r = (float)mr.size.height / (float)mr.size.width;

		if ((area < min || area > max) || (r < rmin || r > rmax) ||
			(mr.size.width < 20 || mr.size.width > 300) )
			return false;
		else
			return true;

	}
	

}
