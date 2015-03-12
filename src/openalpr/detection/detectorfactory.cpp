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
      #if COMPILE_GPU
      return new DetectorCUDA(config);
      #else
      std::cerr << "Error: GPU detector requested, but GPU extensions are not compiled.  " <<
              "Add COMPILE_GPU=1 to the compiler definitions to enable GPU compilation." <<
              std::endl;
      #endif
    }
  }

}

