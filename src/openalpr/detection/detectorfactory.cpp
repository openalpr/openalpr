#include "detectorfactory.h"

Detector* createDetector(Config* config)
{
    return new DetectorCPU(config);
}

