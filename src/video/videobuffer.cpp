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

#include "videobuffer.h"

using namespace alpr;

void imageCollectionThread(void* arg);
void getALPRImages(cv::VideoCapture cap, VideoDispatcher* dispatcher);


VideoBuffer::VideoBuffer()
{
  dispatcher = NULL;
  
}

VideoBuffer::~VideoBuffer()
{
  if (dispatcher != NULL)
  {
    dispatcher->active = false;
  }
}

VideoDispatcher* VideoBuffer::createDispatcher(std::string mjpeg_url, int fps)
{
  return new VideoDispatcher(mjpeg_url, fps);
}

void VideoBuffer::connect(std::string mjpeg_url, int fps)
{
  
    if (startsWith(mjpeg_url, "http") && hasEnding(mjpeg_url, ".mjpg") == false)
    {
      // The filename doesn't end with ".mjpg" so the downstream processing may not treat it as such
      // OpenCV doesn't have a way to force the rendering, other than via URL path.  So, let's add it to the URL
      
     
      std::size_t found = mjpeg_url.find("?");
      if (found!=std::string::npos)
      {
	// URL already contains a "?"
	mjpeg_url = mjpeg_url + "&openalprfiletype=file.mjpg";
      }
      else
      {
	// URL does not contain a "?"
	mjpeg_url = mjpeg_url + "?openalprfiletype=file.mjpg";
      }

    }
    
    dispatcher = createDispatcher(mjpeg_url, fps);
      
    tthread::thread* t = new tthread::thread(imageCollectionThread, (void*) dispatcher);
    
}

int VideoBuffer::getLatestFrame(cv::Mat* frame, std::vector<cv::Rect>& regionsOfInterest)
{
  if (dispatcher == NULL)
    return -1;
  
  return dispatcher->getLatestFrame(frame, regionsOfInterest);
}


void VideoBuffer::disconnect()
{
  if (dispatcher != NULL)
  {
    dispatcher->active = false;
  }
  
  dispatcher = NULL;
}





void imageCollectionThread(void* arg)
{
  
  VideoDispatcher* dispatcher = (VideoDispatcher*) arg;

  while (dispatcher->active)
  {
    try
    {
      cv::VideoCapture cap=cv::VideoCapture();
      dispatcher->log_info("Video stream connecting...");

      // Check if it's a webcam, if so, pass the device ID
      std::string video_prefix = "/dev/video";
      if (startsWith(dispatcher->mjpeg_url, video_prefix))
      {
        std::string device_number_str = dispatcher->mjpeg_url.substr(video_prefix.length());
        dispatcher->log_info("Opening webcam video device " + device_number_str);
        
        int webcam_number = atoi(device_number_str.c_str());
        cap.open(webcam_number);
      }
      else if (dispatcher->mjpeg_url == "webcam")
      {
        cap.open(0);
      }
      else
      {
        cap.open(dispatcher->mjpeg_url);
      }
      
      if (cap.isOpened())
      {
        dispatcher->log_info("Video stream connected");
        getALPRImages(cap, dispatcher);
      }
      else
      {
	std::stringstream ss;
	ss << "Stream " << dispatcher->mjpeg_url << " failed to open.";
	dispatcher->log_error(ss.str());
      }

    }
    catch (const std::runtime_error& error)
    {
      // Error occured while trying to gather image.  Retry, don't exit.
      std::stringstream ss;
      ss << "VideoBuffer exception: " << error.what();
      dispatcher->log_error( ss.str() );
    }
    catch (cv::Exception e)
    {
      // OpenCV Exception occured while trying to gather image.  Retry, don't exit.
      std::stringstream ss;
      ss << "VideoBuffer OpenCV exception: " << e.what();
      dispatcher->log_error( ss.str() );
    }
    
    // Delay 1 second
    sleep_ms(1000);
    
  }

  
}


// Continuously grabs images from the video capture.  If there is an error,
// it returns so that the video capture can be recreated.
void getALPRImages(cv::VideoCapture cap, VideoDispatcher* dispatcher)
{

  while (dispatcher->active)
  {
    while (dispatcher->active)
    {
      
      bool hasImage = false;
      try
      {
        cv::Mat frame;
	hasImage = cap.read(frame);
		  // Double check the image to make sure it's valid.
	if (!frame.data || frame.empty())
	{
	  std::stringstream ss;
	  ss << "Stream " << dispatcher->mjpeg_url << " received invalid frame";
	  dispatcher->log_error(ss.str());
	  return;
	}
	
	dispatcher->mMutex.lock();
	dispatcher->setLatestFrame(frame);
	dispatcher->mMutex.unlock();
      }
      catch (const std::runtime_error& error)
      {
	// Error occured while trying to gather image.  Retry, don't exit.
	std::stringstream ss;
	ss << "Exception happened " <<  error.what();
	dispatcher->log_error(ss.str());
	dispatcher->mMutex.unlock();
	return;
      }

      
      dispatcher->mMutex.unlock();
      
      if (hasImage == false)
	break;
      

      // Delay 15ms
      sleep_ms(15);    
    }
    
    // Delay 100ms
    sleep_ms(100);
  }
}
