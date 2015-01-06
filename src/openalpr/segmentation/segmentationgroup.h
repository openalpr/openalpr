/*
 * Copyright (c) 2015 New Designs Unlimited, LLC
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

#ifndef OPENALPR_SEGMENTATIONGROUP_H
#define OPENALPR_SEGMENTATIONGROUP_H

#include <vector>
#include <stdio.h>

#include "opencv2/imgproc/imgproc.hpp"

#include "segment.h"

namespace alpr
{

  class SegmentationGroup
  {

    public:
      SegmentationGroup();
      virtual ~SegmentationGroup();

      void add(int segmentID);

      std::vector<int> segmentIDs;

      bool equals(SegmentationGroup otherGroup);


    private:
      float strength;	// Debuggin purposes -- how many threshold segmentations match this one perfectly

  };

}

#endif // OPENALPR_SEGMENTATIONGROUP_H
