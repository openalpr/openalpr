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

#ifndef BINARIZEWOLF_H
#define BINARIZEWOLF_H

#include "support/filesystem.h"

#include <stdio.h>
#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

enum NiblackVersion
{
  NIBLACK=0,
  SAUVOLA,
  WOLFJOLION,
};

#define BINARIZEWOLF_VERSION	"2.3 (February 26th, 2013)"
#define BINARIZEWOLF_DEFAULTDR 128

#define uget(x,y)    at<unsigned char>(y,x)
#define uset(x,y,v)  at<unsigned char>(y,x)=v;
#define fget(x,y)    at<float>(y,x)
#define fset(x,y,v)  at<float>(y,x)=v;

void NiblackSauvolaWolfJolion (Mat im, Mat output, NiblackVersion version,
                               int winx, int winy, float k);

#endif // BINARIZEWOLF_H
