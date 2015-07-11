#include "version.h"


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
int cmpVersion(const char *v1, const char *v2)
{
    int i;
    int oct_v1[4], oct_v2[4];
    
    for (int i = 0; i < 4; i++)
    {
      oct_v1[i] = 0;
      oct_v2[i] = 0;
    }
    
    sscanf(v1, "%d.%d.%d.%d", &oct_v1[0], &oct_v1[1], &oct_v1[2], &oct_v1[3]);
    sscanf(v2, "%d.%d.%d.%d", &oct_v2[0], &oct_v2[1], &oct_v2[2], &oct_v2[3]);

    
    for (i = 0; i < 4; i++) {
        if (oct_v1[i] > oct_v2[i])
            return 1;
        else if (oct_v1[i] < oct_v2[i])
            return -1;
    }

    return 0;
}