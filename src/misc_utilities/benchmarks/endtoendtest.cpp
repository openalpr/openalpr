#include "endtoendtest.h"

using namespace std;
using namespace cv;
using namespace alpr;


EndToEndTest::EndToEndTest(string inputDir, string outputDir)
{
  this->inputDir = inputDir;
  this->outputDir = outputDir;
}


void EndToEndTest::runTest(string country, vector<std::string> files)
{

  
  
  AlprImpl alpr(country);
  alpr.config->setDebug(false);
  alpr.setDetectRegion(false);

  vector<EndToEndBenchmarkResult> benchmarkResults;

  vector<string> textFiles = filterByExtension(files, ".txt");

  for (int i = 0; i< textFiles.size(); i++)
  {
    cout << "Benchmarking file " << (i + 1) << " / " << textFiles.size() << " -- " << textFiles[i] << endl;
    EndToEndBenchmarkResult benchmarkResult;
	
    string fulltextpath = inputDir + "/" + textFiles[i];
    
    ifstream inputFile(fulltextpath.c_str());
    string line;
    
    getline(inputFile, line);
    
    istringstream ss(line);

    string imgfile, plate_number;
    int x, y, w, h;

    ss >> imgfile >> x >> y >> w >> h >> plate_number;
    
    string fullimgpath = inputDir + "/" + imgfile;
    
    benchmarkResult.imageName = imgfile;
    
    Mat frame = imread( fullimgpath.c_str() );

    Rect actualPlateRect(x, y, w, h);
    
    vector<Rect> rois;
    rois.push_back(Rect(0,0,frame.cols,frame.rows));
    AlprFullDetails recognitionDetails = alpr.recognizeFullDetails(frame, rois);
    

    
    //cv::circle(frame, centerPoint, 2, Scalar(0, 0, 255), 5);
    //drawAndWait(&frame);
    
    benchmarkResult.detectionFalsePositives = 0;
    
    for (int z = 0; z < recognitionDetails.plateRegions.size(); z++)
    {
      benchmarkResult.detectionFalsePositives += totalRectCount(recognitionDetails.plateRegions[z]);
      
      bool rectmatches = rectMatches(actualPlateRect, recognitionDetails.plateRegions[z]);
      
      
      if (rectmatches)
      {
	// This region matches our plate_number
	benchmarkResult.detectedPlate = true;
	benchmarkResult.detectionFalsePositives--;
	break;
      }
      
    }
    
    benchmarkResult.resultsFalsePositives = recognitionDetails.results.plates.size();
    
    // Determine if the top result and the top N results match the correct value
    for (int z = 0; z < recognitionDetails.results.plates.size(); z++)
    {
      //cout << "Actual: " << plate_number << endl;
      //cout << "Candidate: " << recognitionDetails.results[z].bestPlate.characters << endl;
      if (recognitionDetails.results.plates[z].bestPlate.characters == plate_number)
      {
	benchmarkResult.topResultCorrect = true;
	benchmarkResult.top10ResultCorrect = true;
	benchmarkResult.resultsFalsePositives--;
	break;
      }
      
      for (int idx = 0; idx < recognitionDetails.results.plates[z].topNPlates.size(); idx++)
      {
	if (recognitionDetails.results.plates[z].topNPlates[idx].characters == plate_number)
	{
	  benchmarkResult.top10ResultCorrect = true;
	  benchmarkResult.resultsFalsePositives--;
	  break;
	}
      }
      
      if (benchmarkResult.top10ResultCorrect)
	break;
    }
    
    benchmarkResults.push_back(benchmarkResult);
    
  
  }

  // Print results data
  
  int image_name_padding = 0;
  for (int i = 0; i < benchmarkResults.size(); i++)
  {
    EndToEndBenchmarkResult br = benchmarkResults[i];
    if (br.imageName.length() > image_name_padding)
      image_name_padding = br.imageName.length();
  }
  image_name_padding += 4;
  
  ofstream data;
  string outputResultsFile = outputDir + "/results.txt";
  data.open(outputResultsFile.c_str());
  
  
  data << setfill(' ') << setw(image_name_padding) << "Image name" << setw(20) << "Detected Plate" << setw(20) << "# False Detections" << setw(20) << "Top Result Correct" << setw(20) << "Top 10 Correct" << setw(20) << "# False Results" << endl;
  for (int i = 0; i < benchmarkResults.size(); i++)
  {
    EndToEndBenchmarkResult br = benchmarkResults[i];
    data << setfill(' ') << setw(image_name_padding) << br.imageName << setw(20) << br.detectedPlate << setw(20) << br.detectionFalsePositives << setw(20) << br.topResultCorrect << setw(20) << br.top10ResultCorrect << setw(20) << br.resultsFalsePositives << endl;
  }
  data.close();
  
  // Print summary data
  
  
  int totalDetections = 0;
  int totalTopResultCorrect = 0;
  int totalTop10Correct = 0;
  int falseDetectionPositives = 0;
  int falseResults = 0;
  for (int i = 0; i < benchmarkResults.size(); i++)
  {
    if (benchmarkResults[i].detectedPlate) totalDetections++;
    if (benchmarkResults[i].topResultCorrect) totalTopResultCorrect++;
    if (benchmarkResults[i].top10ResultCorrect) totalTop10Correct++;
    
    falseDetectionPositives += benchmarkResults[i].detectionFalsePositives;
    falseResults += benchmarkResults[i].resultsFalsePositives;
  }
  
  // Percentage of how many are correct (higher is better)
  float detectionScore = 100.0 * ((float) totalDetections) / ((float) benchmarkResults.size());
  float topResultScore = 100.0 * ((float) totalTopResultCorrect) / ((float) benchmarkResults.size());
  float top10ResultScore = 100.0 * ((float) totalTop10Correct) / ((float) benchmarkResults.size());
  
  // How many false positives per image (higher is worse)
  float falseDetectionPositivesScore = ((float) falseDetectionPositives) / ((float) benchmarkResults.size());
  float falseResultsScore = ((float) falseResults) / ((float) benchmarkResults.size());
  
  string outputSummaryFile = outputDir + "/summary.txt";
  data.open(outputSummaryFile.c_str());
  
  data << "-------------------" << endl;
  data << "|     SUMMARY     |" << endl;
  data << "-------------------" << endl;
  data << endl;
  data << "Accuracy scores (higher is better)" << endl;
  data << "Percent of plates DETECTED: " << detectionScore << endl;
  data << "Percent of correct TOP10:   " << top10ResultScore << endl;
  data << "Percent of correct MATCHES: " << topResultScore << endl;
  data << endl;
  data << "False Positives Score (lower is better)" << endl;
  data << "False DETECTIONS per image: " << falseDetectionPositivesScore << endl;
  data << "False RESULTS per image:    " << falseResultsScore << endl;
  
  data.close();
  
  
  // Print the contents of these files now:
  string line;
  ifstream resultsFileIn(outputResultsFile.c_str());
  while(getline(resultsFileIn, line))
  {
    cout << line << endl;
  }
  ifstream summaryFileIn(outputSummaryFile.c_str());
  while(getline(summaryFileIn, line))
  {
    cout << line << endl;
  }
}


