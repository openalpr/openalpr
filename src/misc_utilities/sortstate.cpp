/*
 * Copyright (c) 2015 New Designs Unlimited, LLC
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

#include "licenseplatecandidate.h"
#include "stateidentifier.h"
#include "utility.h"
#include "support/filesystem.h"

using namespace std;
using namespace cv;
using namespace alpr;

// Given a directory full of pre-cropped images, identify the state that each image belongs to.
// This is used to sort our own positive image database as a first step before grabbing characters to use to train the OCR.

bool detectPlate( StateIdentifier* identifier, Mat frame);

int main( int argc, const char** argv )
{
  string inDir;
  string outDir;
  Mat frame;

  //Check if user specify image to process
  if(argc == 3 )
  {
    inDir = argv[1];
    outDir =  argv[2];
    outDir = outDir + "/";
  }
  else
  {
    printf("Use:\n\t%s directory \n",argv[0]);
    printf("Ex: \n\t%s ./pics/   \n",argv[0]);
    return 0;
  }

  Config config("us");
  StateIdentifier identifier(&config);

  if (DirectoryExists(outDir.c_str()) == false)
  {
    printf("Output dir does not exist\n");
    return 0;
  }

  if (DirectoryExists(inDir.c_str()))
  {
    vector<string> files = getFilesInDir(inDir.c_str());

    for (int i = 0; i< files.size(); i++)
    {
      if (hasEnding(files[i], ".png"))
      {
        string fullpath = inDir + "/" + files[i];
        cout << fullpath << endl;
        frame = imread( fullpath.c_str() );

	PipelineData pipeline_data(frame, Rect(0, 0, frame.cols, frame.rows), &config);
        identifier.recognize(&pipeline_data);

        if (pipeline_data.region_confidence <= 20)
        {
	  pipeline_data.region_code = "zz";
          pipeline_data.region_confidence = 100;
        }
        else
	{
          cout << pipeline_data.region_confidence << " : " << pipeline_data.region_code;

          ostringstream convert;   // stream used for the conversion
          convert << i;      // insert the textual representation of 'Number' in the characters in the stream

          string copyCommand = "cp \"" + fullpath + "\" " + outDir + pipeline_data.region_code + convert.str() + ".png";
          system( copyCommand.c_str() );
          waitKey(50);
          //while ((char) waitKey(50) != 'c') { }
        }
      }
    }
  }
}

bool detectPlate( StateIdentifier* identifier, Mat frame);
