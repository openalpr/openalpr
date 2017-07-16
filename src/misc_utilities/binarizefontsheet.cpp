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
#include <string>
#include <vector>
#include "../tclap/CmdLine.h"
#include "utility.h"
#include "support/utf8.h"

using namespace std;

using namespace std;
using namespace cv;
using namespace alpr;


// This utility operates on a large image file generated from a TTF font file
// The font sheet is used to train OCR.  The process is:
//    Find the exact ttf font used by the number plates.
//    Generate text with all of the characters that could be on a license plate
//    Print the pages on a piece of paper
//    Take pictures with a digital camera under different lighting conditions to add realistic noise
//    Binarize and process each character with OpenALPR morphology functions to make them look similar to how OpenALPR sees them.  Produce a tif file.
//    Produce a box file based on text from #2 and the image from #5.
//    Train OCR with this box/tif data 


// This utility is used before "prepcharsfortraining"
// Given a series of images (font sheets) and a text file giving the order, it will pluck out each character, binarize it,
// and output it to a single image file for each character.

// These characters can later be reassembled into a tif/box file using prepcharsfortraining.


 
 

bool sort_lined_rectangles(Rect i, Rect j) {
  
  // If they're on different lines
  if (abs(i.y - j.y) > 15)
  {
    return i.y < j.y;
  }
  
  // They're on the same line, give the left-most.
  return (i.x < j.x); 
}

void show_debug_image(vector<Rect> rectangles, Mat img)
{
    Mat debugImg;
    cvtColor(img, debugImg, CV_GRAY2BGR);
    for (unsigned int i = 0; i < rectangles.size(); i++)
    {
      Rect mr = rectangles[i];
      Mat croppedChar = img(mr);

      rectangle(debugImg, mr, Scalar(0,0,255), 2);
      putText(debugImg, toString(i), mr.tl(), FONT_HERSHEY_PLAIN, 1.3, Scalar(0,0,0), 2);
    }
    float new_height = 1000;
    float aspect_ratio = ((float)debugImg.rows) / ((float) new_height);
    float new_width = ((float) debugImg.cols) / aspect_ratio;
    resize(debugImg, debugImg, Size(new_width, new_height));
    drawAndWait(&debugImg);
}

