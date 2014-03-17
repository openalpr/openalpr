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

#include "platelines.h"


PlateLines::PlateLines(Config* config)
{
  this->config = config;
  this->debug = config->debugPlateLines;

  if (debug)
    cout << "PlateLines constructor" << endl;


}

PlateLines::~PlateLines()
{

}



void PlateLines::processImage(Mat inputImage, float sensitivity)
{
  if (this->debug)
    cout << "PlateLines findLines" << endl;


  timespec startTime;
  getTime(&startTime);


  Mat smoothed(inputImage.size(), inputImage.type());
  inputImage.copyTo(smoothed);
    int morph_elem  = 2;
    int morph_size = 2;
  Mat element = getStructuringElement( morph_elem, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );

  morphologyEx( smoothed, smoothed, MORPH_CLOSE, element );

  morph_size = 1;
  element = getStructuringElement( morph_elem, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );

  //morphologyEx( thresholded, thresholded, MORPH_GRADIENT, element );

  morph_size = 1;
  element = getStructuringElement( morph_elem, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
  morphologyEx( smoothed, smoothed, MORPH_OPEN, element );



  Mat edges(inputImage.size(), inputImage.type());
  Canny(smoothed, edges, 66, 133);


  vector<LineSegment> hlines = this->getLines(edges, sensitivity, false);
  vector<LineSegment> vlines = this->getLines(edges, sensitivity, true);
  for (int i = 0; i < hlines.size(); i++)
    this->horizontalLines.push_back(hlines[i]);
  for (int i = 0; i < vlines.size(); i++)
    this->verticalLines.push_back(vlines[i]);




  // if debug is enabled, draw the image
  if (this->debug)
  {
    Mat debugImgHoriz(edges.size(), edges.type());
    Mat debugImgVert(edges.size(), edges.type());
    edges.copyTo(debugImgHoriz);
    edges.copyTo(debugImgVert);
    cvtColor(debugImgHoriz,debugImgHoriz,CV_GRAY2BGR);
    cvtColor(debugImgVert,debugImgVert,CV_GRAY2BGR);

    for( size_t i = 0; i < this->horizontalLines.size(); i++ )
    {
      line( debugImgHoriz, this->horizontalLines[i].p1, this->horizontalLines[i].p2, Scalar(0,0,255), 1, CV_AA);
    }

    for( size_t i = 0; i < this->verticalLines.size(); i++ )
    {
      line( debugImgVert, this->verticalLines[i].p1, this->verticalLines[i].p2, Scalar(0,0,255), 1, CV_AA);
    }

    vector<Mat> images;
    images.push_back(debugImgHoriz);
    images.push_back(debugImgVert);

    Mat dashboard = drawImageDashboard(images, debugImgVert.type(), 1);
    displayImage(config, "Hough Lines", dashboard);
  }



  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "Plate Lines Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }
  //smoothed.release();


  //////////////// METHOD2!!!!!!!////////////////////

  /*
      Mat imgBlur;
    Mat imgCanny;
    GaussianBlur(inputImage, imgBlur, Size(9, 9), 1, 1);



    Canny(imgBlur, imgCanny, 10, 30, 3);



    //int morph_elem  = 2;
    //int morph_size = 1;
    //Mat element = getStructuringElement( morph_elem, Size( 2*morph_size + 1, 2*morph_size+1 ), Point( morph_size, morph_size ) );
    morphologyEx( imgCanny, imgCanny, MORPH_CLOSE, element );


    Mat imgShaped;
    imgCanny.copyTo(imgShaped);
    //Find contours of possibles characters
    vector< vector< Point> > biggestShapes;
    findContours(imgShaped,
	    biggestShapes, // a vector of contours
	    CV_RETR_EXTERNAL, // retrieve the external contours
	    CV_CHAIN_APPROX_SIMPLE ); // all pixels of each contours

    // Draw blue contours on a white image
    //cvtColor(imgShaped, imgShaped, CV_GRAY2RGB);
    cv::drawContours(imgShaped,biggestShapes,
	    -1, // draw all contours
	    cv::Scalar(255,255,255), // in blue
	    1); // with a thickness of 1

    displayImage(config, "Blurred", imgCanny);
    displayImage(config, "Blurred Contours", imgShaped);

    vector<Rect> shapeRects( biggestShapes.size() );

    vector<vector<Point> >hull( biggestShapes.size() );
    for( int i = 0; i < biggestShapes.size(); i++ )
     {
       //approxPolyDP( Mat(biggestShapes[i]), shapeRects[i], 3, true );
       convexHull( biggestShapes[i], hull[i], false );
       //approxPolyDP( biggestShapes[i], hull[i], 10, true );

       //minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
     }
  */

}

/*
vector<LineSegment> PlateLines::getLines(Mat edges, bool vertical)
{

  vector<LineSegment> filteredLines;

  int sensitivity;

  LSWMS lswms(Size(edges.cols, edges.rows), 3, 155, false);

  vector<LSEG> lsegs;
  vector<double> errors;
  lswms.run(edges, lsegs, errors);


  for( size_t i = 0; i < lsegs.size(); i++ )
  {

     if (vertical)
     {
	LineSegment candidate;
	if (lsegs[i][0].y <= lsegs[i][1].y)
	  candidate = LineSegment(lsegs[i][0].x, lsegs[i][0].y, lsegs[i][1].x, lsegs[i][1].y);
	else
	  candidate = LineSegment(lsegs[i][1].x, lsegs[i][1].y, lsegs[i][0].x, lsegs[i][0].y);

	cout << "VERT Angle: " << candidate.angle << endl;
	//if ((candidate.angle > 70 && candidate.angle < 110) || (candidate.angle > 250 && candidate.angle < 290))
	//{
	  // good vertical
	  filteredLines.push_back(candidate);

	//}
     }
     else
     {
	LineSegment candidate;
	if (lsegs[i][0].x <= lsegs[i][1].x)
	  candidate = LineSegment(lsegs[i][0].x, lsegs[i][0].y, lsegs[i][1].x, lsegs[i][1].y);
	else
	  candidate = LineSegment(lsegs[i][1].x, lsegs[i][1].y, lsegs[i][0].x, lsegs[i][0].y);
	cout << "HORIZAngle: " << candidate.angle << endl;

	//if ( (candidate.angle > -20 && candidate.angle < 20) || (candidate.angle > 160 && candidate.angle < 200))
	//{
	  // good horizontal
	  filteredLines.push_back(candidate);

	//}
     }
  }

  // if debug is enabled, draw the image
  if (this->debug)
  {
    Mat debugImg(edges.size(), edges.type());
    edges.copyTo(debugImg);
    cvtColor(debugImg,debugImg,CV_GRAY2BGR);

    for( size_t i = 0; i < filteredLines.size(); i++ )
    {

      line( debugImg, filteredLines[i].p1, filteredLines[i].p2, Scalar(0,0,255), 1, CV_AA);
    }
    if (vertical)
      displayImage(config, "Lines Vertical", debugImg);
    else
      displayImage(config, "Lines Horizontal", debugImg);
  }

  return filteredLines;
}
*/


vector<LineSegment> PlateLines::getLines(Mat edges, float sensitivityMultiplier, bool vertical)
{
  if (this->debug)
    cout << "PlateLines::getLines" << endl;

  static int HORIZONTAL_SENSITIVITY = config->plateLinesSensitivityHorizontal;
  static int VERTICAL_SENSITIVITY = config->plateLinesSensitivityVertical;

  vector<Vec2f> allLines;
  vector<LineSegment> filteredLines;

  int sensitivity;
  if (vertical)
    sensitivity = VERTICAL_SENSITIVITY * (1.0 / sensitivityMultiplier);
  else
    sensitivity = HORIZONTAL_SENSITIVITY * (1.0 / sensitivityMultiplier);

  HoughLines( edges, allLines, 1, CV_PI/180, sensitivity, 0, 0 );


  for( size_t i = 0; i < allLines.size(); i++ )
  {
     float rho = allLines[i][0], theta = allLines[i][1];
     Point pt1, pt2;
     double a = cos(theta), b = sin(theta);
     double x0 = a*rho, y0 = b*rho;

     double angle = theta * (180 / CV_PI);
     pt1.x = cvRound(x0 + 1000*(-b));
     pt1.y = cvRound(y0 + 1000*(a));
     pt2.x = cvRound(x0 - 1000*(-b));
     pt2.y = cvRound(y0 - 1000*(a));

     if (vertical)
     {
	if (angle < 20 || angle > 340 || (angle > 160 && angle < 210))
	{
	  // good vertical

	  LineSegment line;
	  if (pt1.y <= pt2.y)
	    line = LineSegment(pt2.x, pt2.y, pt1.x, pt1.y);
	  else
	    line = LineSegment(pt1.x, pt1.y, pt2.x, pt2.y);

	  // Get rid of the -1000, 1000 stuff.  Terminate at the edges of the image
	  // Helps with debugging/rounding issues later
	  LineSegment top(0, 0, edges.cols, 0);
	  LineSegment bottom(0, edges.rows, edges.cols, edges.rows);
	  Point p1 = line.intersection(bottom);
	  Point p2 = line.intersection(top);
	  filteredLines.push_back(LineSegment(p1.x, p1.y, p2.x, p2.y));
	}
     }
     else
     {

	if ( (angle > 70 && angle < 110) || (angle > 250 && angle < 290))
	{
	  // good horizontal

	  LineSegment line;
	  if (pt1.x <= pt2.x)
	    line = LineSegment(pt1.x, pt1.y, pt2.x, pt2.y);
	  else
	    line =LineSegment(pt2.x, pt2.y, pt1.x, pt1.y);

	  // Get rid of the -1000, 1000 stuff.  Terminate at the edges of the image
	  // Helps with debugging/ rounding issues later
	  int newY1 = line.getPointAt(0);
	  int newY2 = line.getPointAt(edges.cols);

	  filteredLines.push_back(LineSegment(0, newY1, edges.cols, newY2));
	}
     }
  }


  return filteredLines;
}





Mat PlateLines::customGrayscaleConversion(Mat src)
{
  Mat img_hsv;
  cvtColor(src,img_hsv,CV_BGR2HSV);


    Mat grayscale = Mat(img_hsv.size(), CV_8U );
    Mat hue(img_hsv.size(), CV_8U );

    for (int row = 0; row < img_hsv.rows; row++)
    {
      for (int col = 0; col < img_hsv.cols; col++)
      {
	int h = (int) img_hsv.at<Vec3b>(row, col)[0];
	int s = (int) img_hsv.at<Vec3b>(row, col)[1];
	int v = (int) img_hsv.at<Vec3b>(row, col)[2];

	int pixval = pow(v, 1.05);


	if (pixval > 255)
	  pixval = 255;
	grayscale.at<uchar>(row, col) = pixval;

	hue.at<uchar>(row, col) = h * (255.0 / 180.0);
      }
    }

    //displayImage(config, "Hue", hue);
    return grayscale;
}
