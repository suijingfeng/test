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

#include "tr_local.h"


extern shaderCommands_t tess;
extern cvar_t* r_nocull;
extern refimport_t ri;
extern trGlobals_t tr;


static cvar_t* r_lodscale;
static cvar_t* r_lodbias;// push/pull LOD transitions
/*

All bones should be an identity orientation to display the mesh exactly as it is specified.

For all other frames, the bones represent the transformation from the 
orientation of the bone in the base frame to the orientation in this frame.

*/



// copied and adapted from tr_mesh.c

static float ProjectRadius( float r, vec3_t location )
{

	float c = DotProduct( tr.viewParms.or.axis[0], tr.viewParms.or.origin );
	float dist = DotProduct( tr.viewParms.or.axis[0], location ) - c;

	if ( dist <= 0 )
		return 0;
	vec3_t	p;
	p[0] = 0;
	p[1] = fabs( r );
	p[2] = -dist;

	float projected[4];

/*   
    projected[0] = p[0] * tr.viewParms.projectionMatrix[0] + 
		           p[1] * tr.viewParms.projectionMatrix[4] +
				   p[2] * tr.viewParms.projectionMatrix[8] +
				   tr.viewParms.projectionMatrix[12];

    projected[2] = p[0] * tr.viewParms.projectionMatrix[2] + 
		           p[1] * tr.viewParms.projectionMatrix[6] +
				   p[2] * tr.viewParms.projectionMatrix[10] +
				   tr.viewParms.projectionMatrix[14];
*/

	projected[1] = p[0] * tr.viewParms.projectionMatrix[1] + 
		           p[1] * tr.viewParms.projectionMatrix[5] +
				   p[2] * tr.viewParms.projectionMatrix[9] +
				   tr.viewParms.projectionMatrix[13];


	projected[3] = p[0] * tr.viewParms.projectionMatrix[3] + 
		           p[1] * tr.viewParms.projectionMatrix[7] +
				   p[2] * tr.viewParms.projectionMatrix[11] +
				   tr.viewParms.projectionMatrix[15];


	float pr = projected[1] / projected[3];

	if ( pr > 1.0f )
		pr = 1.0f;

	return pr;
}





static int R_CullModel( md3Header_t *header, trRefEntity_t *ent )
{
	vec3_t bounds[2];

	// compute frame pointers
	md3Frame_t* newFrame = ( md3Frame_t * ) ( (unsigned char*) header + header->ofsFrames ) + ent->e.frame;
	md3Frame_t* oldFrame = ( md3Frame_t * ) ( (unsigned char*) header + header->ofsFrames ) + ent->e.oldframe;

	// cull bounding sphere ONLY if this is not an upscaled entity
	if ( !ent->e.nonNormalizedAxes )
	{
		if ( ent->e.frame == ent->e.oldframe )
		{
			switch ( R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius ) )
			{
                case CULL_OUT:
                    tr.pc.c_sphere_cull_md3_out++;
                    return CULL_OUT;

                case CULL_IN:
                    tr.pc.c_sphere_cull_md3_in++;
                    return CULL_IN;

                case CULL_CLIP:
                    tr.pc.c_sphere_cull_md3_clip++;
                    break;
			}
		}
		else
		{
			int sphereCullB;
			int sphereCull = R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius );
			
            if( newFrame == oldFrame )
				sphereCullB = sphereCull;
			else
				sphereCullB = R_CullLocalPointAndRadius( oldFrame->localOrigin, oldFrame->radius );
	

			if ( sphereCull == sphereCullB )
			{
				if ( sphereCull == CULL_OUT )
				{
					tr.pc.c_sphere_cull_md3_out++;
					return CULL_OUT;
				}
				else if ( sphereCull == CULL_IN )
				{
					tr.pc.c_sphere_cull_md3_in++;
					return CULL_IN;
				}
				else
				{
					tr.pc.c_sphere_cull_md3_clip++;
				}
			}
		}
	}
	
	// calculate a bounding box in the current coordinate system
    int	i;
	for (i = 0 ; i < 3 ; i++)
    {
		bounds[0][i] = oldFrame->bounds[0][i] < newFrame->bounds[0][i] ? oldFrame->bounds[0][i] : newFrame->bounds[0][i];
		bounds[1][i] = oldFrame->bounds[1][i] > newFrame->bounds[1][i] ? oldFrame->bounds[1][i] : newFrame->bounds[1][i];
	}

	switch ( R_CullLocalBox( bounds ) )
	{
        case CULL_IN:
            tr.pc.c_box_cull_md3_in++;
            return CULL_IN;
        case CULL_CLIP:
            tr.pc.c_box_cull_md3_clip++;
            return CULL_CLIP;
        case CULL_OUT:
        default:
            tr.pc.c_box_cull_md3_out++;
            return CULL_OUT;
	}
}




