#ifndef OPENALPR_VIDEOBUFFER_H
#define OPENALPR_VIDEOBUFFER_H

#include <cstdio>

#include "opencv2/highgui/highgui.hpp"

#include "support/filesystem.h"
#include "support/tinythread.h"



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
    
    
    int getLatestFrame(cv::Mat* frame)
    {
      if (latestFrameNumber == lastFrameRead)
	return -1;
      
      tthread::lock_guard<tthread::mutex> guard(mMutex);
      
      frame->create(latestFrame->size(), latestFrame->type());
      latestFrame->copyTo(*frame);
      
      this->lastFrameRead = this->latestFrameNumber;
      
      return this->lastFrameRead;
    }
    
    void setLatestFrame(cv::Mat* frame)
    {      
      this->latestFrame = frame;
      
      this->latestFrameNumber++;
    }
    
    bool active;
    
    int latestFrameNumber;
    int lastFrameRead;
    
    std::string mjpeg_url;
    int fps;
    tthread::mutex mMutex;
    
  private:
    cv::Mat* latestFrame;

};

class VideoBuffer
{

  public:
    VideoBuffer();
    virtual ~VideoBuffer();

    void connect(std::string mjpeg_url, int fps);

    // If a new frame is available, the function sets "frame" to it and returns the frame number
    // If no frames are available, or the latest has already been grabbed, returns -1.
    int getLatestFrame(cv::Mat* frame);

    void disconnect();
    
  private:
    
    
    VideoDispatcher* dispatcher;
};




#endif // OPENALPR_VIDEOBUFFER_H