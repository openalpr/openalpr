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

#include "featurematcher.h"

using namespace cv;
using namespace std;

namespace alpr
{

  //const int DEFAULT_QUERY_FEATURES = 305;
  //const int DEFAULT_TRAINING_FEATURES = 305;
  const float MAX_DISTANCE_TO_MATCH = 100.0f;

  FeatureMatcher::FeatureMatcher()
  {
    //this->descriptorMatcher = DescriptorMatcher::create( "BruteForce-HammingLUT" );
    this->descriptorMatcher = new BFMatcher(NORM_HAMMING, false);

    //this->descriptorMatcher = DescriptorMatcher::create( "FlannBased" );
#if OPENCV_MAJOR_VERSION == 2
    this->detector = new FastFeatureDetector(10, true);
    this->extractor = new BRISK(10, 1, 0.9);
#else
    // OpenCV 3
    this->detector = FastFeatureDetector::create();
    this->extractor = BRISK::create(10, 1, 0.9);
#endif

  }

  FeatureMatcher::~FeatureMatcher()
  {
    for (unsigned int i = 0; i < trainingImgKeypoints.size(); i++)
      trainingImgKeypoints[i].clear();
    trainingImgKeypoints.clear();

    descriptorMatcher.release();
    detector.release();
    extractor.release();
  }

  bool FeatureMatcher::isLoaded()
  {
    if( detector.empty() || extractor.empty() || descriptorMatcher.empty()  )
    {
      return false;
    }

    return true;
  }

  int FeatureMatcher::numTrainingElements()
  {
    return billMapping.size();
  }

  void FeatureMatcher::surfStyleMatching( const Mat& queryDescriptors, vector<KeyPoint> queryKeypoints,
                                          vector<DMatch>& matches12 )
  {
    vector<vector<DMatch> > matchesKnn;

    this->descriptorMatcher->radiusMatch(queryDescriptors, matchesKnn, MAX_DISTANCE_TO_MATCH);

    vector<DMatch> tempMatches;
    _surfStyleMatching(queryDescriptors, matchesKnn, tempMatches);

    crisscrossFiltering(queryKeypoints, tempMatches, matches12);
  }

  void FeatureMatcher::_surfStyleMatching(const Mat& queryDescriptors, vector<vector<DMatch> > matchesKnn, vector<DMatch>& matches12)
  {
    //objectMatches.clear();
    //objectMatches.resize(objectIds.size());
    //cout << "starting matcher" << matchesKnn.size() << endl;
    for (int descInd = 0; descInd < queryDescriptors.rows; descInd++)
    {
      //const std::vector<DMatch> & matches = matchesKnn[descInd];
      //cout << "two: " << descInd << ":" << matches.size() << endl;

      // Check to make sure we have 2 matches.  I think this is always the case, but it doesn't hurt to be sure
      if (matchesKnn[descInd].size() > 1)
      {
        // Next throw out matches with a crappy score
        // Ignore... already handled by the radiusMatch
        //if (matchesKnn[descInd][0].distance < MAX_DISTANCE_TO_MATCH)
        //{
        float ratioThreshold = 0.75;

        // Check if both matches came from the same image.  If they both came from the same image, score them slightly less harshly
        if (matchesKnn[descInd][0].imgIdx == matchesKnn[descInd][1].imgIdx)
        {
          ratioThreshold = 0.85;
        }

        if ((matchesKnn[descInd][0].distance / matchesKnn[descInd][1].distance) < ratioThreshold)
        {
          bool already_exists = false;
          // Quickly run through the matches we've already added and make sure it's not a duplicate...
          for (unsigned int q = 0; q < matches12.size(); q++)
          {
            if (matchesKnn[descInd][0].queryIdx == matches12[q].queryIdx)
            {
              already_exists = true;
              break;
            }
            else if ((matchesKnn[descInd][0].trainIdx == matches12[q].trainIdx) &&
                     (matchesKnn[descInd][0].imgIdx == matches12[q].imgIdx))
            {
              already_exists = true;
              break;
            }
          }

          // Good match.
          if (already_exists == false)
            matches12.push_back(matchesKnn[descInd][0]);
        }

        //}
      }
      else if (matchesKnn[descInd].size() == 1)
      {
        // Only match?  Does this ever happen?
        matches12.push_back(matchesKnn[descInd][0]);
      }
      // In the ratio test, we will compare the quality of a match with the next match that is not from the same object:
      // we can accept several matches with similar scores as long as they are for the same object. Those should not be
      // part of the model anyway as they are not discriminative enough

      //for (unsigned int first_index = 0; first_index < matches.size(); ++first_index)
      //{
      //matches12.push_back(match);
      //}
    }
  }

