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

#include <cstdio>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <signal.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "tclap/CmdLine.h"
#include "support/filesystem.h"
#include "support/timing.h"
#include "videobuffer.h"
#include "alpr.h"

const std::string MAIN_WINDOW_NAME = "ALPR main window";

const bool SAVE_LAST_VIDEO_STILL = false;
const std::string LAST_VIDEO_STILL_LOCATION = "/tmp/laststill.jpg";

/** Function Headers */
bool detectandshow(Alpr* alpr, cv::Mat frame, std::string region, bool writeJson);
void sighandler(int sig);

bool measureProcessingTime = false;

// This boolean is set to false when the user hits terminates (e.g., CTRL+C )
// so we can end infinite loops for things like video processing.
bool program_active = true;

int main( int argc, const char** argv )
{
  std::string filename;
  std::string configFile = "";
  bool outputJson = false;
  int seektoms = 0;
  bool detectRegion = false;
  std::string templateRegion;
  std::string country;
  int topn;

  TCLAP::CmdLine cmd("OpenAlpr Command Line Utility", ' ', Alpr::getVersion());

  TCLAP::UnlabeledValueArg<std::string>  fileArg( "image_file", "Image containing license plates", false, "", "image_file_path"  );

  
  TCLAP::ValueArg<std::string> countryCodeArg("c","country","Country code to identify (either us for USA or eu for Europe).  Default=us",false, "us" ,"country_code");
  TCLAP::ValueArg<int> seekToMsArg("","seek","Seek to the specied millisecond in a video file. Default=0",false, 0 ,"integer_ms");
  TCLAP::ValueArg<std::string> configFileArg("","config","Path to the openalpr.conf file",false, "" ,"config_file");
  TCLAP::ValueArg<std::string> templateRegionArg("t","template_region","Attempt to match the plate number against a region template (e.g., md for Maryland, ca for California)",false, "" ,"region code");
  TCLAP::ValueArg<int> topNArg("n","topn","Max number of possible plate numbers to return.  Default=10",false, 10 ,"topN");

  TCLAP::SwitchArg jsonSwitch("j","json","Output recognition results in JSON format.  Default=off", cmd, false);
  TCLAP::SwitchArg detectRegionSwitch("d","detect_region","Attempt to detect the region of the plate image.  Default=off", cmd, false);
  TCLAP::SwitchArg clockSwitch("","clock","Measure/print the total time to process image and all plates.  Default=off", cmd, false);

  try
  {
    cmd.add( templateRegionArg );
    cmd.add( seekToMsArg );
    cmd.add( topNArg );
    cmd.add( configFileArg );
    cmd.add( fileArg );
    cmd.add( countryCodeArg );

    
    if (cmd.parse( argc, argv ) == false)
    {
      // Error occured while parsing.  Exit now.
      return 1;
    }

    filename = fileArg.getValue();

    country = countryCodeArg.getValue();
    seektoms = seekToMsArg.getValue();
    outputJson = jsonSwitch.getValue();
    configFile = configFileArg.getValue();
    detectRegion = detectRegionSwitch.getValue();
    templateRegion = templateRegionArg.getValue();
    topn = topNArg.getValue();
    measureProcessingTime = clockSwitch.getValue();
  }
  catch (TCLAP::ArgException &e)    // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }

  struct sigaction sigIntHandler;
  
  sigIntHandler.sa_handler = sighandler;
  sigemptyset(&sigIntHandler.sa_mask);
  sigIntHandler.sa_flags = 0;
  
  sigaction(SIGHUP, &sigIntHandler, NULL);
  sigaction(SIGINT, &sigIntHandler, NULL);
  sigaction(SIGQUIT, &sigIntHandler, NULL);
  sigaction(SIGKILL, &sigIntHandler, NULL);
  sigaction(SIGTERM, &sigIntHandler, NULL);
  //sigaction(SIGABRT, &sigIntHandler, NULL);
  
  cv::Mat frame;

  Alpr alpr(country, configFile);
  alpr.setTopN(topn);

  if (detectRegion)
    alpr.setDetectRegion(detectRegion);

  if (templateRegion.empty() == false)
    alpr.setDefaultRegion(templateRegion);

  if (alpr.isLoaded() == false)
  {
    std::cerr << "Error loading OpenALPR" << std::endl;
    return 1;
  }

  if (filename.empty())
  {
    std::string filename;
    while (std::getline(std::cin, filename))
    {
      if (fileExists(filename.c_str()))
      {
	frame = cv::imread( filename );
	detectandshow( &alpr, frame, "", outputJson);
      }
      else
      {
	std::cerr << "Image file not found: " << filename << std::endl;
      }

    }
  }
  else if (filename == "webcam")
  {
    int framenum = 0;
    cv::VideoCapture cap(0);
    if (!cap.isOpened())
    {
      std::cout << "Error opening webcam" << std::endl;
      return 1;
    }

    while (cap.read(frame))
    {
      detectandshow(&alpr, frame, "", outputJson);
      cv::waitKey(1);
      framenum++;
    }
  }
  else if (startsWith(filename, "http://") || startsWith(filename, "https://"))
  {
    int framenum = 0;
    
    VideoBuffer videoBuffer;
    
    videoBuffer.connect(filename, 5);
    
    cv::Mat latestFrame;
    
    while (program_active)
    {
      int response = videoBuffer.getLatestFrame(&latestFrame);
      
      if (response != -1)
      {
        detectandshow( &alpr, latestFrame, "", outputJson);
      }
      
      cv::waitKey(10);
    }
    
    videoBuffer.disconnect();
    
    std::cout << "Video processing ended" << std::endl;
  }
  else if (hasEnding(filename, ".avi") || hasEnding(filename, ".mp4") || hasEnding(filename, ".webm") || hasEnding(filename, ".flv"))
  {
    if (fileExists(filename.c_str()))
    {
      int framenum = 0;

      cv::VideoCapture cap=cv::VideoCapture();
      cap.open(filename);
      cap.set(CV_CAP_PROP_POS_MSEC, seektoms);

      while (cap.read(frame))
      {
        if (SAVE_LAST_VIDEO_STILL)
        {
          cv::imwrite(LAST_VIDEO_STILL_LOCATION, frame);
        }
        std::cout << "Frame: " << framenum << std::endl;

        detectandshow( &alpr, frame, "", outputJson);
        //create a 1ms delay
        cv::waitKey(1);
        framenum++;
      }
    }
    else
    {
      std::cerr << "Video file not found: " << filename << std::endl;
    }
  }
  else if (hasEnding(filename, ".png") || hasEnding(filename, ".jpg") || hasEnding(filename, ".gif"))
  {
    if (fileExists(filename.c_str()))
    {
      frame = cv::imread( filename );

      detectandshow( &alpr, frame, "", outputJson);
    }
    else
    {
      std::cerr << "Image file not found: " << filename << std::endl;
    }
  }
  else if (DirectoryExists(filename.c_str()))
  {
    std::vector<std::string> files = getFilesInDir(filename.c_str());

    std::sort( files.begin(), files.end(), stringCompare );

    for (int i = 0; i< files.size(); i++)
    {
      if (hasEnding(files[i], ".jpg") || hasEnding(files[i], ".png"))
      {
        std::string fullpath = filename + "/" + files[i];
        std::cout << fullpath << std::endl;
        frame = cv::imread( fullpath.c_str() );
        if (detectandshow( &alpr, frame, "", outputJson))
        {
          //while ((char) cv::waitKey(50) != 'c') { }
        }
        else
        {
          //cv::waitKey(50);
        }
      }
    }
  }
  else
  {
    std::cerr << "Unknown file type" << std::endl;
    return 1;
  }

  return 0;
}


