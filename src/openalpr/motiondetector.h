#include "opencv2/opencv.hpp"

class MotionDetector
{
	private: cv::Ptr<cv::BackgroundSubtractor> pMOG2; //MOG2 Background subtractor
	private: cv::Mat fgMaskMOG2;
	public:
		MotionDetector();
		virtual ~MotionDetector();

		void ResetMotionDetection(cv::Mat* frame);
		cv::Rect MotionDetect(cv::Mat* frame);
};
