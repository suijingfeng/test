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

float origin[3] = {0};
float visBounds[2][3] = {-10, 90, 0.5, -30.5, 60, -200};

static void R_SetFarClip( void )
{
	float	farthestCornerDistance = 0;
	int		i;

	//
	// set far clipping planes dynamically
	//
	farthestCornerDistance = 0;
	for ( i = 0; i < 8; i++ )
	{
		float v[3];
		float vecTo[3];
		float distance;

		if ( i & 1 )
		{
			v[0] = visBounds[0][0];
		}
		else
		{
			v[0] = visBounds[1][0];
		}

		if ( i & 2 )
		{
			v[1] = visBounds[0][1];
		}
		else
		{
			v[1] = visBounds[1][1];
		}

		if ( i & 4 )
		{
			v[2] = visBounds[0][2];
		}
		else
		{
			v[2] = visBounds[1][2];
		}

		// VectorSubtract( v, tr.viewParms.or.origin, vecTo );

		vecTo[0] = v[0] - origin[0];
		vecTo[1] = v[1] - origin[1];
		vecTo[2] = v[2] - origin[2];

		distance = vecTo[0] * vecTo[0] + vecTo[1] * vecTo[1] + vecTo[2] * vecTo[2];

		if ( distance > farthestCornerDistance )
		{
			farthestCornerDistance = distance;
		}
	}
    
	printf("%f\n", sqrt( farthestCornerDistance ));
}



static void R_SetFarClip2( void )
{
	float	farthestCornerDistance = 0;
	int		i;
	for ( i = 0; i < 8; i++ )
	{
		float v[3] = {0};
 
		v[0] = visBounds[!(i&1)][0] - origin[0];
		v[1] = visBounds[!(i&2)][1] - origin[1];
		v[2] = visBounds[!(i&4)][2] - origin[2];
       
		float distance = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

		if( distance > farthestCornerDistance )
		{
			farthestCornerDistance = distance;
		}
	}
	printf("%f\n", sqrtf( farthestCornerDistance ));
}


/*
=================
main
=================
*/
int main( int argc, char **argv )
{
    R_SetFarClip();
    R_SetFarClip2();
	return 0;
}

