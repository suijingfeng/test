/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


typedef struct vidmode_s {
	const char *description;
	int width, height;
	float pixelAspect;		// pixel width / height
} vidmode_t;

static const vidmode_t r_vidModes[] = {
	{ "Mode  0: 320x240",		320,	240,	1 },
	{ "Mode  1: 400x300",		400,	300,	1 },
	{ "Mode  2: 512x384",		512,	384,	1 },
	{ "Mode  3: 640x480 (480p)",	640,	480,	1 },
	{ "Mode  4: 800x600",		800,	600,	1 },
	{ "Mode  5: 960x720",		960,	720,	1 },
	{ "Mode  6: 1024x768",		1024,	768,	1 },
	{ "Mode  7: 1152x864",		1152,	864,	1 },
	{ "Mode  8: 1280x1024",		1280,	1024,	1 },
	{ "Mode  9: 1600x1200",		1600,	1200,	1 },
	{ "Mode 10: 2048x1536",		2048,	1536,	1 },
	{ "Mode 11: 856x480",		856,	480,	1 },		// Q3 MODES END HERE AND EXTENDED MODES BEGIN
	{ "Mode 12: 1280x720 (720p)",	1280,	720,	1 },
	{ "Mode 13: 1280x768",		1280,	768,	1 },
	{ "Mode 14: 1280x800",		1280,	800,	1 },
	{ "Mode 15: 1280x960",		1280,	960,	1 },
	{ "Mode 16: 1360x768",		1360,	768,	1 },
	{ "Mode 17: 1366x768",		1366,	768,	1 }, // yes there are some out there on that extra 6
	{ "Mode 18: 1360x1024",		1360,	1024,	1 },
	{ "Mode 19: 1400x1050",		1400,	1050,	1 },
	{ "Mode 20: 1400x900",		1400,	900,	1 },
	{ "Mode 21: 1600x900",		1600,	900,	1 },
	{ "Mode 22: 1680x1050",		1680,	1050,	1 },
	{ "Mode 23: 1920x1080 (1080p)",	1920,	1080,	1 },
	{ "Mode 24: 1920x1200",		1920,	1200,	1 },
	{ "Mode 25: 1920x1440",		1920,	1440,	1 },
	{ "Mode 26: 2560x1600",		2560,	1600,	1 },
	{ "Mode 27: 3840x2160 (4K)",	3840,	2160,	1 }
};

static const int s_numVidModes = (sizeof(r_vidModes) / sizeof(*(r_vidModes)));


void setFovM1( float fov_x, float fov_y, int width, int height)
{

    float fovX = fov_x;
	float fovY = fov_y;
    
    float windowAspect = (float)width/(float)height;
    if(windowAspect > 1.3f)
    {
        // undo vert-
        fovY = fovX * (73.739792 / 90.0);
        // recalculate the fov_x
        fovX = (360.0/M_PI) * atan(tan(fovY * (M_PI/360.0)) * windowAspect);
    }

    printf("leilei ------ fovX: %f, fovY: %f, aspect: %f\n", fovX, fovY, tan(fovX*(M_PI/360.0)) / tan(fovY*(M_PI/360.0)));
}



void setFovM2(float fov_x, float fov_y, int width, int height)
{
	float fovX = fov_x;
	float fovY = fov_y;

    float windowAspect = (float)width/(float)height;

    if(windowAspect > 1.3f)
    {
        fovY = (360.0 / M_PI) * atan( tan( (M_PI/360.0) * fovX ) * 0.75f );
            // Then we use the Hor+ vertical FOV to calculate our new
            // expanded horizontal FOV
        fovX = (360.0 / M_PI) * atan( tan( (M_PI/360.0) * fovY ) * windowAspect );
    }

    printf("ryvnf ------ fovX: %f, fovY: %f, aspect: %f\n", fovX, fovY, tan(fovX*(M_PI/360.0)) / tan(fovY*(M_PI/360.0)));
}


/*
=================
main
=================
*/
int main( int argc, char **argv )
{
    int i = 0;

    float a = 90;
    float b = 0;

    for (i = 0; i < s_numVidModes; i++)
    {
        printf("width: %d, height: %d\n", r_vidModes[i].width, r_vidModes[i].height);
        setFovM1(a, b, r_vidModes[i].width, r_vidModes[i].height);
        setFovM2(a, b, r_vidModes[i].width, r_vidModes[i].height);
        printf("\n");
    }

	return 0;
}

