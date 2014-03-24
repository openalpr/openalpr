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

/**************************************************************
 * Binarization with several methods
 * (0) Niblacks method
 * (1) Sauvola & Co.
 *     ICDAR 1997, pp 147-152
 * (2) by myself - Christian Wolf
 *     Research notebook 19.4.2001, page 129
 * (3) by myself - Christian Wolf
 *     20.4.2007
 *
 * See also:
 * Research notebook 24.4.2001, page 132 (Calculation of s)
 **************************************************************/

#include "binarize_wolf.h"

// *************************************************************
// glide a window across the image and
// create two maps: mean and standard deviation.
// *************************************************************

float calcLocalStats (Mat &im, Mat &map_m, Mat &map_s, int winx, int winy)
{
  float m,s,max_s;
  long sum, sum_sq;
  uchar foo;
  int wxh	= winx/2;
  int wyh	= winy/2;
  int x_firstth= wxh;
  int y_lastth = im.rows-wyh-1;
  int y_firstth= wyh;
  float winarea = winx*winy;

  max_s = 0;
  for	(int j = y_firstth ; j<=y_lastth; j++)
  {
    // Calculate the initial window at the beginning of the line
    sum = sum_sq = 0;
    for	(int wy=0 ; wy<winy; wy++)
      for	(int wx=0 ; wx<winx; wx++)
      {
        foo = im.uget(wx,j-wyh+wy);
        sum    += foo;
        sum_sq += foo*foo;
      }
    m  = ((float)sum) / winarea;
    s  = sqrt ((((float)sum_sq) - ((float)(sum*sum))/winarea)/winarea);
    if (s > max_s)
      max_s = s;
    map_m.fset(x_firstth, j, m);
    map_s.fset(x_firstth, j, s);

    // Shift the window, add and remove	new/old values to the histogram
    for	(int i=1 ; i <= im.cols-winx; i++)
    {
      // Remove the left old column and add the right new column
      for (int wy=0; wy<winy; ++wy)
      {
        foo = im.uget(i-1,j-wyh+wy);
        sum    -= foo;
        sum_sq -= foo*foo;
        foo = im.uget(i+winx-1,j-wyh+wy);
        sum    += foo;
        sum_sq += foo*foo;
      }
      m  = ((float)sum) / winarea;
      s  = sqrt ((((float)sum_sq) - ((float) (sum*sum))/winarea)/winarea);
      if (s > max_s)
        max_s = s;
      map_m.fset(i+wxh, j, m);
      map_s.fset(i+wxh, j, s);
    }
  }

  return max_s;
}

/**********************************************************
 * The binarization routine
 **********************************************************/

void NiblackSauvolaWolfJolion (Mat im, Mat output, NiblackVersion version,
                               int winx, int winy, float k)
{
  float dR = BINARIZEWOLF_DEFAULTDR;

  float m, s, max_s;
  float th=0;
  double min_I, max_I;
  int wxh	= winx/2;
  int wyh	= winy/2;
  int x_firstth= wxh;
  int x_lastth = im.cols-wxh-1;
  int y_lastth = im.rows-wyh-1;
  int y_firstth= wyh;
  int mx, my;

  // Create local statistics and store them in a float matrices
  Mat map_m = Mat::zeros (im.rows, im.cols, CV_32F);
  Mat map_s = Mat::zeros (im.rows, im.cols, CV_32F);
  max_s = calcLocalStats (im, map_m, map_s, winx, winy);

  minMaxLoc(im, &min_I, &max_I);

  Mat thsurf (im.rows, im.cols, CV_32F);

  // Create the threshold surface, including border processing
  // ----------------------------------------------------

  for	(int j = y_firstth ; j<=y_lastth; j++)
  {
    // NORMAL, NON-BORDER AREA IN THE MIDDLE OF THE WINDOW:
    for	(int i=0 ; i <= im.cols-winx; i++)
    {
      m  = map_m.fget(i+wxh, j);
      s  = map_s.fget(i+wxh, j);

      // Calculate the threshold
      switch (version)
      {
      case NIBLACK:
        th = m + k*s;
        break;

      case SAUVOLA:
        th = m * (1 + k*(s/dR-1));
        break;

      case WOLFJOLION:
        th = m + k * (s/max_s-1) * (m-min_I);
        break;

      default:
        cerr << "Unknown threshold type in ImageThresholder::surfaceNiblackImproved()\n";
        exit (1);
      }

      thsurf.fset(i+wxh,j,th);

      if (i==0)
      {
        // LEFT BORDER
        for (int i=0; i<=x_firstth; ++i)
          thsurf.fset(i,j,th);

        // LEFT-UPPER CORNER
        if (j==y_firstth)
          for (int u=0; u<y_firstth; ++u)
            for (int i=0; i<=x_firstth; ++i)
              thsurf.fset(i,u,th);

        // LEFT-LOWER CORNER
        if (j==y_lastth)
          for (int u=y_lastth+1; u<im.rows; ++u)
            for (int i=0; i<=x_firstth; ++i)
              thsurf.fset(i,u,th);
      }

      // UPPER BORDER
      if (j==y_firstth)
        for (int u=0; u<y_firstth; ++u)
          thsurf.fset(i+wxh,u,th);

      // LOWER BORDER
      if (j==y_lastth)
        for (int u=y_lastth+1; u<im.rows; ++u)
          thsurf.fset(i+wxh,u,th);
    }

    // RIGHT BORDER
    for (int i=x_lastth; i<im.cols; ++i)
      thsurf.fset(i,j,th);

    // RIGHT-UPPER CORNER
    if (j==y_firstth)
      for (int u=0; u<y_firstth; ++u)
        for (int i=x_lastth; i<im.cols; ++i)
          thsurf.fset(i,u,th);

    // RIGHT-LOWER CORNER
    if (j==y_lastth)
      for (int u=y_lastth+1; u<im.rows; ++u)
        for (int i=x_lastth; i<im.cols; ++i)
          thsurf.fset(i,u,th);
  }

  for	(int y=0; y<im.rows; ++y)
    for	(int x=0; x<im.cols; ++x)
    {
      if (im.uget(x,y) >= thsurf.fget(x,y))
      {
        output.uset(x,y,255);
      }
      else
      {
        output.uset(x,y,0);
      }
    }
}