  // Compares the matches keypoints for parallel lines.  Removes matches that are criss-crossing too much
  // We assume that license plates won't be upside-down or backwards.  So expect lines to be closely parallel
  void FeatureMatcher::crisscrossFiltering(const vector<KeyPoint> queryKeypoints, const vector<DMatch> inputMatches, vector<DMatch> &outputMatches)
  {
    Rect crissCrossAreaVertical(0, 0, w, h * 2);
    Rect crissCrossAreaHorizontal(0, 0, w * 2, h);

    for (unsigned int i = 0; i < billMapping.size(); i++)
    {
      vector<DMatch> matchesForOnePlate;
      for (unsigned int j = 0; j < inputMatches.size(); j++)
      {
        if (inputMatches[j].imgIdx == (int) i)
          matchesForOnePlate.push_back(inputMatches[j]);
      }

      // For each plate, compare the lines for the keypoints (training image and query image)
      // go through each line between keypoints and filter out matches that are criss-crossing
      vector<LineSegment> vlines;
      vector<LineSegment> hlines;
      vector<int> matchIdx;

      for (unsigned int j = 0; j < matchesForOnePlate.size(); j++)
      {
        KeyPoint tkp = trainingImgKeypoints[i][matchesForOnePlate[j].trainIdx];
        KeyPoint qkp = queryKeypoints[matchesForOnePlate[j].queryIdx];

        vlines.push_back(LineSegment(tkp.pt.x, tkp.pt.y + h, qkp.pt.x, qkp.pt.y));
        hlines.push_back(LineSegment(tkp.pt.x, tkp.pt.y, qkp.pt.x + w, qkp.pt.y));
        matchIdx.push_back(j);
      }

      // Iterate through each line (n^2) removing the one with the most criss-crosses until there are none left.
      int mostIntersections = 1;
      while (mostIntersections > 0 && vlines.size() > 0)
      {
        int mostIntersectionsIndex = -1;
        mostIntersections = 0;

        for (unsigned int j = 0; j < vlines.size(); j++)
        {
          int intrCount = 0;
          for (unsigned int q = 0; q < vlines.size(); q++)
          {
            Point vintr = vlines[j].intersection(vlines[q]);
            Point hintr = hlines[j].intersection(hlines[q]);
            float vangleDiff = abs(vlines[j].angle - vlines[q].angle);
            float hangleDiff = abs(hlines[j].angle - hlines[q].angle);
            if (vintr.inside(crissCrossAreaVertical) && vangleDiff > 10)
            {
              intrCount++;
            }
            else if (hintr.inside(crissCrossAreaHorizontal) && hangleDiff > 10)
            {
              intrCount++;
            }
          }

          if (intrCount > mostIntersections)
          {
            mostIntersections = intrCount;
            mostIntersectionsIndex = j;
          }
        }

        if (mostIntersectionsIndex >= 0)
        {
//          if (this->config->debugStateId)
//            cout << "Filtered intersection! " << billMapping[i] <<  endl;
          vlines.erase(vlines.begin() + mostIntersectionsIndex);
          hlines.erase(hlines.begin() + mostIntersectionsIndex);
          matchIdx.erase(matchIdx.begin() + mostIntersectionsIndex);
        }
      }

      // Push the non-crisscrosses back on the list
      for (unsigned int j = 0; j < matchIdx.size(); j++)
      {
        outputMatches.push_back(matchesForOnePlate[matchIdx[j]]);
      }
    }
  }

