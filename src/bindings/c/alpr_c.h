/*
 * Copyright (c) 2016 OpenALPR Technology, Inc.
 * Open source Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenALPR.
 *
 * OpenALPR is free software: you can redistribute it and/or modify
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


#ifndef ALPR_C_H
#define ALPR_C_H

#ifdef __cplusplus
extern "C" { 
#endif

#ifdef WIN32
  #define OPENALPRC_DLL_EXPORT __declspec( dllexport )
#else
  #define OPENALPRC_DLL_EXPORT
#endif

typedef void OPENALPR;

struct AlprCRegionOfInterest
{
  int x;
  int y;
  int width;
  int height;
};

// Initializes the openALPR library and returns a pointer to the OpenALPR instance
OPENALPR* openalpr_init(const char* country, const char* configFile, const char* runtimeDir);

// Returns 1 if the library was loaded successfully, 0 otherwise
int openalpr_is_loaded(OPENALPR* instance);

// Set the country used for plate recognition
void openalpr_set_country(OPENALPR* instance, const char* country);

// Update the prewarp setting without reloading the library
void openalpr_set_prewarp(OPENALPR* instance, const char* prewarp_config);
// Update the detection mask without reloading the library
void openalpr_set_mask(OPENALPR* instance, unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight);

// Enable/disable region detection.  Pass a 0 or 1
void openalpr_set_detect_region(OPENALPR* instance, int detectRegion);
void openalpr_set_topn(OPENALPR* instance, int topN);
void openalpr_set_default_region(OPENALPR* instance, const char* region);
      
// Recognizes the provided image and responds with JSON. 
// Image is expected to be raw pixel data (BGR, 3 channels)
// Caller must call free() on the returned object
char* openalpr_recognize_rawimage(OPENALPR* instance, unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight, struct AlprCRegionOfInterest roi);

// Recognizes the encoded (e.g., JPEG, PNG) image.  bytes are the raw bytes for the image data.
char* openalpr_recognize_encodedimage(OPENALPR* instance, unsigned char* bytes, long long length, struct AlprCRegionOfInterest roi);

// Frees a char* response that was provided from a recognition request.
// This is required for interoperating with managed languages (e.g., C#) that can't free the memory themselves
void openalpr_free_response_string(char* response);

// Free the memory for the OpenALPR instance created with openalpr_init
void openalpr_cleanup(OPENALPR* instance);


#ifdef __cplusplus
}
#endif

#endif /* ALPR_C_H */

