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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "tr_globals.h"
#include "tr_cvar.h"
#include "ref_import.h"
#include "matrix_multiplication.h"
#include "tr_light.h"
#include "tr_surface.h"
#include "tr_world.h"
#include "R_RotateForViewer.h"
#include "R_SortDrawSurfs.h"
#include "srfPoly_type.h"


// these are sort of arbitrary limits.
// the limits apply to the sum of all scenes in a frame --
// the main view, all the 3D icons, etc
#define	MAX_POLYS		600
#define	MAX_POLYVERTS	3000

static int		max_polys;
static int		max_polyverts;

static int	r_firstSceneDrawSurf;

static int	r_numdlights;
static int	r_firstSceneDlight;

static int	r_numentities;
static int	r_firstSceneEntity;

static int	r_numpolys;
static int	r_firstScenePoly;

static int	r_numpolyverts;

static int	r_frameCount;	// incremented every frame



// All of the information needed by the back end must be contained in a backEndData_t.
// This entire structure is duplicated so the front and back end can run in parallel
// on an SMP machine

typedef struct
{
	drawSurf_t	drawSurfs[MAX_DRAWSURFS];
	dlight_t	dlights[MAX_DLIGHTS];
	trRefEntity_t entities[MAX_REFENTITIES];
	srfPoly_t	*polys;//[MAX_POLYS];
	polyVert_t	*polyVerts;//[MAX_POLYVERTS];
//	renderCommandList_t	commands;
} backEndData_t;


static backEndData_t* backEndData;

void R_SceneSetRefDef(void)
{
    // a single frame may have multiple scenes draw inside it --
	// a 3D game view, 3D status bar renderings, 3D menus, etc.
	// They need to be distinguished by the light flare code, because
	// the visibility state for a given surface may be different in
	// each scene / view.

	// derived info

	tr.refdef.floatTime = tr.refdef.time * 0.001f;

	tr.refdef.numDrawSurfs = r_firstSceneDrawSurf;
	tr.refdef.drawSurfs = backEndData->drawSurfs;

	tr.refdef.num_entities = r_numentities - r_firstSceneEntity;
	tr.refdef.entities = &backEndData->entities[r_firstSceneEntity];

	tr.refdef.num_dlights = r_numdlights - r_firstSceneDlight;
	tr.refdef.dlights = &backEndData->dlights[r_firstSceneDlight];

	tr.refdef.numPolys = r_numpolys - r_firstScenePoly;
	tr.refdef.polys = &backEndData->polys[r_firstScenePoly];

	// turn off dynamic lighting globally by clearing all the
	// dlights if it needs to be disabled or if vertex lighting is enabled
	if ( r_dynamiclight->integer == 0 || r_vertexLight->integer == 1 ) {
		tr.refdef.num_dlights = 0;
	}
}


void R_InitNextFrame(void)
{
	r_firstSceneDrawSurf = 0;

	r_numdlights = 0;
	r_firstSceneDlight = 0;

	r_numentities = 0;
	r_firstSceneEntity = 0;

	r_numpolys = 0;
	r_firstScenePoly = 0;

	r_numpolyverts = 0;

    r_frameCount++;
}


void R_InitScene(void)
{
	max_polys = r_maxpolys->integer;
	if (max_polys < MAX_POLYS)
		max_polys = MAX_POLYS;

	max_polyverts = r_maxpolyverts->integer;
	if (max_polyverts < MAX_POLYVERTS)
		max_polyverts = MAX_POLYVERTS;

	unsigned int len = sizeof( backEndData_t ) + sizeof(srfPoly_t) * max_polys + sizeof(polyVert_t) * max_polyverts;
    
    char* ptr = ri.Hunk_Alloc( len, h_low);
    memset(ptr, 0, len);

	backEndData = (backEndData_t *) ptr;
	backEndData->polys = (srfPoly_t *) (ptr + sizeof( backEndData_t ));
	backEndData->polyVerts = (polyVert_t *) (ptr + sizeof( backEndData_t ) + sizeof(srfPoly_t) * max_polys);

    R_InitNextFrame();
}



void RE_ClearScene( void ) {
	r_firstSceneDlight = r_numdlights;
	r_firstSceneEntity = r_numentities;
	r_firstScenePoly = r_numpolys;
}


void R_TheNextScene(void)
{
	r_firstSceneDrawSurf = tr.refdef.numDrawSurfs;
	r_firstSceneEntity = r_numentities;
	r_firstSceneDlight = r_numdlights;
	r_firstScenePoly = r_numpolys;
}




