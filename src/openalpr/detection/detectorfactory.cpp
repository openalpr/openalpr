#include "detectorfactory.h"

namespace alpr
{

  Detector* createDetector(Config* config)
  {
      return new DetectorCPU(config);
  }

}