static int R_MDRCullModel( mdrHeader_t *header, trRefEntity_t *ent )
{
	vec3_t bounds[2];
	int	i;

	int frameSize = (size_t)( &((mdrFrame_t *)0)->bones[ header->numBones ] );
	
	// compute frame pointers
	mdrFrame_t* newFrame = ( mdrFrame_t * ) ( (unsigned char*) header + header->ofsFrames + frameSize * ent->e.frame);
	mdrFrame_t* oldFrame = ( mdrFrame_t * ) ( (unsigned char*) header + header->ofsFrames + frameSize * ent->e.oldframe);

	// cull bounding sphere ONLY if this is not an upscaled entity
	if ( !ent->e.nonNormalizedAxes )
	{
		if ( ent->e.frame == ent->e.oldframe )
		{
			switch ( R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius ) )
			{
				// Ummm... yeah yeah I know we don't really have an md3 here.. but we pretend
				// we do. After all, the purpose of mdrs are not that different, are they?
				
				case CULL_OUT:
					tr.pc.c_sphere_cull_md3_out++;
					return CULL_OUT;

				case CULL_IN:
					tr.pc.c_sphere_cull_md3_in++;
					return CULL_IN;

				case CULL_CLIP:
					tr.pc.c_sphere_cull_md3_clip++;
					break;
			}
		}
		else
		{

			int sphereCull = R_CullLocalPointAndRadius( newFrame->localOrigin, newFrame->radius );
			int sphereCullB = ( newFrame == oldFrame ) ? sphereCull : R_CullLocalPointAndRadius( oldFrame->localOrigin, oldFrame->radius );


			if ( sphereCull == sphereCullB )
			{
				if ( sphereCull == CULL_OUT )
				{
					tr.pc.c_sphere_cull_md3_out++;
					return CULL_OUT;
				}
				else if ( sphereCull == CULL_IN )
				{
					tr.pc.c_sphere_cull_md3_in++;
					return CULL_IN;
				}
				else
				{
					tr.pc.c_sphere_cull_md3_clip++;
				}
			}
		}
	}
	
	// calculate a bounding box in the current coordinate system
	for (i = 0 ; i < 3 ; i++)
    {
		bounds[0][i] = oldFrame->bounds[0][i] < newFrame->bounds[0][i] ? oldFrame->bounds[0][i] : newFrame->bounds[0][i];
		bounds[1][i] = oldFrame->bounds[1][i] > newFrame->bounds[1][i] ? oldFrame->bounds[1][i] : newFrame->bounds[1][i];
	}

	switch ( R_CullLocalBox( bounds ) )
	{
		case CULL_IN:
			tr.pc.c_box_cull_md3_in++;
			return CULL_IN;
		case CULL_CLIP:
			tr.pc.c_box_cull_md3_clip++;
			return CULL_CLIP;
		case CULL_OUT:
		default:
			tr.pc.c_box_cull_md3_out++;
			return CULL_OUT;
	}
}


static int R_MDRComputeFogNum( mdrHeader_t *header, trRefEntity_t *ent )
{
	vec3_t localOrigin;

	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		return 0;
	}
	
	int frameSize = (size_t)( &((mdrFrame_t *)0)->bones[ header->numBones ] );

	// FIXME: non-normalized axis issues
	mdrFrame_t* mdrFrame = ( mdrFrame_t * ) ( ( byte * ) header + header->ofsFrames + frameSize * ent->e.frame);
	
    VectorAdd( ent->e.origin, mdrFrame->localOrigin, localOrigin );

    int	i, j;
	for ( i = 1 ; i < tr.world->numfogs ; i++ )
    {
		fog_t* fog = &tr.world->fogs[i];
		for ( j = 0 ; j < 3 ; j++ )
        {
			if ( localOrigin[j] - mdrFrame->radius >= fog->bounds[1][j] )
				break;
		
			if ( localOrigin[j] + mdrFrame->radius <= fog->bounds[0][j] )
				break;
		}

		if ( j == 3 )
			return i;
	}

	return 0;
}