/*
=====================
RE_AddPolyToScene

=====================
*/
void RE_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys ) {
	srfPoly_t	*poly;
	int			i, j;
	int			fogIndex;
	fog_t		*fog;
	vec3_t		bounds[2];

	if ( !tr.registered ) {
		return;
	}

	if ( !hShader ) {
		ri.Printf( PRINT_WARNING, "WARNING: RE_AddPolyToScene: NULL poly shader\n");
		return;
	}

	for ( j = 0; j < numPolys; j++ ) {
		if ( r_numpolyverts + numVerts > max_polyverts || r_numpolys >= max_polys ) {
      /*
      NOTE TTimo this was initially a PRINT_WARNING
      but it happens a lot with high fighting scenes and particles
      since we don't plan on changing the const and making for room for those effects
      simply cut this message to developer only
      */
			ri.Printf( PRINT_DEVELOPER, "WARNING: RE_AddPolyToScene: r_max_polys or r_max_polyverts reached\n");
			return;
		}

		poly = &backEndData->polys[r_numpolys];
		poly->surfaceType = SF_POLY;
		poly->hShader = hShader;
		poly->numVerts = numVerts;
		poly->verts = &backEndData->polyVerts[r_numpolyverts];
		
		memcpy( poly->verts, &verts[numVerts*j], numVerts * sizeof( *verts ) );

		// done.
		r_numpolys++;
		r_numpolyverts += numVerts;

		// if no world is loaded
		if ( tr.world == NULL ) {
			fogIndex = 0;
		}
		// see if it is in a fog volume
		else if ( tr.world->numfogs == 1 ) {
			fogIndex = 0;
		} else {
			// find which fog volume the poly is in
			VectorCopy( poly->verts[0].xyz, bounds[0] );
			VectorCopy( poly->verts[0].xyz, bounds[1] );
			for ( i = 1 ; i < poly->numVerts ; i++ ) {
				AddPointToBounds( poly->verts[i].xyz, bounds[0], bounds[1] );
			}
			for ( fogIndex = 1 ; fogIndex < tr.world->numfogs ; fogIndex++ ) {
				fog = &tr.world->fogs[fogIndex]; 
				if ( bounds[1][0] >= fog->bounds[0][0]
					&& bounds[1][1] >= fog->bounds[0][1]
					&& bounds[1][2] >= fog->bounds[0][2]
					&& bounds[0][0] <= fog->bounds[1][0]
					&& bounds[0][1] <= fog->bounds[1][1]
					&& bounds[0][2] <= fog->bounds[1][2] ) {
					break;
				}
			}
			if ( fogIndex == tr.world->numfogs ) {
				fogIndex = 0;
			}
		}
		poly->fogIndex = fogIndex;
	}
}


//=================================================================================


/*
=====================
RE_AddRefEntityToScene

=====================
*/
void RE_AddRefEntityToScene( const refEntity_t *ent )
{
	if ( !tr.registered ) {
		return;
	}
  // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=402
	if ( r_numentities >= REFENTITYNUM_WORLD ) {
		return;
	}
	if ( ent->reType < 0 || ent->reType >= RT_MAX_REF_ENTITY_TYPE ) {
		ri.Error( ERR_DROP, "RE_AddRefEntityToScene: bad reType %i", ent->reType );
	}

	backEndData->entities[r_numentities].e = *ent;
	backEndData->entities[r_numentities].lightingCalculated = qfalse;

	r_numentities++;
}


static void R_AddDynamicLightToScene( const vec3_t org, float intensity, float r, float g, float b, int additive,
        dlight_t* const pDlightArrary)
{

	if ( r_numdlights >= MAX_DLIGHTS )
    {
        ri.Printf(PRINT_WARNING, "MAX_DLIGHTS HITS\n");
		return;
	}
    
    pDlightArrary[r_numdlights].origin[0] = org[0];
    pDlightArrary[r_numdlights].origin[1] = org[1];
    pDlightArrary[r_numdlights].origin[2] = org[2];
	pDlightArrary[r_numdlights].color[0] = r;
	pDlightArrary[r_numdlights].color[1] = g;
	pDlightArrary[r_numdlights].color[2] = b;
    pDlightArrary[r_numdlights].radius = intensity;
	pDlightArrary[r_numdlights].additive = additive;
    ++r_numdlights;
}

void RE_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) 
{
	if ( tr.registered && (intensity > 0.0f) )
    {
        R_AddDynamicLightToScene( org, intensity, r, g, b, qfalse, backEndData->dlights );
    }
}

void RE_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b )
{
    if ( tr.registered && (intensity <= 0.0f) )
    {
	    R_AddDynamicLightToScene( org, intensity, r, g, b, qtrue, backEndData->dlights );
    }
}


