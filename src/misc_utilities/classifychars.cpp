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

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <sys/stat.h>

#include "regiondetector.h"
#include "licenseplatecandidate.h"
#include "stateidentifier.h"
#include "utility.h"
#include "support/filesystem.h"
#include "ocr.h"

using namespace std;
using namespace cv;

// Given a directory full of lp images (named [statecode]#.png) crop out the alphanumeric characters.
// These will be used to train the OCR

#ifdef __APPLE__
const int LEFT_ARROW_KEY = 2;
const int RIGHT_ARROW_KEY = 3;
const int SPACE_KEY = 32;
const int ENTER_KEY = 13;
const int ESCAPE_KEY = 27;

const int DOWN_ARROW_KEY = 1;
const int UP_ARROW_KEY= 0;
const int DASHBOARD_COLUMNS = 9;

#else
const int LEFT_ARROW_KEY = 81;
const int RIGHT_ARROW_KEY = 83;
const int SPACE_KEY = 32;
const int ENTER_KEY = 10;
const int ESCAPE_KEY = 27;

const int DOWN_ARROW_KEY = 84;
const int UP_ARROW_KEY= 82;
const int DASHBOARD_COLUMNS = 3;

#endif

void showDashboard(vector<Mat> images, vector<bool> selectedImages, int selectedIndex);
vector<char> showCharSelection(Mat image, vector<Rect> charRegions, string state);

