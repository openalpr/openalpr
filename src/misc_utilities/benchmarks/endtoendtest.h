#ifndef OPENALPR_ENDTOENDTEST_H
#define OPENALPR_ENDTOENDTEST_H


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "alpr_impl.h"
#include "benchmark_utils.h"

class EndToEndTest
{
  public:
    EndToEndTest(std::string inputDir, std::string outputDir);
    void runTest(std::string country, std::vector<std::string> files);
  
  private:
    
    bool rectMatches(cv::Rect actualPlate, alpr::PlateRegion candidate);
    int totalRectCount(alpr::PlateRegion rootCandidate);
	
    std::string inputDir;
    std::string outputDir;
  
  
};

class EndToEndBenchmarkResult {
  public:
    EndToEndBenchmarkResult()
    {
    this->imageName = "";
    this->detectedPlate = false;
    this->topResultCorrect = false;
    this->top10ResultCorrect = false;
    this->detectionFalsePositives = 0;
    this->resultsFalsePositives = 0;
    }
    
    std::string imageName;
    bool detectedPlate;
    bool topResultCorrect;
    bool top10ResultCorrect;
    int detectionFalsePositives;
    int resultsFalsePositives;
};

#endif	//OPENALPR_ENDTOENDTEST_H