static int R_ComputeFogNum( md3Header_t *header, trRefEntity_t *ent )
{
	int	i, j;
	vec3_t localOrigin;

	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		return 0;
	}

	// FIXME: non-normalized axis issues
	md3Frame_t* md3Frame = ( md3Frame_t * ) ( ( unsigned char * ) header + header->ofsFrames ) + ent->e.frame;
	VectorAdd( ent->e.origin, md3Frame->localOrigin, localOrigin );
	
    for ( i = 1 ; i < tr.world->numfogs ; i++ )
    {
		fog_t* fog = &tr.world->fogs[i];
		for ( j = 0 ; j < 3 ; j++ )
        {
			if ( localOrigin[j] - md3Frame->radius >= fog->bounds[1][j] )
				break;
		
			if ( localOrigin[j] + md3Frame->radius <= fog->bounds[0][j] )
				break;
		}

		if ( j == 3 )
			return i;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////


/*
=================
R_CullLocalBox

Returns CULL_IN, CULL_CLIP, or CULL_OUT
=================
*/
int R_CullLocalBox(vec3_t bounds[2])
{
	int	i, j;
	vec3_t	transformed[8];
	int	anyBack = 0;

	if( r_nocull->integer ) {
		return CULL_CLIP;
	}

	// transform into world space
	for (i = 0 ; i < 8 ; i++)
    {
        vec3_t v;

		v[0] = bounds[i&1][0];
		v[1] = bounds[(i>>1)&1][1];
		v[2] = bounds[(i>>2)&1][2];

		VectorCopy( tr.or.origin, transformed[i] );
		VectorMA( transformed[i], v[0], tr.or.axis[0], transformed[i] );
		VectorMA( transformed[i], v[1], tr.or.axis[1], transformed[i] );
		VectorMA( transformed[i], v[2], tr.or.axis[2], transformed[i] );
	}

	// check against frustum planes
	for (i = 0 ; i < 4 ; i++)
    {
        int front = 0;
        int back = 0;
        float dists[8];
		cplane_t* frust = &tr.viewParms.frustum[i];

		for (j = 0 ; j < 8 ; j++)
        {
			dists[j] = DotProduct(transformed[j], frust->normal);
			if( dists[j] > frust->dist )
            {
				front = 1;
				if ( back )
					break;		// a point is in front
			
			}
            else
				back = 1;

		}
		if ( !front )
        {
			// all points were behind one of the planes
			return CULL_OUT;
		}
		anyBack |= back;
	}

	if ( !anyBack )
    {
		return CULL_IN;		// completely inside frustum
	}

	return CULL_CLIP;		// partially clipped
}


// much stuff in there is just copied from R_AddMd3Surfaces in tr_mesh.c

void R_MDRAddAnimSurfaces( trRefEntity_t *ent )
{
	shader_t* shader;
	int	i, j;

	mdrHeader_t* header = (mdrHeader_t *) tr.currentModel->modelData;
	qboolean personalModel = (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal;
	
	if( ent->e.renderfx & RF_WRAP_FRAMES )
	{
		ent->e.frame %= header->numFrames;
		ent->e.oldframe %= header->numFrames;
	}
	
	
	// Validate the frames so there is no chance of a crash.
	// This will write directly into the entity structure, 
    // so when the surfaces are rendered,
    // they don't need to be range checked again.
	
	if ((ent->e.frame >= header->numFrames)	|| (ent->e.frame < 0) || (ent->e.oldframe >= header->numFrames)	|| (ent->e.oldframe < 0) )
	{
		ri.Printf( PRINT_ALL, "R_MDRAddAnimSurfaces: no such frame %d to %d for '%s'\n", ent->e.oldframe, ent->e.frame, tr.currentModel->name );
		ent->e.frame = 0;
		ent->e.oldframe = 0;
	}

	//
	// cull the entire model if merged bounding box of both frames
	// is outside the view frustum.
	//
	int cull = R_MDRCullModel(header, ent);
	if ( cull == CULL_OUT )
    {
		return;
	}	

	// figure out the current LOD of the model we're rendering, and set the lod pointer respectively.
	int lodnum = R_ComputeLOD(ent);
	// check whether this model has as that many LODs at all. If not, try the closest thing we got.
	if(header->numLODs <= 0)
		return;
	if(header->numLODs <= lodnum)
		lodnum = header->numLODs - 1;

	mdrLOD_t* lod = (mdrLOD_t *)( (unsigned char *)header + header->ofsLODs);
	for(i = 0; i < lodnum; i++)
	{
		lod = (mdrLOD_t *) ((unsigned char *)lod + lod->ofsEnd);
	}
	
	// set up lighting
	if ( !personalModel )
	{
		R_SetupEntityLighting( &tr.refdef, ent );
	}

	// fogNum?
	int fogNum = R_MDRComputeFogNum( header, ent );

	mdrSurface_t* surface = (mdrSurface_t *)( (unsigned char *)lod + lod->ofsSurfaces );

	for ( i = 0 ; i < lod->numSurfaces ; i++ )
	{
		if(ent->e.customShader)
			shader = R_GetShaderByHandle(ent->e.customShader);
		else if(ent->e.customSkin > 0 && ent->e.customSkin < tr.numSkins)
		{
			skin_t* skin = tr.skins[ent->e.customSkin];
			shader = tr.defaultShader;
			
			for(j = 0; j < skin->numSurfaces; j++)
			{
				if (!strcmp(skin->surfaces[j].name, surface->name))
				{
					shader = skin->surfaces[j].shader;
					break;
				}
			}
		}
		else if(surface->shaderIndex > 0)
			shader = R_GetShaderByHandle( surface->shaderIndex );
		else
			shader = tr.defaultShader;

		// we will add shadows even if the main object isn't visible in the view

		if (!personalModel)
			R_AddDrawSurf( (void *)surface, shader, fogNum, qfalse );

		surface = (mdrSurface_t *)( (byte *)surface + surface->ofsEnd );
	}
}



// tr_mesh.c: triangle model functions



void R_AddMD3Surfaces( trRefEntity_t *ent )
{
	int	i;
	md3Shader_t* md3Shader = NULL;
	shader_t* shader = NULL;


	// don't add third_person objects if not in a portal
	qboolean personalModel = (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal;

	if ( ent->e.renderfx & RF_WRAP_FRAMES )
    {
		ent->e.frame %= tr.currentModel->md3[0]->numFrames;
		ent->e.oldframe %= tr.currentModel->md3[0]->numFrames;
	}


	// Validate the frames so there is no chance of a crash.
	// This will write directly into the entity structure, 
    // so when the surfaces are rendered, they don't need to be range checked again.

	if ( (ent->e.frame >= tr.currentModel->md3[0]->numFrames) || (ent->e.frame < 0)
		|| (ent->e.oldframe >= tr.currentModel->md3[0]->numFrames) || (ent->e.oldframe < 0) )
    {
			ri.Printf( PRINT_DEVELOPER, "R_AddMD3Surfaces: no such frame %d to %d for '%s'\n", ent->e.oldframe, ent->e.frame, tr.currentModel->name );
			ent->e.frame = 0;
			ent->e.oldframe = 0;
	}

	// compute LOD
	int lod = R_ComputeLOD( ent );

	md3Header_t* header = tr.currentModel->md3[lod];

	// cull the entire model if merged bounding box of both frames is outside the view frustum.
	int cull = R_CullModel( header, ent );
	if ( cull == CULL_OUT )
    {
		return;
	}

	// set up lighting now that we know we aren't culled
	if ( !personalModel )
    {
		R_SetupEntityLighting( &tr.refdef, ent );
	}

	//
	// see if we are in a fog volume
	//
	int fogNum = R_ComputeFogNum( header, ent );

	//
	// draw all surfaces
	//
	md3Surface_t* surface = (md3Surface_t *)( (unsigned char *)header + header->ofsSurfaces );
	for(i = 0; i<header->numSurfaces; i++)
    {
		if ( ent->e.customShader )
        {
			shader = R_GetShaderByHandle( ent->e.customShader );
		}
        else if ( (ent->e.customSkin > 0) && (ent->e.customSkin < tr.numSkins) )
        {
			skin_t *skin = tr.skins[ent->e.customSkin];

			// match the surface name to something in the skin file
			shader = tr.defaultShader;
            
            int	j;
			for(j = 0 ; j < skin->numSurfaces; j++ )
            {
				// the names have both been lowercased
				if ( !strcmp( skin->surfaces[j].name, surface->name ) )
                {
					shader = skin->surfaces[j].shader;
					break;
				}
			}

			if (shader == tr.defaultShader)
            {
				ri.Printf( PRINT_DEVELOPER, "WARNING: no shader for surface %s in skin %s\n", surface->name, skin->name);
			}
			else if (shader->defaultShader)
            {
				ri.Printf( PRINT_DEVELOPER, "WARNING: shader %s in skin %s not found\n", shader->name, skin->name);
			}

		}
        else if ( surface->numShaders <= 0 )
        {
			shader = tr.defaultShader;
		}
        else
        {
			md3Shader = (md3Shader_t *) ( (byte *)surface + surface->ofsShaders );
			md3Shader += ent->e.skinNum % surface->numShaders;
			shader = tr.shaders[ md3Shader->shaderIndex ];
		}


		// don't add third_person objects if not viewing through a portal
		if ( !personalModel )
        {
			R_AddDrawSurf( (void *)surface, shader, fogNum, qfalse );
		}

		surface = (md3Surface_t *)( (unsigned char*)surface + surface->ofsEnd );
	}
}


int R_ComputeLOD( trRefEntity_t *ent )
{
	float radius;
	float flod, lodscale;
	md3Frame_t *frame;
	mdrHeader_t *mdr;
	mdrFrame_t *mdrframe;
	int lod;

	if ( tr.currentModel->numLods < 2 )
	{
		// model has only 1 LOD level, skip computations and bias
		lod = 0;
	}
	else
	{
		// multiple LODs exist, so compute projected bounding sphere
		// and use that as a criteria for selecting LOD

		if(tr.currentModel->type == MOD_MDR)
		{
			int frameSize;
			mdr = (mdrHeader_t *) tr.currentModel->modelData;
			frameSize = (size_t) (&((mdrFrame_t *)0)->bones[mdr->numBones]);
			
			mdrframe = (mdrFrame_t *) ((unsigned char*) mdr + mdr->ofsFrames + frameSize * ent->e.frame);
			
			radius = RadiusFromBounds(mdrframe->bounds[0], mdrframe->bounds[1]);
		}
		else
		{
			frame = ( md3Frame_t * ) ( ( ( unsigned char * ) tr.currentModel->md3[0] ) + tr.currentModel->md3[0]->ofsFrames );

			frame += ent->e.frame;

			radius = RadiusFromBounds( frame->bounds[0], frame->bounds[1] );
		}

        float projectedRadius = ProjectRadius(radius, ent->e.origin);

		if ( projectedRadius != 0 )
		{
			lodscale = r_lodscale->value;
			if (lodscale > 20)
                lodscale = 20;
			flod = 1.0f - projectedRadius * lodscale;
		}
		else
		{
			// object intersects near view plane, e.g. view weapon
			flod = 0;
		}

		flod *= tr.currentModel->numLods;
		lod = (long)(flod);

		if ( lod < 0 )
		{
			lod = 0;
		}
		else if ( lod >= tr.currentModel->numLods )
		{
			lod = tr.currentModel->numLods - 1;
		}
	}

	lod += r_lodbias->integer;
	
	if ( lod >= tr.currentModel->numLods )
		lod = tr.currentModel->numLods - 1;
	if ( lod < 0 )
		lod = 0;

	return lod;
}


void R_InitAnimation(void)
{
    r_lodscale = ri.Cvar_Get( "r_lodscale", "5", CVAR_CHEAT );
    r_lodbias = ri.Cvar_Get( "r_lodbias", "0", CVAR_ARCHIVE );
}

