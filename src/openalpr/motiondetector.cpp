#include "motiondetector.h"

using namespace cv;

namespace alpr
{
  
MotionDetector::MotionDetector()
{
	#if OPENCV_MAJOR_VERSION == 2
	pMOG2 = new BackgroundSubtractorMOG2();
	#else
	// OpenCV 3
	pMOG2 = createBackgroundSubtractorMOG2();
	#endif
}

MotionDetector::~MotionDetector()
{

}

void MotionDetector::ResetMotionDetection(cv::Mat* frame)
{
#if OPENCV_MAJOR_VERSION == 2
	pMOG2->operator()(*frame, fgMaskMOG2, 1);
#else
	// OpenCV 3
	pMOG2->apply(*frame, fgMaskMOG2, 1);
#endif
}

cv::Rect MotionDetector::MotionDetect(cv::Mat* frame)
//Detect motion and create ONE recangle that contains all the detected motion
{
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Rect bounding_rect;
	std::vector<cv::Rect> rects;
	cv::Rect largest_rect, rect_temp;

	// Detect motion

#if OPENCV_MAJOR_VERSION == 2
	pMOG2->operator()(*frame, fgMaskMOG2, -1);
#else
	// OpenCV 3
	pMOG2->apply(*frame, fgMaskMOG2);
#endif

	//Remove noise
	cv::erode(fgMaskMOG2, fgMaskMOG2, getStructuringElement(cv::MORPH_RECT, cv::Size(6, 6)));
	// Find the contours of motion areas in the image
	findContours(fgMaskMOG2, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	// Find the bounding rectangles of the areas of motion
	if (contours.size() > 0)
	{
		for (int i = 0; i < contours.size(); i++)
		{
			bounding_rect = boundingRect(contours[i]);
			rects.push_back(bounding_rect);
		}
		// Determine the overall area with motion.
		largest_rect = rects[0];
		for (int i = 1; i < rects.size(); i++)
		{
			rect_temp.x = min(largest_rect.x,rects[i].x);
			rect_temp.y = min(largest_rect.y,rects[i].y);
			rect_temp.width = max(largest_rect.x + largest_rect.width, rects[i].x + rects[i].width)-rect_temp.x;
			rect_temp.height = max(largest_rect.y + largest_rect.height, rects[i].y + rects[i].height) - rect_temp.y;
			largest_rect = rect_temp;
		}
		rectangle(*frame, rect_temp, cv::Scalar(0, 255, 0), 1, 8, 0);
	}
	else
        {
          largest_rect.x = 0;
          largest_rect.y = 0;
          largest_rect.width = 0;
          largest_rect.height = 0;
        }
		
//	imshow("Motion detect", fgMaskMOG2);
	return expandRect(largest_rect, 0, 0, frame->cols, frame->rows);
}

}