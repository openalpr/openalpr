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
#include <fstream>
#include <stdio.h>
#include <ctype.h>

#include "support/filesystem.h"
#include "config.h"



#ifdef __APPLE__
const int LEFT_ARROW_KEY = 2;
const int RIGHT_ARROW_KEY = 3;
const int SPACE_KEY = 32;
const int ENTER_KEY = 13;
const int ESCAPE_KEY = 27;
const int BACKSPACE_KEY = 8;

const int DOWN_ARROW_KEY = 1;
const int UP_ARROW_KEY= 0;

#elif WINDOWS
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
const int LEFT_ARROW_KEY = 37; // VK_LEFT
const int RIGHT_ARROW_KEY = 39; // VK_RIGHT
const int SPACE_KEY = 32; // VK_SPACE
const int ENTER_KEY = 13; // VK_RETURN
const int ESCAPE_KEY = 27; // VK_ESCAPE 
const int BACKSPACE_KEY = 8; // VK_BACK

const int DOWN_ARROW_KEY = 40; // VK_DOWN
const int UP_ARROW_KEY= 38; // VK_UP
#else
const int LEFT_ARROW_KEY = 81;
const int RIGHT_ARROW_KEY = 83;
const int SPACE_KEY = 32;
const int ENTER_KEY = 10;
const int ESCAPE_KEY = 27;
const int BACKSPACE_KEY = 8;

const int DOWN_ARROW_KEY = 84;
const int UP_ARROW_KEY= 82;

#endif


using namespace std;
using namespace cv;
using namespace alpr;

static bool ldragging = false;
static int xPos1 = 0;
static int yPos1 = 0;
static int xPos2 = 0;
static int yPos2 = 0;
float ASPECT_RATIO = 1.404;

static bool rdragging = false;
static int rDragStartX = 0;
static int rDragStartY = 0;

void mouseCallback(int event, int x, int y, int flags, void* userdata)
{
  if  ( event == EVENT_LBUTTONDOWN )
  {
    ldragging = true;
    xPos1 = x;
    yPos1 = y;
    xPos2 = x;
    yPos2 = y;
  }
  else if  ( event == EVENT_LBUTTONUP )
  {
    ldragging = false;
  }
  else if  ( event == EVENT_RBUTTONDOWN )
  {
    rdragging = true;
    rDragStartX = x;
    rDragStartY = y;
  }
  else if  ( event == EVENT_RBUTTONUP )
  {
    rdragging = false;
  }
  else if ( event == EVENT_MOUSEMOVE )
  {
    if (ldragging && x > xPos1 && y > yPos1)
    {
      int w = x - xPos1;
      int h = (int) (((float) w) / ASPECT_RATIO);
      
      xPos2 = x;
      yPos2 = yPos1 + h;
      
    }
    else if (rdragging)
    {
      int xDiff = x - rDragStartX;
      int yDiff = y - rDragStartY;
      
      xPos1 += xDiff;
      yPos1 += yDiff;
      xPos2 += xDiff;
      yPos2 += yDiff;
      
      rDragStartX = x;
      rDragStartY = y;
    }

  }
}



int main( int argc, const char** argv )
{
  string country;
  string inDir;
  string outDir;

  //Check if user specify image to process
  if(argc == 4)
  {
    country = argv[1];
    inDir = argv[2];
    outDir = argv[3];
  }
  else
  {
    printf("Use:\n\t%s [country] [img input dir] [data output dir]\n",argv[0]);
    printf("\tex: %s us ./usimages ./usdata\n",argv[0]);
    printf("\n\n");
    return 0;
  }

  if (DirectoryExists(inDir.c_str()) == false)
  {
    printf("Input dir does not exist\n");
    return 0;
  }
  if (DirectoryExists(outDir.c_str()) == false)
  {
    printf("Output dir does not exist\n");
    return 0;
  }

  Config config(country);
  ASPECT_RATIO = config.plateWidthMM / config.plateHeightMM;
  
  vector<string> files = getFilesInDir(inDir.c_str());
  
  vector<string> imgFiles;
  sort( files.begin(), files.end(), stringCompare );
  
  
  for (int i = 0; i < files.size(); i++)
  {
    if (hasEnding(files[i], ".png") || hasEnding(files[i], ".jpg"))
    {
      imgFiles.push_back(files[i]);
    }
  }
  

  for (int i = 0; i< imgFiles.size(); i++)
  {

    cout << "Loading: " << imgFiles[i] << " (" << (i+1) << "/" << imgFiles.size() << ")" << endl;
    
    string fullimgpath = inDir + "/" + imgFiles[i];
    
    Mat frame = imread( fullimgpath.c_str() );

    if (frame.cols == 0 || frame.rows == 0)
      continue;
    
    
    string curplatetag = "";
	  //Create a window
    namedWindow("Input image", 1);

    //set the callback function for any mouse event
    setMouseCallback("Input image", mouseCallback, NULL);
    
    char key = (char) cv::waitKey(50);
    while (key != ENTER_KEY)
    {
      
      if ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'z'))
      {
	curplatetag = curplatetag + (char) toupper( key );
      }
      else if (key == BACKSPACE_KEY)
      {
	curplatetag = curplatetag.substr(0, curplatetag.size() - 1);
      }
      Mat tmpFrame(frame.size(), frame.type());
      frame.copyTo(tmpFrame);
      rectangle(tmpFrame, Point(xPos1, yPos1), Point(xPos2, yPos2), Scalar(0, 0, 255), 2);
      
      rectangle(tmpFrame, Point(xPos1, yPos1 - 35), Point(xPos1 + 175, yPos1 - 5), Scalar(255, 255, 255), CV_FILLED);
      putText(tmpFrame, curplatetag, Point(xPos1 + 2, yPos1 - 10), FONT_HERSHEY_PLAIN, 1.5, Scalar(100,50,0), 2);
      
      imshow("Input image", tmpFrame);
      
      key = cv::waitKey(50);
    }
    
    if (curplatetag != "")
    {
      // Write the data to disk
      ofstream outputdatafile;

      std::string outputTextFile = outDir + "/" + filenameWithoutExtension(imgFiles[i]) + ".txt";
      outputdatafile.open(outputTextFile.c_str());
      
      outputdatafile << imgFiles[i] << "\t" << xPos1 << "\t" << yPos1 << "\t" << (xPos2 - xPos1) << "\t" << (yPos2 - yPos1) << "\t" << curplatetag << endl;
      outputdatafile.close();
    }
    
  }
  
}