  // Returns true if successful, false otherwise
  bool FeatureMatcher::loadRecognitionSet(string directory, string country)
  {
    std::ostringstream out;
    out << directory << "/keypoints/" << country << "/";
    string country_dir = out.str();

    if (DirectoryExists(country_dir.c_str()))
    {
      vector<Mat> trainImages;
      vector<string> plateFiles = getFilesInDir(country_dir.c_str());

      for (unsigned int i = 0; i < plateFiles.size(); i++)
      {
        if (hasEnding(plateFiles[i], ".jpg") == false)
          continue;

        string fullpath = country_dir + plateFiles[i];
        Mat img = imread( fullpath );

        // convert to gray and resize to the size of the templates
        cvtColor(img, img, CV_BGR2GRAY);

        if( img.empty() )
        {
          cout << "Can not read images" << endl;
          return -1;
        }

        Mat descriptors;

        vector<KeyPoint> keypoints;
        detector->detect( img, keypoints );
        extractor->compute(img, keypoints, descriptors);

        if (descriptors.cols > 0)
        {
          billMapping.push_back(plateFiles[i].substr(0, 2));
          trainImages.push_back(descriptors);
          trainingImgKeypoints.push_back(keypoints);
        }
      }

      this->descriptorMatcher->add(trainImages);
      this->descriptorMatcher->train();

      return true;
    }

    return false;
  }

  RecognitionResult FeatureMatcher::recognize( const Mat& queryImg, bool drawOnImage, Mat* outputImage,
      bool debug_on, vector<int> debug_matches_array
                                             )
  {
    RecognitionResult result;

    this->w = queryImg.cols;
    this->h = queryImg.rows;

    result.haswinner = false;
    result.confidence = 0;

    Mat queryDescriptors;
    vector<KeyPoint> queryKeypoints;

    detector->detect( queryImg, queryKeypoints );
    extractor->compute(queryImg, queryKeypoints, queryDescriptors);

    if (queryKeypoints.size() <= 5)
    {
      // Cut it loose if there's less than 5 keypoints... nothing would ever match anyway and it could crash the matcher.
      if (drawOnImage)
      {
        drawKeypoints(  queryImg, queryKeypoints, *outputImage, CV_RGB(0, 255, 0), DrawMatchesFlags::DEFAULT );
      }
      return result;
    }

    vector<DMatch> filteredMatches;

    surfStyleMatching( queryDescriptors, queryKeypoints, filteredMatches );

    // Create and initialize the counts to 0
    std::vector<int> bill_match_counts( billMapping.size() );

    for (unsigned int i = 0; i < billMapping.size(); i++)
    {
      bill_match_counts[i] = 0;
    }

    for (unsigned int i = 0; i < filteredMatches.size(); i++)
    {
      bill_match_counts[filteredMatches[i].imgIdx]++;
      //if (filteredMatches[i].imgIdx
    }

    float max_count = 0;	// represented as a percent (0 to 100)
    int secondmost_count = 0;
    int maxcount_index = -1;
    for (unsigned int i = 0; i < billMapping.size(); i++)
    {
      if (bill_match_counts[i] > max_count && bill_match_counts[i] >= 4)
      {
        secondmost_count = max_count;
        if (secondmost_count <= 2) 	// A value of 1 or 2 is effectively 0
          secondmost_count = 0;

        max_count = bill_match_counts[i];
        maxcount_index = i;
      }
    }

    float score = ((max_count - secondmost_count - 3) / 10) * 100;
    if (score < 0)
      score = 0;
    else if (score > 100)
      score = 100;

    if (score > 0)
    {
      result.haswinner = true;
      result.winner = billMapping[maxcount_index];
      result.confidence = score;

      if (drawOnImage)
      {
        vector<KeyPoint> positiveMatches;
        for (unsigned int i = 0; i < filteredMatches.size(); i++)
        {
          if (filteredMatches[i].imgIdx == maxcount_index)
          {
            positiveMatches.push_back( queryKeypoints[filteredMatches[i].queryIdx] );
          }
        }

        Mat tmpImg;
        drawKeypoints(  queryImg, queryKeypoints, tmpImg, CV_RGB(185, 0, 0), DrawMatchesFlags::DEFAULT );
        drawKeypoints(  tmpImg, positiveMatches, *outputImage, CV_RGB(0, 255, 0), DrawMatchesFlags::DEFAULT );

        if (result.haswinner)
        {
          std::ostringstream out;
          out << result.winner << " (" << result.confidence << "%)";

          // we detected a bill, let the people know!
          //putText(*outputImage, out.str(), Point(15, 27), FONT_HERSHEY_DUPLEX, 1.1, CV_RGB(0, 0, 0), 2);
        }
      }
    }

//    if (this->config->debugStateId)
//    {
//      for (unsigned int i = 0; i < billMapping.size(); i++)
//      {
//        cout << billMapping[i] << " : " << bill_match_counts[i] << endl;
//      }
//    }

    return result;
  }
  
}