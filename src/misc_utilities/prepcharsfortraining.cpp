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

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sys/stat.h>
#include "support/filesystem.h"
#include "../tclap/CmdLine.h"
#include "support/utf8.h"

using namespace std;
using namespace cv;
using namespace alpr;

// Takes a directory full of single char images, and plops them on a big tif files
// Also creates a box file so Tesseract can recognize it
int main( int argc, const char** argv )
{
  string inDir;
  int tile_width;
  int tile_height;

  TCLAP::CmdLine cmd("OpenAlpr OCR Training Prep Utility", ' ', "1.0.0");

  TCLAP::UnlabeledValueArg<std::string>  inputDirArg( "input_dir", "Folder containing individual character images", true, "", "input_dir_path"  );

  
  TCLAP::ValueArg<int> tileWidthArg("","tile_width","Width (in pixels) for each character tile.  Default=50",false, 50 ,"tile_width_px");
  TCLAP::ValueArg<int> tileHeightArg("","tile_height","Height (in pixels) for each character tile.  Default=60",false, 60 ,"tile_height_px");
  
  try
  {
    cmd.add( inputDirArg );
    cmd.add( tileWidthArg );
    cmd.add( tileHeightArg );

    
    if (cmd.parse( argc, argv ) == false)
    {
      // Error occured while parsing.  Exit now.
      return 1;
    }

    inDir = inputDirArg.getValue();
    tile_width = tileWidthArg.getValue();
    tile_height = tileHeightArg.getValue();
    
  }
  catch (TCLAP::ArgException &e)    // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }
  
  

  if (DirectoryExists(inDir.c_str()) == false)
  {
    printf("Input dir does not exist\n");
    return 1;
  }



  const int CHAR_PADDING_HORIZONTAL = 0;
  const int CHAR_PADDING_VERTICAL = 0;

  const int X_OFFSET = 5;
  const int Y_OFFSET = 5;

  const int PAGE_MARGIN_X = 70;
  const int PAGE_MARGIN_Y = 70;
  const int HORIZONTAL_RESOLUTION = 3500;
  const int MAX_VERTICAL_RESOLUTION = 6000; // Maximum vertical size before chopping into additional pages.

  const int TILE_WIDTH = tile_width;
  const int TILE_HEIGHT = tile_height;

  const int FIXED_CHAR_HEIGHT = 40; // RESIZE all characters to this height

  vector<string> all_files = getFilesInDir(inDir.c_str());

  sort( all_files.begin(), all_files.end(), stringCompare );
  
  vector<string> files;
  
  for (int i = 0; i< all_files.size(); i++)
  {
    if (hasEnding(all_files[i], ".png") || hasEnding(all_files[i], ".jpg"))
    {
      files.push_back(all_files[i]);
    }
  }


  int tiles_per_row = ((float) (HORIZONTAL_RESOLUTION - (PAGE_MARGIN_X * 2))) / ((float) TILE_WIDTH) + 1;
  int lines = files.size() / (tiles_per_row);
  int vertical_resolution = ((lines + 1) * TILE_HEIGHT) + (PAGE_MARGIN_Y * 2) ;
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

      cout << "Processing file: " << (i + 1) << " of " << files.size() << " (" << files[i] << ")" << endl;

      string::iterator utf_iterator = files[i].begin();
      int cp = utf8::next(utf_iterator, files[i].end());
      string charcode = utf8chr(cp);

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

      Rect destinationRect(xPos, yPos, tallestRect.width, tallestRect.height);
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
