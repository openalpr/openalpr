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
#include <sys/stat.h>

#include "postprocess/regexrule.h"
#include "licenseplatecandidate.h"
#include "utility.h"
#include "support/filesystem.h"
#include "ocr/ocrfactory.h"
#include "ocr/ocr.h"

using namespace std;
using namespace cv;
using namespace alpr;

// Given a directory full of lp images (named [statecode]#.png) crop out the alphanumeric characters.
// These will be used to train the OCR

#ifdef __APPLE__
const int LEFT_ARROW_KEY = 2;
const int RIGHT_ARROW_KEY = 3;

const int DOWN_ARROW_KEY = 1;
const int UP_ARROW_KEY= 0;

const int ENTER_KEY_ONE = 13;
const int ENTER_KEY_TWO = 10;

#elif WIN32
const int LEFT_ARROW_KEY = 2424832;
const int RIGHT_ARROW_KEY = 2555904;

const int DOWN_ARROW_KEY = 2621440;
const int UP_ARROW_KEY = 2490368;

const int ENTER_KEY_ONE = 13;
const int ENTER_KEY_TWO = 10;
#else
const int LEFT_ARROW_KEY = 1113937;
const int RIGHT_ARROW_KEY = 1113939;

const int DOWN_ARROW_KEY = 1113940;
const int UP_ARROW_KEY= 1113938;

const int ENTER_KEY_ONE = 1048586;
const int ENTER_KEY_TWO = 1048586;
#endif

const string SPACE = " ";
const int SPACE_KEY = 32;
const int ESCAPE_KEY = 27;

const int DASHBOARD_COLUMNS = 3;

void showDashboard(vector<Mat> images, vector<bool> selectedImages, int selectedIndex);
vector<string> showCharSelection(Mat image, vector<Rect> charRegions, string state);