void sighandler(int sig)
{
  program_active = false;
  //std::cout << "Sig handler caught " << sig << std::endl;
}


bool detectandshow( Alpr* alpr, cv::Mat frame, std::string region, bool writeJson)
{
  std::vector<uchar> buffer;
  cv::imencode(".bmp", frame, buffer );

  timespec startTime;
  getTime(&startTime);

  std::vector<AlprResult> results = alpr->recognize(buffer);

  if (writeJson)
  {
    std::cout << alpr->toJson(results) << std::endl;
  }
  else
  {
    for (int i = 0; i < results.size(); i++)
    {
      std::cout << "plate" << i << ": " << results[i].result_count << " results -- Processing Time = " << results[i].processing_time_ms << "ms." << std::endl;

      for (int k = 0; k < results[i].topNPlates.size(); k++)
      {
        std::cout << "    - " << results[i].topNPlates[k].characters << "\t confidence: " << results[i].topNPlates[k].overall_confidence << "\t template_match: " << results[i].topNPlates[k].matches_template << std::endl;
      }
    }
  }

  timespec endTime;
  getTime(&endTime);
  if (measureProcessingTime)
    std::cout << "Total Time to process image: " << diffclock(startTime, endTime) << "ms." << std::endl;

  return results.size() > 0;
}
