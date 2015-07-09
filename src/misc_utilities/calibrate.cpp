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

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "support/filesystem.h"
#include "../tclap/CmdLine.h"
#include "prewarp.h"

#include <sstream>

using namespace std;
using namespace cv;

const int INSTRUCTIONS_HEIGHT = 32;


bool panning;
bool left_clicking;
Point left_click_start;
Point left_click_cur;


Point lastPos;

float w;
float h;
float panX = 0;
float panY = 0;
float rotationx = 0;
float rotationy = 0;
float rotationz = 0;
float stretchX = 1.0;
float dist = 1.0;

Mat imgOriginal;
Mat curWarpedImage;

alpr::Config config("us");

const string WINDOW_NAME = "Adjust OpenALPR Perspective";

string get_config()
{
  stringstream output;
  output << "planar," << std::fixed;
  output << w << "," << h << ",";
  output << rotationx << "," << rotationy << "," << rotationz << ",";
  output << stretchX << "," << dist << ",";
  output << panX << "," << panY;
  
  return output.str();
}

void drawImage(Mat img)
{  

  config.prewarp = get_config();
  alpr::PreWarp prewarp(&config);
  

  if (!left_clicking)
  {
    curWarpedImage = prewarp.warpImage(img);
  }

  
  Mat imgWithInstructions(curWarpedImage.rows + INSTRUCTIONS_HEIGHT, curWarpedImage.cols, curWarpedImage.type());
  
  curWarpedImage.copyTo(imgWithInstructions(Rect(0, INSTRUCTIONS_HEIGHT, curWarpedImage.cols, curWarpedImage.rows)));

  if (left_clicking)
  {
    Point start = left_click_start;
    Point end = left_click_cur;
    start.y += INSTRUCTIONS_HEIGHT;
    end.y += INSTRUCTIONS_HEIGHT;
    rectangle(imgWithInstructions, start, end, Scalar(255,0,255), 2);
  }
  
  rectangle(imgWithInstructions, Point(0,0), Point(curWarpedImage.cols, INSTRUCTIONS_HEIGHT), Scalar(255,255,255), -1);

  putText(imgWithInstructions, "Press 'o' to output config to console.", 
          Point(5,25), FONT_HERSHEY_DUPLEX, 1.0, Scalar(0,0,0));
  
  
  imshow(WINDOW_NAME, imgWithInstructions);
  
}




void mouse_callback(int event, int x, int y, int flags, void* userdata)
{
  y = y - INSTRUCTIONS_HEIGHT;
  if (y < 0)
    return;
  
  
  if (event == EVENT_RBUTTONDOWN)
  {
    lastPos.x = x;
    lastPos.y = y;
    panning = true;
  }
  if (event == EVENT_RBUTTONUP)
  {
    panning = false;
    drawImage(imgOriginal);
  }
  if (event == EVENT_MOUSEMOVE && panning)
  {
    int xdiff = x - lastPos.x;
    int ydiff = y - lastPos.y;
    panX -= xdiff;
    panY -= ydiff;
    
    lastPos.x = x;
    lastPos.y = y;
    
    // Reduce the computation by only doing it every 3rd pixel
    if (x % 3 == 0 && y % 3 == 0)
    {
      drawImage(imgOriginal);
    }
  }
  
  if (event == EVENT_LBUTTONDOWN)
  {
    left_click_start.x = x;
    left_click_start.y = y;
    left_clicking = true;
  }
  if (event == EVENT_LBUTTONUP)
  {
    left_clicking = false;
    drawImage(imgOriginal);
  }
  if (event == EVENT_MOUSEMOVE && left_clicking)
  {
    left_click_cur.x = x;
    
    float IDEAL_PLATE_RATIO = config.plateWidthMM / config.plateHeightMM;
    float curWidth = left_click_cur.x - left_click_start.x;
    
    left_click_cur.y = left_click_start.y + (curWidth / IDEAL_PLATE_RATIO );
    
    // Reduce the computation by only doing it every 3rd pixel
    if (x % 2 == 0 || y % 2 == 0)
    {
      drawImage(imgOriginal);
    }
  }
  
}

void ZChange(int pos, void* userdata)
{
  
  rotationz = -((float)pos - 100) / 100.0;

  drawImage(imgOriginal);
}

void XChange(int pos, void* userdata)
{
  
  rotationx = -((float)pos - 100) / 20000.0;
  
  drawImage(imgOriginal);
}


void YChange(int pos, void* userdata)
{
  rotationy = ((float)pos - 100) / 20000.0;
 
  drawImage(imgOriginal);
}


void DistChange(int pos, void* userdata)
{
  dist = 1.0 - ((float)pos - 100) / 200.0;
 
  drawImage(imgOriginal);
}

void StretchChange(int pos, void* userdata)
{
  stretchX = 1.0 + ((float)pos - 100) / -200.0;
 
  drawImage(imgOriginal);
}


