/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   alprc_test.c
 * Author: mhill
 *
 * Created on October 16, 2016, 11:09 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include "alpr_c.h"

void ReadFile(const char *name, char** buffer, size_t* size)
{
	FILE *file;

	//Open file
	file = fopen(name, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open file %s", name);
		return;
	}
	
	//Get file length
	fseek(file, 0, SEEK_END);
	*size=ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	*buffer=(char *)malloc(*size+1);
	if (!(*buffer))
	{
		fprintf(stderr, "Memory error!");
                                fclose(file);
		return;
	}

	//Read file contents into buffer
	fread(*buffer, *size, 1, file);
	fclose(file);

}

/*
 * 
 */
int main(int argc, char** argv) {

  OPENALPR* openalpr = openalpr_init("us", "/etc/openalpr/openalpr.conf", "/usr/share/openalpr/runtime_data/");
  
  const char* IMAGE_FILE="/storage/projects/alpr/samples/testing/car1.jpg";
  size_t size;
  char* buffer;
  printf("Loading image: %s\n", IMAGE_FILE);
  ReadFile(IMAGE_FILE, &buffer, &size);
  
  struct AlprCRegionOfInterest roi;
  roi.x = 0;
  roi.y = 0;
  roi.width = 1024;
  roi.height = 768;
  
  printf("Recognizing plates\n");
  printf("Image size: %lu\n", (long) size);
  char* plate_json = openalpr_recognize_encodedimage(openalpr, buffer, size, roi);
  printf("results:\n%s\n", plate_json);
  free(plate_json);
  free(buffer);
  
  openalpr_cleanup(openalpr);
  
  return (EXIT_SUCCESS);
}

