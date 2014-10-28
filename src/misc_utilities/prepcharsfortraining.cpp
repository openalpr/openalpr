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

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include "support/filesystem.h"

using namespace std;
using namespace cv;
using namespace alpr;

// Takes a directory full of single char images, and plops them on a big tif files
// Also creates a box file so Tesseract can recognize it
int main( int argc, const char** argv )
{
  string inDir;

  //Check if user specify image to process
  if(argc == 2)
  {
    inDir = argv[1];
  }
  else
  {
    printf("Use:\n\t%s input dir \n",argv[0]);
    return 0;
  }

  if (DirectoryExists(inDir.c_str()) == false)
  {
    printf("Output dir does not exist\n");
    return 0;
  }

  cout << "Usage: " << endl;
  cout << "\tinputdir	-- input dir for benchmark data" << endl;

  if (DirectoryExists(inDir.c_str()))
  {
    const int CHAR_PADDING_HORIZONTAL = 0;
    const int CHAR_PADDING_VERTICAL = 0;
    
    const int X_OFFSET = 5;
    const int Y_OFFSET = 5;

    const int PAGE_MARGIN_X = 70;
    const int PAGE_MARGIN_Y = 70;
    const int HORIZONTAL_RESOLUTION = 3500;
    const int MAX_VERTICAL_RESOLUTION = 6000; // Maximum vertical size before chopping into additional pages.

    const int TILE_WIDTH = 25;
    const int TILE_HEIGHT = 60;
    const int CHAR_HORIZ_OFFSET = 40;
    const int CHAR_VERT_OFFSET = 48;

    const int FIXED_CHAR_HEIGHT = 40; // RESIZE all characters to this height
    
    vector<string> files = getFilesInDir(inDir.c_str());

    sort( files.begin(), files.end(), stringCompare );
    
    for (int i = 0; i< files.size(); i++)
    {
      if (hasEnding(files[i], ".png") || hasEnding(files[i], ".jpg"))
      {
	
      }
      else
      {
	std::cerr << "Non-image file detected in this directory.  This must be removed first" << std::endl;
	return 1;
      }
    }
    
    
    int tiles_per_row = ((float) (HORIZONTAL_RESOLUTION - (PAGE_MARGIN_X * 2))) / ((float) TILE_WIDTH);
    int lines = files.size() / (tiles_per_row);
    int vertical_resolution = (lines * TILE_HEIGHT) + (PAGE_MARGIN_Y * 3) ;
    cout << tiles_per_row <<   " : " << vertical_resolution << endl;

    Mat bigTif = Mat::zeros(Size(HORIZONTAL_RESOLUTION, vertical_resolution), CV_8U);
    bitwise_not(bigTif, bigTif);

    stringstream boxFileOut;

    for (int i = 0; i< files.size(); i++)
    {
      int col = i % tiles_per_row;
      int line = i / tiles_per_row;

      int xPos = (col * TILE_WIDTH) + PAGE_MARGIN_X;
      int yPos = (line * TILE_HEIGHT) + PAGE_MARGIN_Y;

      if (hasEnding(files[i], ".png") || hasEnding(files[i], ".jpg"))
      {
        string fullpath = inDir + "/" + files[i];

        cout << "Processing file: " << (i + 1) << " of " << files.size() << endl;

        char charcode = files[i][0];

        Mat characterImg = imread(fullpath);

	
        Mat charImgCopy = Mat::zeros(Size(150, 150), characterImg.type());
        bitwise_not(charImgCopy, charImgCopy);

        characterImg.copyTo(charImgCopy(Rect(X_OFFSET, Y_OFFSET, characterImg.cols, characterImg.rows)));
        cvtColor(charImgCopy, charImgCopy, CV_BGR2GRAY);
        bitwise_not(charImgCopy, charImgCopy);

        vector<vector<Point> > contours;

        //imshow("copy", charImgCopy);
        findContours(charImgCopy, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	float minHeightPercent = 0.35;
	int minHeight = (int) (((float) characterImg.rows) * minHeightPercent);

	vector<Rect> tallEnoughRects;
        for (int c = 0; c < contours.size(); c++)
        {
          Rect tmpRect = boundingRect(contours[c]);
          if (tmpRect.height > minHeight)
            tallEnoughRects.push_back( tmpRect );
        }

	int xMin = 9999999, xMax = 0, yMin = 9999999, yMax = 0;
        // Combine all the "tall enough" rectangles into one super rectangle
        for (int r = 0; r < tallEnoughRects.size(); r++)
	{
	  if (tallEnoughRects[r].x < xMin)
	    xMin = tallEnoughRects[r].x;
	  if (tallEnoughRects[r].y < yMin)
	    yMin = tallEnoughRects[r].y;
	  if (tallEnoughRects[r].x + tallEnoughRects[r].width > xMax)
	    xMax = tallEnoughRects[r].x + tallEnoughRects[r].width;
	  if (tallEnoughRects[r].y + tallEnoughRects[r].height > yMax)
	    yMax = tallEnoughRects[r].y + tallEnoughRects[r].height;
	}
	
        Rect tallestRect(xMin, yMin, xMax - xMin, yMax - yMin);
	
	
        //cout << tallestRect.x << ":" << tallestRect.y << " -- " << tallestRect.width << ":" << tallestRect.height << endl;

        Rect cropRect(0, tallestRect.y - Y_OFFSET, tallestRect.width, tallestRect.height);

        //cout << "Cropped: " << cropRect.x << ":" << cropRect.y << " -- " << cropRect.width << ":" << cropRect.height << endl;
        Mat cropped(characterImg, cropRect);
        cvtColor(cropped, cropped, CV_BGR2GRAY);

        Rect destinationRect(xPos + (CHAR_HORIZ_OFFSET - TILE_WIDTH), yPos + (CHAR_VERT_OFFSET - TILE_HEIGHT + (TILE_HEIGHT - tallestRect.height)), tallestRect.width, tallestRect.height);

        //cout << "1" << endl;

        cropped.copyTo(bigTif(destinationRect));

        int x1 = destinationRect.x - CHAR_PADDING_HORIZONTAL;
        int y1 = (vertical_resolution - destinationRect.y - destinationRect.height) - CHAR_PADDING_VERTICAL;
        int x2 = (destinationRect.x + destinationRect.width) + CHAR_PADDING_HORIZONTAL;
        int y2 = (vertical_resolution - destinationRect.y) + CHAR_PADDING_VERTICAL;
        //0 70 5602 85 5636 0
        boxFileOut << charcode << " " << x1 << " " << y1 << " ";
        boxFileOut << x2 << " " << y2 ;
        boxFileOut << " 0" << endl;

        //rectangle(characterImg, tallestRect, Scalar(0, 255, 0));
        //imshow("characterImg", cropped);

        waitKey(2);
      }
    }

    imwrite("combined.tif", bigTif);
    ofstream boxFile("combined.box", std::ios::out);
    boxFile << boxFileOut.str();
  }
}
