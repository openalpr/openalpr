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

#ifndef OPENALPR_STATEIDENTIFIER_H
#define OPENALPR_STATEIDENTIFIER_H

#include "opencv2/imgproc/imgproc.hpp"
#include "constants.h"
#include "featurematcher.h"
#include "utility.h"
#include "config.h"
#include "pipeline_data.h"

namespace alpr
{

  class StateIdentifier
  {

    public:
      StateIdentifier(Config* config);
      virtual ~StateIdentifier();

      bool recognize(PipelineData* pipeline_data);

      //int confidence;

    protected:
      Config* config;

    private:

      FeatureMatcher* featureMatcher;

  };

}
#endif // OPENALPR_STATEIDENTIFIER_H