int main( int argc, const char** argv )
{
  string country;
  string inDir;
  string outDir;
  Mat frame;

  //Check if user specify image to process
  if(argc == 4)
  {
    country = argv[1];
    inDir = argv[2];
    outDir = argv[3];
  }
  else
  {
    printf("Use:\n\t%s country indirectory outdirectory\n",argv[0]);
    printf("Ex: \n\t%s eu ./pics/ ./out\n",argv[0]);
    return 1;
  }

  if (DirectoryExists(outDir.c_str()) == false)
  {
    printf("Output dir does not exist\n");
    return 2;
  }

  cout << "Usage: " << endl;
  cout << "\tn		-- Next plate" << endl;
  cout << "\tp		-- Previous plate" << endl;
  cout << "\tW		-- Select image and save characters according to OCR results, then go to next image" << endl;
  cout << "\ts		-- Save characters" << endl;
  cout << "\t<- and ->	-- Cycle between images" << endl;
  cout << "\tEnt/space	-- Select plate" << endl;
  cout << endl;
  cout << "Within a plate" << endl;
  cout << "\t<- and ->		-- Cycle between characters" << endl;
  cout << "\t[0-9A-Z]		-- Identify a character (saves the image)" << endl;
  cout << "\tESC/Ent/Space	-- Back to plate selection" << endl;

  Config config(country);
  config.debugGeneral = false;
  config.debugCharAnalysis = false;
  config.debugCharSegmenter = false;
  OCR* ocr = createOcr(&config);

  if (DirectoryExists(inDir.c_str()))
  {
    vector<string> files = getFilesInDir(inDir.c_str());

    sort( files.begin(), files.end(), stringCompare );

    for (int i = 0; i< files.size(); i++)
    {
      if (hasEnding(files[i], ".png") || hasEnding(files[i], ".jpg"))
      {
        string fullpath = inDir + "/" + files[i];
        cout << fullpath << endl;
        frame = imread( fullpath.c_str() );
        if(frame.data == NULL)
        {
          cout << "Unable to read license plate: " << fullpath << endl;
          continue;
        }
        resize(frame, frame, Size(config.ocrImageWidthPx, config.ocrImageHeightPx));

        imshow ("Original", frame);

	PipelineData pipeline_data(frame, Rect(0, 0, frame.cols, frame.rows), &config);
	cvtColor(frame, frame, CV_BGR2GRAY);
	pipeline_data.crop_gray = Mat(frame, Rect(0, 0, frame.cols, frame.rows));
    pipeline_data.thresholds = produceThresholds(pipeline_data.crop_gray, &config);
    
        char statecode[3];
        statecode[0] = files[i][0];
        statecode[1] = files[i][1];
        statecode[2] = '\0';
        string statecodestr(statecode);

        CharacterAnalysis regionizer(&pipeline_data);
        
        if (pipeline_data.plate_inverted)
          bitwise_not(pipeline_data.crop_gray, pipeline_data.crop_gray);
        

        ocr->performOCR(&pipeline_data);
        ocr->postProcessor.analyze(statecodestr, 25);
        cout << "OCR results: " << ocr->postProcessor.bestChars << endl;

        vector<bool> selectedBoxes(pipeline_data.thresholds.size());
        for (int z = 0; z < pipeline_data.thresholds.size(); z++)
          selectedBoxes[z] = false;

        int curDashboardSelection = 0;

        vector<string> humanInputs(pipeline_data.charRegionsFlat.size());

        for (int z = 0; z < pipeline_data.charRegionsFlat.size(); z++)
          humanInputs[z] = SPACE;

        showDashboard(pipeline_data.thresholds, selectedBoxes, 0);

        int waitkey = waitKey(50);

        while ((char) waitkey != 'n' && (char) waitkey != 'p')	 // Next image
        {
          if (waitkey == LEFT_ARROW_KEY) // left arrow key
          {
            if (curDashboardSelection > 0)
              curDashboardSelection--;
            showDashboard(pipeline_data.thresholds, selectedBoxes, curDashboardSelection);
          }
          else if (waitkey == RIGHT_ARROW_KEY) // right arrow key
          {
            if (curDashboardSelection < pipeline_data.thresholds.size() - 1)
              curDashboardSelection++;
            showDashboard(pipeline_data.thresholds, selectedBoxes, curDashboardSelection);
          }
          else if (waitkey == DOWN_ARROW_KEY)
          {
            if (curDashboardSelection + DASHBOARD_COLUMNS <= pipeline_data.thresholds.size() - 1)
              curDashboardSelection += DASHBOARD_COLUMNS;
            showDashboard(pipeline_data.thresholds, selectedBoxes, curDashboardSelection);
          }
          else if (waitkey == UP_ARROW_KEY)
          {
            if (curDashboardSelection - DASHBOARD_COLUMNS >= 0)
              curDashboardSelection -= DASHBOARD_COLUMNS;
            showDashboard(pipeline_data.thresholds, selectedBoxes, curDashboardSelection);
          }
          else if (waitkey == ENTER_KEY_ONE || waitkey == ENTER_KEY_TWO)
          {
	    if (pipeline_data.charRegionsFlat.size() > 0)
	    {
	      vector<string> tempdata = showCharSelection(pipeline_data.thresholds[curDashboardSelection], pipeline_data.charRegionsFlat, statecodestr);
	      for (int c = 0; c < pipeline_data.charRegionsFlat.size(); c++)
		humanInputs[c] = tempdata[c];
	    }
	    else
	    {
	      cout << "No character regions available in this image" << endl;
	    }
          }
          else if ((char) waitkey == SPACE_KEY)
          {
            selectedBoxes[curDashboardSelection] = !selectedBoxes[curDashboardSelection];
            showDashboard(pipeline_data.thresholds, selectedBoxes, curDashboardSelection);
          }
          else if ((char) waitkey == 's' || (char) waitkey == 'S' )
          {

            bool somethingSelected = false;
            bool chardataTagged = false;
            for (int c = 0; c < pipeline_data.thresholds.size(); c++)
            {
              if (selectedBoxes[c])
              {
                somethingSelected = true;
                break;
              }
            }
            for (int c = 0; c < pipeline_data.charRegionsFlat.size(); c++)
            {
              if (humanInputs[c] != SPACE)
              {
                chardataTagged = true;
                break;
              }
            }
            // Save
            if (somethingSelected && chardataTagged)
            {
              for (int c = 0; c < pipeline_data.charRegionsFlat.size(); c++)
              {
                if (humanInputs[c] == SPACE)
                  continue;

                for (int t = 0; t < pipeline_data.thresholds.size(); t++)
                {
                  if (selectedBoxes[t] == false)
                    continue;

                  stringstream filename;
                  
                  // Ensure that crop rect does not extend beyond extent of image.
                  cv::Rect char_region = expandRect(pipeline_data.charRegionsFlat[c], 0, 0, 
                          pipeline_data.thresholds[t].cols,
                          pipeline_data.thresholds[t].rows);
                  
                  Mat cropped = pipeline_data.thresholds[t](char_region);
                  filename << outDir << "/" << humanInputs[c] << "-" << t << "-" << files[i];
                  imwrite(filename.str(), cropped);
                  cout << "Writing char image: " << filename.str() << endl;
                }
              }
            }
            else if (somethingSelected == false)
              cout << "Did not select any boxes" << endl;
            else if (chardataTagged == false)
              cout << "You have not tagged any characters" << endl;

            if ((char) waitkey == 'W')
            {
              waitkey = 'n';
              continue;
            }
          }

          waitkey = waitKey(50);
		  //std::cout << "key: " << (int) waitkey << std::endl;
        }

        if ((char) waitkey == 'p')
          i = i - 2;
        if (i < -1)
          i = -1;
      }
    }
  }
}

