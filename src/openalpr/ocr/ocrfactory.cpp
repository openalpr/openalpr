#include "ocrfactory.h"
#include "tesseract_ocr.h"

namespace alpr
{
  OCR* createOcr(Config* config)
  {
    return new TesseractOcr(config);
  }

}

