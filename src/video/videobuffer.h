#ifndef OPENALPR_VIDEOBUFFER_H
#define OPENALPR_VIDEOBUFFER_H

#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <vector>

#include "opencv2/highgui/highgui.hpp"

#include "support/filesystem.h"
#include "support/tinythread.h"
#include "support/platform.h"



class VideoDispatcher
{
  public:
    VideoDispatcher(std::string mjpeg_url, int fps)
    {
      this->active = true;
      this->latestFrameNumber = -1;
      this->lastFrameRead = -1;
      this->fps = fps;
      this->mjpeg_url = mjpeg_url;
    }
    
    
    int getLatestFrame(cv::Mat* frame, std::vector<cv::Rect>& regionsOfInterest)
    {
      tthread::lock_guard<tthread::mutex> guard(mMutex);
      
      if (latestFrameNumber == lastFrameRead)
        return -1;
      
      frame->create(latestFrame.size(), latestFrame.type());
      latestFrame.copyTo(*frame);
      
      this->lastFrameRead = this->latestFrameNumber;
      
      // Copy the regionsOfInterest array
      for (int i = 0; i < this->latestRegionsOfInterest.size(); i++)
          regionsOfInterest.push_back(this->latestRegionsOfInterest[i]);
      
      return this->lastFrameRead;
    }
    
    void setLatestFrame(cv::Mat frame)
    {      
      frame.copyTo(this->latestFrame);
      this->latestRegionsOfInterest = calculateRegionsOfInterest(&this->latestFrame);
      
      this->latestFrameNumber++;
    }
    
    virtual void log_info(std::string message)
    {
      std::cout << message << std::endl;
    }
    virtual void log_error(std::string error)
    {
      std::cerr << error << std::endl;
    }
    
    std::vector<cv::Rect> calculateRegionsOfInterest(cv::Mat* frame)
    {
        cv::Rect rect(0, 0, frame->cols, frame->rows);
        
        std::vector<cv::Rect> rois;
        rois.push_back(rect);
        
        return rois;
    }
    
    bool active;
    
    int latestFrameNumber;
    int lastFrameRead;
    
    std::string mjpeg_url;
    int fps;
    tthread::mutex mMutex;
    
  private:
    cv::Mat latestFrame;
    std::vector<cv::Rect> latestRegionsOfInterest;
};

class VideoBuffer
{

  public:
    VideoBuffer();
    virtual ~VideoBuffer();

    void connect(std::string mjpeg_url, int fps);
    

    // If a new frame is available, the function sets "frame" to it and returns the frame number
    // If no frames are available, or the latest has already been grabbed, returns -1.
    // regionsOfInterest is set to a list of good regions to check for license plates.  Default is one rectangle for the entire frame.
    int getLatestFrame(cv::Mat* frame, std::vector<cv::Rect>& regionsOfInterest);

    void disconnect();
    
  protected:
  
    virtual VideoDispatcher* createDispatcher(std::string mjpeg_url, int fps);
    
  private:
    
    
    VideoDispatcher* dispatcher;
};




#endif // OPENALPR_VIDEOBUFFER_H