void showDashboard(vector<Mat> images, vector<bool> selectedImages, int selectedIndex)
{
  vector<Mat> vecCopy;

  if (selectedIndex < 0)
    selectedIndex = 0;
  if (selectedIndex >= images.size())
    selectedIndex = images.size() -1;

  for (int i = 0; i < images.size(); i++)
  {
    Mat imgCopy(images[i].size(), images[i].type());
    images[i].copyTo(imgCopy);
    cvtColor(imgCopy, imgCopy, CV_GRAY2BGR);
    if (i == selectedIndex)
    {
      rectangle(imgCopy, Point(1,1), Point(imgCopy.size().width - 1, imgCopy.size().height -1), Scalar(0, 255, 0), 1);
    }
    if (selectedImages[i])
    {
      rectangle(imgCopy, Point(2,2), Point(imgCopy.size().width - 2, imgCopy.size().height -2), Scalar(255, 0, 0), 1);
    }

    vecCopy.push_back(imgCopy);
  }

  Mat dashboard = drawImageDashboard(vecCopy, vecCopy[0].type(), DASHBOARD_COLUMNS);

  imshow("Selection dashboard", dashboard);
}

vector<string> showCharSelection(Mat image, vector<Rect> charRegions, string state)
{
  int curCharIdx = 0;

  vector<string> humanInputs(charRegions.size());
  for (int i = 0; i < charRegions.size(); i++)
    humanInputs[i] = SPACE;

  RegexRule regex_rule("", "[\\pL\\pN]", "", "");
  
  int16_t waitkey = waitKey(50);
  while (waitkey != ENTER_KEY_ONE && waitkey != ENTER_KEY_TWO && waitkey != ESCAPE_KEY)
  {
    Mat imgCopy(image.size(), image.type());
    image.copyTo(imgCopy);
    cvtColor(imgCopy, imgCopy, CV_GRAY2BGR);

    rectangle(imgCopy, charRegions[curCharIdx], Scalar(0, 255, 0), 1);

    imshow("Character selector", imgCopy);

    if ((char) waitkey == LEFT_ARROW_KEY)
      curCharIdx--;
    else if ((char) waitkey == RIGHT_ARROW_KEY)
      curCharIdx++;
    else if (waitkey == SPACE_KEY)
    {
      humanInputs[curCharIdx] = " ";
      curCharIdx++;
    }
    else if (waitkey > 0 && regex_rule.match(utf8chr(waitkey))) // Verify that it's an actual character
    {
      // Save the character to disk
      humanInputs[curCharIdx] = utf8chr(waitkey);
      curCharIdx++;

      if (curCharIdx >= charRegions.size())
      {
        waitkey = (int16_t) ENTER_KEY_ONE;
        break;
      }
    }

    if (curCharIdx < 0)
      curCharIdx = 0;
    if (curCharIdx >= charRegions.size())
      curCharIdx = charRegions.size() -1;

    waitkey = waitKey(50);
  }

  if (waitkey == ENTER_KEY_ONE || waitkey == ENTER_KEY_TWO)
  {
    // Save all the inputs
    for (int i = 0; i < charRegions.size(); i++)
    {
      if (humanInputs[i] != SPACE)
        cout << "Tagged " << state << " char code: '" << humanInputs[i] << "' at char position: " << i << endl;
    }
  }

  destroyWindow("Character selector");

  return humanInputs;
}
