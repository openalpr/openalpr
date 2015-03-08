#include "detectorfactory.h"

namespace alpr
{
  Detector* createDetector(Config* config)
  {
    if (config->gpu_mode == 0)
    {
      // CPU mode
      return new DetectorCPU(config);
    }
    else if (config->gpu_mode == 1)
    {
      return new DetectorCUDA(config);
    }
  }

}