int main( int argc, const char** argv )
{
  string inDir;
  string outDir;
  Mat frame;

  //Check if user specify image to process
  if(argc == 3)
  {
    inDir = argv[1];
    outDir = argv[2];
  }
  else
  {
    printf("Use:\n\t%s indirectory outdirectory\n",argv[0]);
    printf("Ex: \n\t%s ./pics/  ./out  \n",argv[0]);
    return 0;
  }

  if (DirectoryExists(outDir.c_str()) == false)
  {
    printf("Output dir does not exist\n");
    return 0;
  }

  cout << "Usage: " << endl;
  cout << "\tn		-- Next plate" << endl;
  cout << "\tp		-- Previous plate" << endl;
  cout << "\ts		-- Save characters" << endl;
  cout << "\t<- and ->	-- Cycle between images" << endl;
  cout << "\tEnt/space	-- Select plate" << endl;
  cout << endl;
  cout << "Within a plate" << endl;
  cout << "\t<- and ->		-- Cycle between characters" << endl;
  cout << "\t[0-9A-Z]		-- Identify a character (saves the image)" << endl;
  cout << "\tESC/Ent/Space	-- Back to plate selection" << endl;

  Config* config = new Config("eu");
  OCR ocr(config);

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
        resize(frame, frame, Size(config->ocrImageWidthPx, config->ocrImageHeightPx));

        imshow ("Original", frame);

        char statecode[3];
        statecode[0] = files[i][0];
        statecode[1] = files[i][1];
        statecode[2] = '\0';
        string statecodestr(statecode);

        CharacterRegion regionizer(frame, config);

        if (abs(regionizer.getTopLine().angle) > 4)
        {
          // Rotate image:
          Mat rotated(frame.size(), frame.type());
          Mat rot_mat( 2, 3, CV_32FC1 );
          Point center = Point( frame.cols/2, frame.rows/2 );

          rot_mat = getRotationMatrix2D( center, regionizer.getTopLine().angle, 1.0 );
          warpAffine( frame, rotated, rot_mat, frame.size() );

          rotated.copyTo(frame);
        }

        CharacterSegmenter charSegmenter(frame, regionizer.thresholdsInverted(), config);

        //ocr.cleanCharRegions(charSegmenter.thresholds, charSegmenter.characters);

        ocr.performOCR(charSegmenter.getThresholds(), charSegmenter.characters);
        ocr.postProcessor->analyze(statecodestr, 25);
        cout << "OCR results: " << ocr.postProcessor->bestChars << endl;

        vector<bool> selectedBoxes(charSegmenter.getThresholds().size());
        for (int z = 0; z < charSegmenter.getThresholds().size(); z++)
          selectedBoxes[z] = false;

        int curDashboardSelection = 0;

        vector<char> humanInputs(charSegmenter.characters.size());

        for (int z = 0; z < charSegmenter.characters.size(); z++)
          humanInputs[z] = ' ';

        showDashboard(charSegmenter.getThresholds(), selectedBoxes, 0);

        char waitkey = (char) waitKey(50);

        while (waitkey != 'n' && waitkey != 'p')	 // Next image
        {
          if (waitkey == LEFT_ARROW_KEY) // left arrow key
          {
            if (curDashboardSelection > 0)
              curDashboardSelection--;
            showDashboard(charSegmenter.getThresholds(), selectedBoxes, curDashboardSelection);
          }
          else if (waitkey == RIGHT_ARROW_KEY) // right arrow key
          {
            if (curDashboardSelection < charSegmenter.getThresholds().size() - 1)
              curDashboardSelection++;
            showDashboard(charSegmenter.getThresholds(), selectedBoxes, curDashboardSelection);
          }
          else if (waitkey == DOWN_ARROW_KEY)
          {
            if (curDashboardSelection + DASHBOARD_COLUMNS <= charSegmenter.getThresholds().size() - 1)
              curDashboardSelection += DASHBOARD_COLUMNS;
            showDashboard(charSegmenter.getThresholds(), selectedBoxes, curDashboardSelection);
          }
          else if (waitkey == UP_ARROW_KEY)
          {
            if (curDashboardSelection - DASHBOARD_COLUMNS >= 0)
              curDashboardSelection -= DASHBOARD_COLUMNS;
            showDashboard(charSegmenter.getThresholds(), selectedBoxes, curDashboardSelection);
          }
          else if (waitkey == ENTER_KEY)
          {
            vector<char> tempdata = showCharSelection(charSegmenter.getThresholds()[curDashboardSelection], charSegmenter.characters, statecodestr);
            for (int c = 0; c < charSegmenter.characters.size(); c++)
              humanInputs[c] = tempdata[c];
          }
          else if (waitkey == SPACE_KEY)
          {
            selectedBoxes[curDashboardSelection] = !selectedBoxes[curDashboardSelection];
            showDashboard(charSegmenter.getThresholds(), selectedBoxes, curDashboardSelection);
          }
          else if (waitkey == 's' || waitkey == 'S')
          {
            bool somethingSelected = false;
            bool chardataTagged = false;
            for (int c = 0; c < charSegmenter.getThresholds().size(); c++)
            {
              if (selectedBoxes[c])
              {
                somethingSelected = true;
                break;
              }
            }
            for (int c = 0; c < charSegmenter.characters.size(); c++)
            {
              if (humanInputs[c] != ' ')
              {
                chardataTagged = true;
                break;
              }
            }
            // Save
            if (somethingSelected && chardataTagged)
            {
              for (int c = 0; c < charSegmenter.characters.size(); c++)
              {
                if (humanInputs[c] == ' ')
                  continue;

                for (int t = 0; t < charSegmenter.getThresholds().size(); t++)
                {
                  if (selectedBoxes[t] == false)
                    continue;

                  stringstream filename;
                  Mat cropped = charSegmenter.getThresholds()[t](charSegmenter.characters[c]);
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
          }

          waitkey = (char) waitKey(50);
        }

        if (waitkey == 'p')
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

vector<char> showCharSelection(Mat image, vector<Rect> charRegions, string state)
{
  int curCharIdx = 0;

  vector<char> humanInputs(charRegions.size());
  for (int i = 0; i < charRegions.size(); i++)
    humanInputs[i] = (char) SPACE_KEY;

  char waitkey = (char) waitKey(50);
  while (waitkey != ENTER_KEY && waitkey != ESCAPE_KEY)
  {
    Mat imgCopy(image.size(), image.type());
    image.copyTo(imgCopy);
    cvtColor(imgCopy, imgCopy, CV_GRAY2BGR);

    rectangle(imgCopy, charRegions[curCharIdx], Scalar(0, 255, 0), 1);

    imshow("Character selector", imgCopy);

    if (waitkey == LEFT_ARROW_KEY)
      curCharIdx--;
    else if (waitkey == RIGHT_ARROW_KEY )
      curCharIdx++;
    else if ((waitkey >= '0' && waitkey <= '9') || (waitkey >= 'a' && waitkey <= 'z') || waitkey == SPACE_KEY)
    {
      // Save the character to disk
      humanInputs[curCharIdx] = toupper((char) waitkey);
      curCharIdx++;

      if (curCharIdx >= charRegions.size())
      {
        waitkey = (char) ENTER_KEY;
        break;
      }
    }

    if (curCharIdx < 0)
      curCharIdx = 0;
    if (curCharIdx >= charRegions.size())
      curCharIdx = charRegions.size() -1;

    waitkey = (char) waitKey(50);
  }

  if (waitkey == ENTER_KEY)
  {
    // Save all the inputs
    for (int i = 0; i < charRegions.size(); i++)
    {
      if (humanInputs[i] != (char) SPACE_KEY)
        cout << "Tagged " << state << " char code: '" << humanInputs[i] << "' at char position: " << i << endl;
    }
  }

  destroyWindow("Character selector");

  return humanInputs;
}