bool EndToEndTest::rectMatches(cv::Rect actualPlate, PlateRegion candidate)
{
  // Determine if this region matches our plate in the image
  // Do this simply by verifying that the center point of the plate is within the region
  // And that the plate region is not x% larger or smaller

  const float MAX_SIZE_PERCENT_LARGER = 0.65;

  //int plateCenterX = actualPlate.x + (int) (((float) actualPlate.width) / 2.0);
  //int plateCenterY = actualPlate.y + (int) (((float) actualPlate.height) / 2.0);
  //Point centerPoint(plateCenterX, plateCenterY);
  
  vector<Point> requiredPoints;
  requiredPoints.push_back(Point( actualPlate.x + (int) (((float) actualPlate.width) * 0.2),
				   actualPlate.y + (int) (((float) actualPlate.height) * 0.15)
			   ));
  requiredPoints.push_back(Point( actualPlate.x + (int) (((float) actualPlate.width) * 0.8),
				   actualPlate.y + (int) (((float) actualPlate.height) * 0.15)
			  ));
  requiredPoints.push_back(Point( actualPlate.x + (int) (((float) actualPlate.width) * 0.2),
				  actualPlate.y + (int) (((float) actualPlate.height) * 0.85)
			  ));
  requiredPoints.push_back(Point( actualPlate.x + (int) (((float) actualPlate.width) * 0.8),
				actualPlate.y + (int) (((float) actualPlate.height) * 0.85)
			));
  

  float sizeDiff = 1.0 - ((float) actualPlate.area()) / ((float) candidate.rect.area());

  //cout << "Candidate: " << candidate.rect.x << "," << candidate.rect.y << " " << candidate.rect.width << "-" << candidate.rect.height << endl;
  //cout << "Actual:    " << actualPlate.x << "," << actualPlate.y << " " << actualPlate.width << "-" << actualPlate.height << endl;
  
  //cout << "size diff: " << sizeDiff << endl;
  
  bool hasAllPoints = true;
  for (int z = 0; z < requiredPoints.size(); z++)
  {
    if (candidate.rect.contains(requiredPoints[z]) == false)
      hasAllPoints = false;
    break;
  }
  if ( hasAllPoints && 
    (sizeDiff < MAX_SIZE_PERCENT_LARGER) )

  {
    return true;
  }
  else
  {
    for (int i = 0; i < candidate.children.size(); i++)
    {
      if (rectMatches(actualPlate, candidate.children[i]))
	return true;
    }
  }
  
  return false;
}

int EndToEndTest::totalRectCount(PlateRegion rootCandidate)
{
  int childCount = 0;
  for (int i = 0; i < rootCandidate.children.size(); i++)
  {
    childCount += totalRectCount(rootCandidate.children[i]);
  }
  
  return childCount + 1;
}
