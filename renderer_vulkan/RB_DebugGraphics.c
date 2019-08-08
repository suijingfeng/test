#include "tr_globals.h"
#include "vk_shade_geometry.h"
#include "vk_instance.h"
#include "vk_image.h"
#include "vk_pipelines.h"
#include "tr_cvar.h"
#include "tr_backend.h"
#include "ref_import.h"
#include "matrix_multiplication.h"
#include "R_ShaderCommands.h"
#include "tr_cmds.h"

extern struct shaderCommands_s tess;

void R_DebugPolygon( int color, int numPoints, float *points )
{
	if (numPoints < 3 || numPoints >= SHADER_MAX_VERTEXES/2)
		return;
    int i;
	// In Vulkan we don't have GL_POLYGON + GLS_POLYMODE_LINE equivalent, 
    // so we use lines to draw polygon outlines.This approach has additional
    // implication that we need to do manual backface culling to reject outlines
    // that belong to back facing polygons. The code assumes that polygons are convex.

	// Backface culling.
    float pa[3], pb[3], p[3];
//	transform_to_eye_space(&points[0], pa);
    
    const float* m = getptr_modelview_matrix();
    
    
	Vec3Transform(points, m, pa);
	Vec3Transform(&points[3], m, pb);    
	VectorSubtract(pb, pa, p);

	float n[3];
	for (i = 2; i < numPoints; ++i)
    {
		Vec3Transform(&points[3*i], m, pb);
		float q[3];
		VectorSubtract(pb, pa, q);
		VectorCross(q, p, n);
		if ( sqrt(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]) > 1e-5)
			break;
	}
	if (DotProduct(n, pa) >= 0)
		return; // discard backfacing polygon

	// Solid shade.
	for (i = 0; i < numPoints; i++)
    {
		VectorCopy(&points[3*i], tess.xyz[i]);

		tess.svars.colors[i][0] = (color&1) ? 255 : 0;
		tess.svars.colors[i][1] = (color&2) ? 255 : 0;
		tess.svars.colors[i][2] = (color&4) ? 255 : 0;
		tess.svars.colors[i][3] = 255;
	}
	tess.numVertexes = numPoints;

	tess.numIndexes = 0;
	for (i = 1; i < numPoints - 1; i++)
    {
		tess.indexes[tess.numIndexes + 0] = 0;
		tess.indexes[tess.numIndexes + 1] = i;
		tess.indexes[tess.numIndexes + 2] = i + 1;
		tess.numIndexes += 3;
	}
    
    vk_UploadXYZI(tess.xyz, tess.numVertexes, tess.indexes, tess.numIndexes);

    updateMVP(backEnd.viewParms.isPortal, backEnd.projection2D, getptr_modelview_matrix());

    vk_shade(g_debugPipelines.surface_solid, &tess, 
            &tr.whiteImage->descriptor_set, VK_FALSE, VK_TRUE);


	// Outline.
	memset(tess.svars.colors, tr.identityLightByte, numPoints * 2 * 4);

	for (i = 0; i < numPoints; ++i)
    {
		VectorCopy(&points[3*i], tess.xyz[2*i]);
		VectorCopy(&points[3*((i + 1) % numPoints)], tess.xyz[2*i + 1]);
	}
	tess.numVertexes = numPoints * 2;
	tess.numIndexes = 0;

    vk_UploadXYZI(tess.xyz, tess.numVertexes, tess.indexes, 0);
    
    updateMVP(backEnd.viewParms.isPortal, backEnd.projection2D, getptr_modelview_matrix());
    vk_shade(g_debugPipelines.surface_outline, &tess, 
            &tr.whiteImage->descriptor_set, VK_FALSE, VK_FALSE);
	
    tess.numVertexes = 0;
}

/*
====================
Visualization aid for movement clipping debugging
====================
*/
void RB_DebugGraphics( void )
{
	// the render thread can't make callbacks to the main thread
	if ( tr.registered ) {
		R_IssueRenderCommands( qfalse );
	}

	ri.CM_DrawDebugSurface( R_DebugPolygon );
}