int value;
void create_window()
{
  namedWindow(WINDOW_NAME, CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED);
  
  
  value = 100;
  panX = 0;
  panY = 0;

  XChange(100, NULL);
  YChange(100, NULL);
  ZChange(100, NULL);
  DistChange(100, NULL);
  
  
    createTrackbar( "X", WINDOW_NAME, &value, 200,  XChange);
    createTrackbar( "Y", WINDOW_NAME, &value, 200,  YChange);
    createTrackbar( "Z", WINDOW_NAME, &value, 200,  ZChange);
    createTrackbar( "W", WINDOW_NAME, &value, 200,  StretchChange);
    createTrackbar( "D", WINDOW_NAME, &value, 200,  DistChange);

  
  setMouseCallback(WINDOW_NAME, mouse_callback, NULL);

   
}

/*
 * 
 */
int main(int argc, char** argv) {


  string filename;
  string country;
  string config_path;
  string translate_config;
  int max_width;
  int max_height;

  TCLAP::CmdLine cmd("OpenAlpr Perspective Utility", ' ', "0.1");

  TCLAP::UnlabeledValueArg<std::string>  fileArg( "image_file", "Image containing license plates", true, "", "image_file_path"  );

  TCLAP::ValueArg<std::string> countryCodeArg("c","country","Country code to identify (either us for USA or eu for Europe).  Default=us",false, "us" ,"country_code");
  
  TCLAP::ValueArg<std::string> configFileArg("","config","Path to the openalpr.conf file",false, "" ,"config_file");
  
  TCLAP::ValueArg<std::string> translateTestArg("t","test","Test an image using the provided translation config",false, "" ,"prewarp config");
  
  TCLAP::ValueArg<int> maxWidthArg("w", "maxwidth", "Max Width used for displaying image in this utility.  Default=1280",false, 1280 ,"max width");
  TCLAP::ValueArg<int> maxHeightArg("", "maxheight", "Max Height used for displaying image in this utility.  Default=1024",false, 1024 ,"max height");
  
  try
  {
    cmd.add( configFileArg );
    cmd.add( fileArg );
    cmd.add( countryCodeArg );
    cmd.add( translateTestArg );
    cmd.add( maxWidthArg );
    cmd.add( maxHeightArg );

    
    if (cmd.parse( argc, argv ) == false)
    {
      // Error occured while parsing.  Exit now.
      return 1;
    }

    filename = fileArg.getValue();
    country = countryCodeArg.getValue();
    config_path = configFileArg.getValue();
    translate_config = translateTestArg.getValue();
    max_width = maxWidthArg.getValue();
    max_height = maxHeightArg.getValue();

  }
  catch (TCLAP::ArgException &e)    // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }
  
  if (!alpr::fileExists(filename.c_str()))
  {
    cerr << "Could not find image file: " << filename << endl;
  }
  
  config = alpr::Config(country);
  
  panning = false;
  left_clicking = false;

  
  imgOriginal = imread(filename);
  
  if (imgOriginal.cols > max_width)
  {
    float aspect = max_width / ((float)imgOriginal.cols);
    float y = ((float)imgOriginal.rows) * aspect;
    
    resize(imgOriginal, imgOriginal, Size((int) max_width, (int) y));
  }
  if (imgOriginal.rows > max_height)
  {
    float aspect = max_height / ((float)imgOriginal.rows);
    float x = ((float)imgOriginal.cols) * aspect;
    
    resize(imgOriginal, imgOriginal, Size((int) x, (int) max_height));
  }
  
  w = imgOriginal.cols;
  h = imgOriginal.rows;
  
  create_window();
  
  
  if (translate_config != "")
  {
    int first_comma = translate_config.find(",");
    
    
    string name = translate_config.substr(0, first_comma);
    stringstream ss(translate_config.substr(first_comma + 1, translate_config.length()));
    
    ss >> w;
    ss.ignore();
    ss >> h;
    ss.ignore();
    ss >> rotationx;
    ss.ignore();  // Ignore comma
    ss >> rotationy;
    ss.ignore();  // Ignore comma
    ss >> rotationz;
    ss.ignore();  // Ignore comma
    ss >> stretchX;
    ss.ignore();  // Ignore comma
    ss >> dist;
    ss.ignore();  // Ignore comma
    ss >> panX;
    ss.ignore();  // Ignore comma
    ss >> panY;
       
  }

  float width_ratio = w / ((float)imgOriginal.cols);
  float height_ratio = h / ((float)imgOriginal.rows);
  w = imgOriginal.cols;
  h = imgOriginal.rows;
  rotationx *=width_ratio;
  rotationy *=width_ratio;
  panX /= width_ratio;
  panY /= height_ratio;

  drawImage(imgOriginal);

  while (cvGetWindowHandle(WINDOW_NAME.c_str()) != 0)
  {

    char c = waitKey(15);

    if (c == 'o')
    {
      cout << "prewarp = " << get_config() << endl;
    } else if (c == 'q')
    {
      cout << "prewarp = " << get_config() << endl;
      break;
    }

  }

  cvDestroyAllWindows();
  
  
  return 0;
}

