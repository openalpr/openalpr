/*
 * Copyright (c) 2014 New Designs Unlimited, LLC
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


#include "textline.h"

TextLine::TextLine(std::vector<cv::Point> textArea, std::vector<cv::Point> linePolygon) {
  if (textArea.size() > 0)
  {
    this->textArea = textArea;
    this->linePolygon = linePolygon;
    
    this->topLine = LineSegment(linePolygon[0].x, linePolygon[0].y, linePolygon[1].x, linePolygon[1].y);
    this->bottomLine = LineSegment(linePolygon[3].x, linePolygon[3].y, linePolygon[2].x, linePolygon[2].y);

    this->charBoxTop = LineSegment(textArea[0].x, textArea[0].y, textArea[1].x, textArea[1].y);
    this->charBoxBottom = LineSegment(textArea[3].x, textArea[3].y, textArea[2].x, textArea[2].y);
    this->charBoxLeft = LineSegment(textArea[3].x, textArea[3].y, textArea[0].x, textArea[0].y);
    this->charBoxRight = LineSegment(textArea[2].x, textArea[2].y, textArea[1].x, textArea[1].y);
  }
}


TextLine::~TextLine() {
}