int main(int argc, char** argv) {

  const int MIN_RECTANGLE_AREA_PIXELS = 500;
  const int MIN_SPECKLE_AREA_PIXELS = 20;
  const int BLOBBER_EROSION_SIZE=6;
  
  vector<string> font_sheet_files;
  string char_list_file;
  string out_dir;
  bool debug;
  

  TCLAP::CmdLine cmd("OpenAlpr OCR Training Font Sheet Prep Utility", ' ', "1.0.0");

  TCLAP::UnlabeledMultiArg<std::string>  fontSheetArg( "font_sheet", "List of font sheet images", true, "", "font_sheet"  );

  TCLAP::ValueArg<std::string> charListArg("","character_file","Text file with the text/order of the individual characters in the font sheets",true, "" ,"character_file");
  
  TCLAP::ValueArg<std::string> outDirArg("","out_dir","Output directory to put the character images",true, "" ,"output_dir");
  
  TCLAP::SwitchArg debugSwitch("","debug","Enable debug output.  Default=off", cmd, false);
  
  try
  {
    cmd.add( fontSheetArg );
    cmd.add( charListArg );
    cmd.add( outDirArg );

    
    if (cmd.parse( argc, argv ) == false)
    {
      // Error occurred while parsing.  Exit now.
      return 1;
    }

    font_sheet_files = fontSheetArg.getValue();
    char_list_file = charListArg.getValue();
    out_dir = outDirArg.getValue();
    debug = debugSwitch.getValue();
    
  }
  catch (TCLAP::ArgException &e)    // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }
  
  
  
  if (DirectoryExists(out_dir.c_str()) == false)
  {
    cout << "Output dir: " << out_dir << " does not exist" << endl;
    return 1;
  }
  
  if (fileExists(char_list_file.c_str()) == false)
  {
    cout << "Character text file: " << char_list_file << " does not exist" << endl;
    return 1;
  }
  
  // Verify all the font sheet files exist
  for (unsigned int i = 0; i < font_sheet_files.size(); i++)
  {
    if (fileExists(font_sheet_files[i].c_str()) == false)
    {
      cout << "Font sheet image: " << font_sheet_files[i] << " does not exist." << endl;
      return 1;
    }
  }
  
  // Read the text content from the character list file
  std::ifstream fs(char_list_file.c_str());
  std::string text_content((std::istreambuf_iterator<char>(fs)),
             std::istreambuf_iterator<char>());
  fs.close();
  
  for (unsigned int font_sheet_index = 0; font_sheet_index < font_sheet_files.size(); font_sheet_index++)
  {
    cout << "Processing: " << font_sheet_files[font_sheet_index] << endl;
    
    Mat frame = cv::imread( font_sheet_files[font_sheet_index] );

    Config config("us");
    
    cvtColor(frame, frame, CV_BGR2GRAY);
    vector<Mat> thresholds = produceThresholds(frame, &config);
    
    
    for (unsigned int t = 0; t < thresholds.size(); t++)
    {
      
      // First clean up any tiny speckles
      Mat speckle_copy(thresholds[t].size(), thresholds[t].type());
      thresholds[t].copyTo(speckle_copy);
      
      
      vector<vector<Point> > speckle_contours;
      vector<Vec4i> speckle_hierarchy;
      findContours(speckle_copy, speckle_contours, speckle_hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
      Mat testImg = Mat::zeros(thresholds[t].size(), thresholds[t].type());
      
      for (unsigned int i = 0; i < speckle_contours.size(); i++)
      {
        Rect speckleRect = boundingRect(speckle_contours[i]);

        if (speckleRect.area() < MIN_SPECKLE_AREA_PIXELS)
        {
          drawContours(thresholds[t], speckle_contours, i, Scalar(0,0,0), CV_FILLED);
          drawContours(testImg, speckle_contours, i, Scalar(255,255,255), CV_FILLED);
        }
      }
      resize(testImg, testImg, Size(700, 1000));
      
      if (debug)
        drawAndWait(&testImg);
      
      // Adjust the threshold w/ the morphology operation that OpenALPR uses
      Mat closureElement = getStructuringElement( 1,
               Size( 2 + 1, 2+1 ),
               Point( 1, 1 ) );
      morphologyEx(thresholds[t], thresholds[t], MORPH_CLOSE, closureElement);
      
      Mat blobby;
      Mat element = getStructuringElement( MORPH_RECT,
                                         Size( 2*BLOBBER_EROSION_SIZE + 1, 2*BLOBBER_EROSION_SIZE+1 ),
                                         Point( BLOBBER_EROSION_SIZE, BLOBBER_EROSION_SIZE ) );
      dilate(thresholds[t], blobby, element );
      erode(blobby, blobby, element);

      
      vector<vector<Point> > contours;
      vector<Vec4i> hierarchy;
      findContours(blobby, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

      bitwise_not(thresholds[t], thresholds[t]);

      vector<Rect> rectangles;
      for (unsigned int i = 0; i < contours.size(); i++)
      {
        Rect mr = boundingRect(contours[i]);
        
        if (mr.area() >= MIN_RECTANGLE_AREA_PIXELS)
          rectangles.push_back(mr);
        
      }
      // sort the rectangles top to bottom left to right
      std::sort(rectangles.begin(), rectangles.end(), sort_lined_rectangles);
      
      //cout << text_content << endl;
      string::iterator end_it = utf8::find_invalid(text_content.begin(), text_content.end());
      if (end_it != text_content.end()) {
        cout << "Invalid UTF-8 encoding detected " << endl;
        return 1;
      }
      
      if (debug)
        show_debug_image(rectangles, thresholds[t]);
      
      vector<std::string> valid_characters;
      
      string::iterator utf_iterator = text_content.begin();
      while (utf_iterator < text_content.end())
      {
        int cp = utf8::next(utf_iterator, text_content.end());
        string utf_character = utf8chr(cp);
        if (utf_character != "\n" && utf_character != " " && utf_character != " ")
          valid_characters.push_back(utf_character);
      }
      
      if (rectangles.size() != valid_characters.size())
      {
        cout << "Number of blobs (" << rectangles.size() << ") != number of characters (" << valid_characters.size() << ")" << endl;
        cout << "Skipping..." << endl;
        //return 1;
        continue;
      }
      
      
      for (unsigned int i = 0; i < rectangles.size(); i++)
      {
        Rect mr = rectangles[i];
        Mat croppedChar = thresholds[t](mr);
        
        stringstream ss;
        ss << out_dir << "/" << valid_characters[i] << "-" << font_sheet_index << "-" << t << "-" << i << ".png";
        
        imwrite(ss.str(), croppedChar);
        
      }
      show_debug_image(rectangles, thresholds[t]);
      
    }
  }
  
  return 0;
}

