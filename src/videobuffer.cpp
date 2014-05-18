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

#include "videobuffer.h"


void imageCollectionThread(void* arg);


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


void VideoBuffer::connect(std::string mjpeg_url, int fps)
{
  
    if (hasEnding(mjpeg_url, ".mjpg") == false)
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
    
    dispatcher = new VideoDispatcher(mjpeg_url, fps);
      
    tthread::thread* t = new tthread::thread(imageCollectionThread, (void*) dispatcher);
    
}

int VideoBuffer::getLatestFrame(cv::Mat* frame)
{
  if (dispatcher == NULL)
    return -1;
  
  return dispatcher->getLatestFrame(frame);
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

  cv::VideoCapture cap=cv::VideoCapture();
  cap.open(dispatcher->mjpeg_url);
  
  cv::Mat frame;
  
  while (dispatcher->active)
  {
    while (dispatcher->active)
    {
      
      dispatcher->mMutex.lock();
      bool hasImage = cap.read(frame);
      dispatcher->setLatestFrame(&frame);
      dispatcher->mMutex.unlock();
      
      if (hasImage == false)
	break;
      

      // Delay 15ms
      cv::waitKey(15);      
    }
    
    // Delay 100ms
    cv::waitKey(100);
  }
}