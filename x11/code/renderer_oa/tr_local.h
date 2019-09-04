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


#ifndef TR_LOCAL_H
#define TR_LOCAL_H

#include "../qcommon/q_shared.h"
#include "../qcommon/qfiles.h"
#include "GL/gl.h"
#include "iqm.h"

#include "../renderercommon/tr_public.h"
#include "../renderercommon/tr_shared.h"

#define PATCH_STITCHING


// any change in the LIGHTMAP_* defines here MUST be reflected in
// R_FindShader() in tr_bsp.c
#define LIGHTMAP_2D         -4	// shader is for 2D rendering
#define LIGHTMAP_BY_VERTEX  -3	// pre-lit triangle models
#define LIGHTMAP_WHITEIMAGE -2
#define LIGHTMAP_NONE       -1


// max dimensions of a grid mesh in memory, tr_surface, tr_bsp
#define	MAX_GRID_SIZE		65
#define	MAX_FACE_POINTS		64


// 14 bits, can't be increased without changing bit packing for drawsurfs, see QSORT_SHADERNUM_SHIFT
#define SHADERNUM_BITS      14
#define MAX_SHADERS         (1<<SHADERNUM_BITS)


typedef struct dlight_s {
	vec3_t	origin;
	vec3_t	color;				// range from 0.0 to 1.0, should be color normalized
	float	radius;

	vec3_t	transformed;		// origin in local coordinate system
	int		additive;			// texture detail is lost tho when the lightmap is dark
} dlight_t;



// a trRefEntity_t has all the information passed in 
// by the client game as well as some locally derived info
typedef struct
{
	refEntity_t	e;

	float		axisLength;		// compensate for non-normalized axis

	qboolean	needDlights;	// true for bmodels that touch a dlight
	qboolean	lightingCalculated;
	vec3_t		lightDir;		// normalized direction towards light
	vec3_t		ambientLight;	// color normalized to 0-255
	unsigned int ambientLightInt;	// 32 bit rgba packed
	vec3_t		directedLight;
	vec3_t		dynamicLight;
	float		lightDistance;
} trRefEntity_t;


typedef struct {
	vec3_t		origin;			// in world coordinates
	vec3_t		axis[3];		// orientation in world
	vec3_t		viewOrigin;		// viewParms->or.origin in local coordinates
	float		modelMatrix[16];
} orientationr_t;

//===============================================================================

typedef enum {
	SS_BAD,
	SS_PORTAL,			// mirrors, portals, viewscreens
	SS_ENVIRONMENT,		// sky box
	SS_OPAQUE,			// opaque

	SS_DECAL,			// scorch marks, etc.
	SS_SEE_THROUGH,		// ladders, grates, grills that may have small blended edges
						// in addition to alpha test
	SS_BANNER,

	SS_FOG,

	SS_UNDERWATER,		// for items that should be drawn in front of the water plane

	SS_BLEND0,			// regular transparency and filters
	SS_BLEND1,			// generally only used for additive type effects
	SS_BLEND2,
	SS_BLEND3,

	SS_BLEND6,
	SS_STENCIL_SHADOW,
	SS_ALMOST_NEAREST,	// gun smoke puffs

	SS_NEAREST			// blood blobs
} shaderSort_t;


#define MAX_SHADER_STAGES 8

typedef enum {
	GF_NONE,

	GF_SIN,
	GF_SQUARE,
	GF_TRIANGLE,
	GF_SAWTOOTH, 
	GF_INVERSE_SAWTOOTH, 

	GF_NOISE

} genFunc_t;


typedef enum {
	DEFORM_NONE,
	DEFORM_WAVE,
	DEFORM_NORMALS,
	DEFORM_BULGE,
	DEFORM_MOVE,
	DEFORM_PROJECTION_SHADOW,
	DEFORM_AUTOSPRITE,
	DEFORM_AUTOSPRITE2,
	DEFORM_TEXT0,
	DEFORM_TEXT1,
	DEFORM_TEXT2,
	DEFORM_TEXT3,
	DEFORM_TEXT4,
	DEFORM_TEXT5,
	DEFORM_TEXT6,
	DEFORM_TEXT7,
	DEFORM_LFX
} deform_t;

typedef enum {
	AGEN_IDENTITY,
	AGEN_SKIP,
	AGEN_ENTITY,
	AGEN_ONE_MINUS_ENTITY,
	AGEN_VERTEX,
	AGEN_ONE_MINUS_VERTEX,
	AGEN_LIGHTING_SPECULAR,
	AGEN_WAVEFORM,
	AGEN_PORTAL,
	AGEN_CONST
} alphaGen_t;


typedef enum {
	CGEN_BAD,
	CGEN_IDENTITY_LIGHTING,	// tr.identityLight
	CGEN_IDENTITY,			// always (1,1,1,1)
	CGEN_ENTITY,			// grabbed from entity's modulate field
	CGEN_ONE_MINUS_ENTITY,	// grabbed from 1 - entity.modulate
	CGEN_EXACT_VERTEX,		// tess.vertexColors
	CGEN_VERTEX,			// tess.vertexColors * tr.identityLight
	CGEN_ONE_MINUS_VERTEX,
	CGEN_WAVEFORM,			// programmatically generated
	CGEN_LIGHTING_DIFFUSE,
	CGEN_LIGHTING_UNIFORM,
	CGEN_LIGHTING_DYNAMIC,
	CGEN_FOG,				// standard fog
	CGEN_CONST,				// fixed color
	CGEN_VERTEX_LIT,			// leilei - tess.vertexColors * tr.identityLight * ambientlight*directlight
} colorGen_t;


typedef enum {
	TCGEN_BAD,
	TCGEN_IDENTITY,			// clear to 0,0
	TCGEN_LIGHTMAP,
	TCGEN_TEXTURE,
	TCGEN_ENVIRONMENT_MAPPED,
    TCGEN_ENVIRONMENT_MAPPED_WATER,
	TCGEN_ENVIRONMENT_CELSHADE_MAPPED,
    TCGEN_ENVIRONMENT_CELSHADE_LEILEI,
	TCGEN_FOG,
	TCGEN_VECTOR			// S and T from world coordinates
} texCoordGen_t;

typedef enum {
	ACFF_NONE,
	ACFF_MODULATE_RGB,
	ACFF_MODULATE_RGBA,
	ACFF_MODULATE_ALPHA
} acff_t;

typedef struct {
	float base;
	float amplitude;
	float phase;
	float frequency;
    genFunc_t func;
} waveForm_t;


////////////////////// image_t  ////////////////////////////// 
typedef enum
{
	IMGTYPE_COLORALPHA, // for color, lightmap, diffuse, and specular
	IMGTYPE_NORMAL,
	IMGTYPE_NORMALHEIGHT,
	IMGTYPE_DELUXE, // normals are swizzled, deluxe are not
} imgType_t;

typedef enum
{
	IMGFLAG_NONE           = 0x0000,
	IMGFLAG_MIPMAP         = 0x0001,
	IMGFLAG_PICMIP         = 0x0002,
	IMGFLAG_CUBEMAP        = 0x0004,
	IMGFLAG_NO_COMPRESSION = 0x0010,
	IMGFLAG_NOLIGHTSCALE   = 0x0020,
	IMGFLAG_CLAMPTOEDGE    = 0x0040,
	IMGFLAG_SRGB           = 0x0080,
	IMGFLAG_GENNORMALMAP   = 0x0100,
} imgFlags_t;

typedef struct image_s {
	char		imgName[MAX_QPATH];		// game path, including extension
    struct image_s*	next;
	
    int			width;
    int         height;				// source image
	int			uploadWidth;
    int         uploadHeight;	// after power of two and picmip but not including clamp to MAX_TEXTURE_SIZE
	GLuint		texnum;					// gl texture binding

	int			frameUsed;			// for texture usage in frame statistics
	int			internalFormat;
	imgType_t   type;
	imgFlags_t  flags;
} image_t;




/*
// leilei - texture atlases
typedef struct {
	float width;			// columns
	float height;			// rows
	float fps;			// frames per second
	int frame;			// offset frame
	float mode;			// 0 - static/anim  1 - entityalpha
} atlas_t;
*/

#define TR_MAX_TEXMODS 4

typedef enum {
	TMOD_NONE,
	TMOD_TRANSFORM,
	TMOD_TURBULENT,
	TMOD_SCROLL,
	TMOD_SCALE,
	TMOD_STRETCH,
	TMOD_LIGHTSCALE,	// leilei - cel hack
//	TMOD_ATLAS,			// leilei - atlases
	TMOD_ROTATE,
	TMOD_ENTITY_TRANSLATE
} texMod_t;

#define	MAX_SHADER_DEFORMS	3
typedef struct
{
	vec3_t		moveVector;
	float		bulgeWidth;
	float		bulgeHeight;
	float		bulgeSpeed;
	float		deformationSpread;
	waveForm_t	deformationWave;
	deform_t	deformation;			// vertex coordinate modification type
} deformStage_t;


typedef struct {
	texMod_t		type;

	// used for TMOD_TURBULENT and TMOD_STRETCH
	waveForm_t		wave;

	// used for TMOD_TRANSFORM
	float			matrix[2][2];		// s' = s * m[0][0] + t * m[1][0] + trans[0]
	float			translate[2];		// t' = s * m[0][1] + t * m[0][1] + trans[1]

	// used for TMOD_SCALE
	float			scale[2];			// s *= scale[0]
	                                    // t *= scale[1]

	// used for TMOD_SCROLL
	float			scroll[2];			// s' = s + scroll[0] * time
										// t' = t + scroll[1] * time
	// leilei - used for TMOD_ATLAS
	//atlas_t			atlas;

	// + = clockwise
	// - = counterclockwise
	float			rotateSpeed;

} texModInfo_t;


#define	MAX_IMAGE_ANIMATIONS	8

typedef struct
{
	image_t*        image[MAX_IMAGE_ANIMATIONS];
	int				numImageAnimations;
	float			imageAnimationSpeed;

	texCoordGen_t	tcGen;
	vec3_t			tcGenVectors[2];

	int				numTexMods;
	texModInfo_t	*texMods;

	int				videoMapHandle;
	qboolean		isLightmap;
	qboolean		isVideoMap;
} textureBundle_t;

#define NUM_TEXTURE_BUNDLES 8

typedef struct {
	qboolean		active;
	
	textureBundle_t	bundle[NUM_TEXTURE_BUNDLES];

	waveForm_t		rgbWave;
	colorGen_t		rgbGen;

	waveForm_t		alphaWave;
	alphaGen_t		alphaGen;

	unsigned char	constantColor[4];			// for CGEN_CONST and AGEN_CONST

	unsigned int	stateBits;					// GLS_xxxx mask

	acff_t			adjustColorsForFog;

	qboolean		isDetail;
} shaderStage_t;

struct shaderCommands_s;

typedef enum {
	CT_FRONT_SIDED,
	CT_BACK_SIDED,
	CT_TWO_SIDED
} cullType_t;

typedef enum {
	FP_NONE,		// surface is translucent and will just be adjusted properly
	FP_EQUAL,		// surface is opaque but possibly alpha tested
	FP_LE			// surface is trnaslucent, but still needs a fog pass (fog surface)
} fogPass_t;

typedef struct {
	float		cloudHeight;
	image_t		*outerbox[6], *innerbox[6];
} skyParms_t;

typedef struct {
	vec3_t	color;
	float	depthForOpaque;
} fogParms_t;



typedef struct shader_s
{
	char		name[MAX_QPATH];		// game path, including extension
	int			lightmapIndex;			// for a shader to match, both name and lightmapIndex must match

	int			index;					// this shader == tr.shaders[index]
	int			sortedIndex;			// this shader == tr.sortedShaders[sortedIndex]

	float		sort;					// lower numbered shaders draw before higher numbered

	qboolean	defaultShader;			// we want to return index 0 if the shader failed to load for some reason, 
                                        // but R_FindShader should still keep a name allocated for it, 
                                        // so if something calls RE_RegisterShader again with the same name, 
                                        // we don't try looking for it again

	qboolean	explicitlyDefined;		// found in a .shader file

	int			surfaceFlags;			// if explicitlyDefined, this will have SURF_* flags
	int			contentFlags;

	qboolean	entityMergable;			// merge across entites optimizable (smoke, blood)

	qboolean	isSky;
	skyParms_t	sky;
	fogParms_t	fogParms;

	float		portalRange;			// distance to fog out at

	int			multitextureEnv;		// 0, GL_MODULATE, GL_ADD (FIXME: put in stage)

	cullType_t	cullType;				// CT_FRONT_SIDED, CT_BACK_SIDED, or CT_TWO_SIDED
	qboolean	polygonOffset;			// set for decals and other items that must be offset 
	qboolean	noMipMaps;				// for console fonts, 2D elements, etc.
	qboolean	noPicMip;				// for images that must always be full resolution

	fogPass_t	fogPass;				// draw a blended pass, possibly with depth test equals

	qboolean	needsNormal;			// not all shaders will need all data to be gathered
	qboolean	needsST1;
	qboolean	needsST2;
	qboolean	needsColor;

	//	leilei - automatic detail texturing
	int		hasDetail;	// shader has a detail stage
	int		hasDepthWrite;	// shader has a depthwrite stage (detailing around holes)
	int		hasMaterial;	// shader represents this material

	int			numDeforms;
	deformStage_t	deforms[MAX_SHADER_DEFORMS];

	int			numUnfoggedPasses;
	shaderStage_t	*stages[MAX_SHADER_STAGES];		

	void	(*optimalStageIteratorFunc)( void );

    float clampTime;                      // time this shader is clamped to
    float timeOffset;                     // current time offset for this shader

    struct shader_s *remappedShader;      // current shader this one is remapped too
    struct shader_s *next;
} shader_t;


// trRefdef_t holds everything that comes in refdef_t,
// as well as the locally generated scene information
typedef struct {
	int			x, y, width, height;
	float		fov_x, fov_y;
	vec3_t		vieworg;
	vec3_t		viewaxis[3];		// transformation matrix

	int			time;				// time in milliseconds for shader effects and other time dependent rendering issues
	int			rdflags;			// RDF_NOWORLDMODEL, etc

	// 1 bits will prevent the associated area from rendering at all
	unsigned char areamask[MAX_MAP_AREA_BYTES];
	qboolean	areamaskModified;	// qtrue if areamask changed since last scene

	float		floatTime;			// tr.refdef.time / 1000.0

	// text messages for deform text shaders
	char		text[MAX_RENDER_STRINGS][MAX_RENDER_STRING_LENGTH];

	int			num_entities;
	trRefEntity_t	*entities;

	int			num_dlights;
	struct dlight_s	*dlights;

	int			numPolys;
	struct srfPoly_s	*polys;

	int			numDrawSurfs;
	struct drawSurf_s* drawSurfs;

} trRefdef_t;


//=================================================================================
// max surfaces per-skin
// This is an arbitry limit. Vanilla Q3 only supported 32 surfaces in skins but failed to
// enforce the maximum limit when reading skin files. It was possile to use more than 32
// surfaces which accessed out of bounds memory past end of skin->surfaces hunk block.
#define MAX_SKIN_SURFACES	256
// skins allow models to be retextured without modifying the model file
typedef struct {
	char		name[MAX_QPATH];
	shader_t	*shader;
} skinSurface_t;

typedef struct skin_s {
	char name[MAX_QPATH];		// game path, including extension
	int	numSurfaces;
	skinSurface_t* surfaces;
} skin_t;


typedef struct {
	int			originalBrushNumber;
	vec3_t		bounds[2];

	unsigned	colorInt;				// in packed byte format
	float		tcScale;				// texture coordinate vector scales
	fogParms_t	parms;

	// for clipping distance in fog when outside
	qboolean	hasSurface;
	float		surface[4];
} fog_t;

typedef struct {
	orientationr_t	or;
	orientationr_t	world;
	vec3_t		pvsOrigin;			// may be different than or.origin for portals
	qboolean	isPortal;			// true if this view is through a portal
	qboolean	isMirror;			// the portal is a mirror, invert the face culling
	int			frameSceneNum;		// copied from tr.frameSceneNum
	int			frameCount;			// copied from tr.frameCount
	cplane_t	portalPlane;		// clip anything behind this if mirroring
	int			viewportX, viewportY, viewportWidth, viewportHeight;
	float		fovX, fovY;
	float		projectionMatrix[16];
	cplane_t	frustum[4];
	vec3_t		visBounds[2];
	float		zFar;
} viewParms_t;


/*
==============================================================================

SURFACES

==============================================================================
*/

// any changes in surfaceType must be mirrored in rb_surfaceTable[]
typedef enum {
	SF_BAD,
    SF_SKIP,
	SF_FACE,
	SF_GRID,
	SF_TRIANGLES,
	SF_POLY,
	SF_MD3,
	SF_MDR,
	SF_IQM,
	SF_FLARE,
	SF_ENTITY,				// beams, rails, lightning, etc that can be determined by entity
	SF_NUM_SURFACE_TYPES,
	SF_MAX = 0x7fffffff			// ensures that sizeof( surfaceType_t ) == sizeof( int )
} surfaceType_t;


typedef struct drawSurf_s
{
    unsigned int sort;			// bit combination for fast compares
    surfaceType_t* surface;		// any of surface*_t
} drawSurf_t;



// when cgame directly specifies a polygon, it becomes a srfPoly_t as soon as it is called
typedef struct srfPoly_s {
	surfaceType_t	surfaceType;
	qhandle_t		hShader;
	int				fogIndex;
	int				numVerts;
	polyVert_t		*verts;
} srfPoly_t;

typedef struct srfDisplayList_s {
	surfaceType_t	surfaceType;
	int				listNum;
} srfDisplayList_t;


typedef struct srfFlare_s {
	surfaceType_t	surfaceType;
	vec3_t			origin;
	vec3_t			normal;
	vec3_t			color;
	shader_t		*shadder;	// leilei - for custom flares
} srfFlare_t;

typedef struct srfGridMesh_s {
	surfaceType_t	surfaceType;

	// dynamic lighting information
	int				dlightBits;

	// culling information
	vec3_t			meshBounds[2];
	vec3_t			localOrigin;
	float			meshRadius;

	// lod information, which may be different
	// than the culling information to allow for
	// groups of curves that LOD as a unit
	vec3_t			lodOrigin;
	float			lodRadius;
	int				lodFixed;
	int				lodStitched;

	// vertexes
	int				width, height;
	float			*widthLodError;
	float			*heightLodError;
	drawVert_t		verts[1];		// variable sized
} srfGridMesh_t;



#define	VERTEXSIZE	8
typedef struct {
	surfaceType_t	surfaceType;
    cplane_t	plane;

	// dynamic lighting information
	int			dlightBits;

	// triangle definitions (no normals at points)
	int			numPoints;
	int			numIndices;
	int			ofsIndices;
	float		points[1][VERTEXSIZE];	// variable sized
										// there is a variable length list of indices here also
} srfSurfaceFace_t;


// misc_models in maps are turned into direct geometry by q3map
typedef struct {
	surfaceType_t	surfaceType;

	// dynamic lighting information
	int				dlightBits;

	// culling information (FIXME: use this!)
	vec3_t			bounds[2];
	vec3_t			localOrigin;
	float			radius;

	// triangle definitions
	int				numIndexes;
	int				*indexes;

	int				numVerts;
	drawVert_t		*verts;
} srfTriangles_t;

// inter-quake-model
typedef struct {
	int		num_vertexes;
	int		num_triangles;
	int		num_frames;
	int		num_surfaces;
	int		num_joints;
	int		num_poses;
	struct srfIQModel_s	*surfaces;

	float		*positions;
	float		*texcoords;
	float		*normals;
	float		*tangents;
	unsigned char* blendIndexes;
	union {
		float	*f;
		unsigned char* b;
	} blendWeights;
	unsigned char* colors;
	int	* triangles;

	// depending upon the exporter, blend indices and weights might be int/float
	// as opposed to the recommended byte/byte, for example Noesis exports
	// int/float whereas the official IQM tool exports byte/byte
	unsigned char blendWeightsType; // IQM_UBYTE or IQM_FLOAT

	int		*jointParents;
	float		*jointMats;
	float		*poseMats;
	float		*bounds;
	char		*names;
} iqmData_t;

// inter-quake-model surface
typedef struct srfIQModel_s {
	surfaceType_t	surfaceType;
	char		name[MAX_QPATH];
	shader_t	*shader;
	iqmData_t	*data;
	int		first_vertex, num_vertexes;
	int		first_triangle, num_triangles;
} srfIQModel_t;




/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2

typedef struct msurface_s {
	surfaceType_t*      data;			// any of srf*_t
    struct shader_s*    shader;

    int					fogIndex;
	int				    viewCount;		// if == tr.viewCount, already added
} msurface_t;



#define	CONTENTS_NODE		-1
typedef struct mnode_s {
	// common with leaf and node
	int			contents;		// -1 for nodes, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current
	vec3_t		mins, maxs;		// for bounding box culling
	struct mnode_s* parent;

	// node specific
	cplane_t* plane;
	struct mnode_s* children[2];	

	// leaf specific
	int			cluster;
	int			area;

	msurface_t** firstmarksurface;
	int			nummarksurfaces;
} mnode_t;

typedef struct {
	vec3_t		bounds[2];		// for culling
	msurface_t* firstSurface;
	int			numSurfaces;
} bmodel_t;

typedef struct {
	char		name[MAX_QPATH];		// ie: maps/tim_dm2.bsp
	char		baseName[MAX_QPATH];	// ie: tim_dm2

	int			dataSize;
	int			numShaders;
	dshader_t	*shaders;

	bmodel_t	*bmodels;

	cplane_t	*planes;
	int			numplanes;
    
	int			numnodes;		// includes leafs
	mnode_t		*nodes;
	int			numDecisionNodes;

	int			numsurfaces;
	msurface_t	*surfaces;

	msurface_t	**marksurfaces;
	int			nummarksurfaces;

	int			numfogs;
	fog_t		*fogs;

	vec3_t		lightGridOrigin;
	vec3_t		lightGridSize;
	vec3_t		lightGridInverseSize;
	int			lightGridBounds[3];


	unsigned char* lightGridData;


	int			numClusters;
	int			clusterBytes;
	const unsigned char* vis;			// may be passed in by CM_LoadMap to save space

	unsigned char* novis;			// clusterBytes of 0xff

	char*       entityString;
	char*       entityParsePoint;
} world_t;

#define MAX_PROGRAMS 256
#define MAX_PROGRAM_OBJECTS 8



//======================================================================

typedef enum {
	MOD_BAD,
	MOD_BRUSH,
	MOD_MESH,
	MOD_MDR,
	MOD_IQM
} modtype_t;


typedef struct model_s {
	char		name[MAX_QPATH];
	int			index;		// model = tr.models[model->index]

	int			dataSize;	// just for listing purposes
	bmodel_t*   bmodel;		// only if type == MOD_BRUSH
	md3Header_t* md3[MD3_MAX_LODS];	// only if type == MOD_MESH
	void*       modelData;			// only if type == (MOD_MDR | MOD_IQM )

	int         numLods;
    modtype_t	type;

} model_t;


#define	MAX_MOD_KNOWN	1024




//====================================================
#define	MAX_DRAWSURFS			0x10000


#define	MAX_DRAWIMAGES			2048
#define	MAX_LIGHTMAPS			256
#define	MAX_SKINS				1024




/*

the drawsurf sort data is packed into a single 32 bit value so it can be
compared quickly during the qsorting process

the bits are allocated as follows:

0 - 1	: dlightmap index
//2		: used to be clipped flag REMOVED - 03.21.00 rad
2 - 6	: fog index
11 - 20	: entity index
21 - 31	: sorted shader index

	TTimo - 1.32
0-1   : dlightmap index
2-6   : fog index
7-16  : entity index
17-30 : sorted shader index
*/
#define	QSORT_FOGNUM_SHIFT          2
#define	QSORT_REFENTITYNUM_SHIFT	7
#define	QSORT_SHADERNUM_SHIFT	(QSORT_REFENTITYNUM_SHIFT+REFENTITYNUM_BITS)
#if (QSORT_SHADERNUM_SHIFT+SHADERNUM_BITS) > 32
	#error "Need to update sorting, too many bits."
#endif



/*
** performanceCounters_t
*/
typedef struct {
	int		c_sphere_cull_patch_in, c_sphere_cull_patch_clip, c_sphere_cull_patch_out;
	int		c_box_cull_patch_in, c_box_cull_patch_clip, c_box_cull_patch_out;
	int		c_sphere_cull_md3_in, c_sphere_cull_md3_clip, c_sphere_cull_md3_out;
	int		c_box_cull_md3_in, c_box_cull_md3_clip, c_box_cull_md3_out;

	int		c_leafs;
	int		c_dlightSurfaces;
	int		c_dlightSurfacesCulled;
} frontEndCounters_t;

#define	FOG_TABLE_SIZE		256
#define FUNCTABLE_SIZE		1024
#define FUNCTABLE_SIZE2		10
#define FUNCTABLE_MASK		(FUNCTABLE_SIZE-1)


// the renderer front end should never modify glstate_t
typedef struct {
	int			currenttextures[2];
	int			currenttmu;
	qboolean	finishCalled;
	int			texEnv[2];
	int			faceCulling;
	unsigned long glStateBits;
} glstate_t;


typedef struct {
	int		c_surfaces, c_shaders, c_vertexes, c_indexes, c_totalIndexes;
	float	c_overDraw;
	
	int		c_dlightVertexes;
	int		c_dlightIndexes;

	int		c_flareAdds;
	int		c_flareTests;
	int		c_flareRenders;

	int		msec;			// total msec for backend run
} backEndCounters_t;


// all state modified by the back end is seperated from the front end state
typedef struct {
	trRefdef_t	refdef;
	viewParms_t	viewParms;
	orientationr_t	or;
	backEndCounters_t	pc;
	trRefEntity_t	entity2D;	// currentEntity will point at this when doing 2D rendering
   	trRefEntity_t* currentEntity; 
    qboolean	skyRenderedThisView;	// flag for drawing sun
	qboolean	isHyperspace;
	qboolean	projection2D;	// if qtrue, drawstretchpic doesn't need to change modes
	qboolean	vertexes2D;		// shader needs to be finished
    qboolean doneSunFlare;

	unsigned char color2D[4];

} backEndState_t;

/*
** trGlobals_t 
**
** Most renderer globals are defined here.
** backend functions should never modify any of these fields,
** but may read fields that aren't dynamically modified by the frontend.
*/
typedef struct {
	qboolean	registered;		// cleared at shutdown, set at beginRegistration

	int			visCount;		// incremented every time a new vis cluster is entered
	int			frameCount;		// incremented every frame
	int			sceneCount;		// incremented every scene
	int			viewCount;		// incremented every view (twice a scene if portaled)
											// and every R_MarkFragments call

	int			frameSceneNum;	// zeroed at RE_BeginFrame

	qboolean	worldMapLoaded;
	world_t*    world;

	const unsigned char*    externalVisData;	// from RE_SetWorldVisData, shared with CM_Load

	image_t*    defaultImage;
	image_t*    scratchImage[32];
	image_t*    fogImage;
	image_t*    dlightImage;	// inverse-quare highlight for projective adding
	image_t*    waterImage;
	image_t*    flareImage;
	image_t*    whiteImage;			// full of 0xff
	image_t*    identityLightImage;	// full of tr.identityLightByte

	shader_t*   defaultShader;
	shader_t*   shadowShader;
	shader_t*   projectionShadowShader;
	shader_t*   flareShader;
	shader_t*   sunShader;

	int					numLightmaps;
	image_t**           lightmaps;

	trRefEntity_t*      currentEntity;
	trRefEntity_t		worldEntity;		// point currentEntity at this when rendering world
	int					currentEntityNum;
	int					shiftedEntityNum;	// currentEntityNum << QSORT_REFENTITYNUM_SHIFT
	model_t*            currentModel;

	viewParms_t			viewParms;

	float				identityLight;		// 1.0 / ( 1 << overbrightBits )
	int					identityLightByte;	// identityLight * 255
	int					overbrightBits;		// r_overbrightBits->integer, but set to 0 if no hw gamma

	orientationr_t		or;					// for current entity

	trRefdef_t			refdef;

	int					viewCluster;

	vec3_t				sunLight;			// from the sky shader for this level
	vec3_t				sunDirection;

	frontEndCounters_t	pc;
	int					frontEndMsec;		// not in pc due to clearing issue

	// put large tables at the end, so most elements will be
	// within the +/32K indexed range on risc processors
	//
	model_t*            models[MAX_MOD_KNOWN];
	int					numModels;


	int					numImages;
	image_t*            images[MAX_DRAWIMAGES];

	// shader indexes from other modules will be looked up in tr.shaders[]
	// shader indexes from drawsurfs will be looked up in sortedShaders[]
	// lower indexed sortedShaders must be rendered first (opaque surfaces before translucent)
	int					numShaders;
	shader_t*           shaders[MAX_SHADERS];
	shader_t*           sortedShaders[MAX_SHADERS];

	int					numSkins;
	skin_t*             skins[MAX_SKINS];

	float				sinTable[FUNCTABLE_SIZE];
	float				squareTable[FUNCTABLE_SIZE];
	float				triangleTable[FUNCTABLE_SIZE];
	float				sawToothTable[FUNCTABLE_SIZE];
	float				inverseSawToothTable[FUNCTABLE_SIZE];
	float				fogTable[FOG_TABLE_SIZE];

} trGlobals_t;







/*
====================================================================

TESSELATOR/SHADER DECLARATIONS

====================================================================
*/

typedef struct stageVars
{
    unsigned char colors[SHADER_MAX_VERTEXES][4];
	vec2_t		texcoords[NUM_TEXTURE_BUNDLES][SHADER_MAX_VERTEXES];
} stageVars_t;


typedef struct shaderCommands_s 
{
	unsigned int indexes[SHADER_MAX_INDEXES] QALIGN(16);
	vec4_t		xyz[SHADER_MAX_VERTEXES];
	vec4_t		normal[SHADER_MAX_VERTEXES];
	vec2_t		texCoords[SHADER_MAX_VERTEXES][2];
	int			vertexDlightBits[SHADER_MAX_VERTEXES];
	unsigned char vertexColors[SHADER_MAX_VERTEXES][4];
	unsigned char constantColor255[SHADER_MAX_VERTEXES][4];

    stageVars_t	svars;
	
    shader_t*   shader;
    
	// info extracted from current shader
	void		(*currentStageIteratorFunc)( void );
	shaderStage_t** xstages;
	int			numPasses;

	float		shaderTime;
	int			fogNum;

	int			dlightBits;	// or together of all vertexDlightBits

	int			numIndexes;
	int			numVertexes;
} shaderCommands_t;






/*
=============================================================

RENDERER BACK END COMMAND QUEUE

=============================================================
*/


typedef struct {
	int		commandId;
	float	color[4];
} setColorCommand_t;

typedef struct {
	int		commandId;
	int		buffer;
} drawBufferCommand_t;

typedef struct {
	int		commandId;
	image_t	*image;
	int		width;
	int		height;
	void	*data;
} subImageCommand_t;

typedef struct {
	int		commandId;
} swapBuffersCommand_t;

typedef struct {
	int		commandId;
	int		buffer;
} endFrameCommand_t;

typedef struct {
	int		commandId;
	shader_t* shader;
	float	x, y;
	float	w, h;
	float	s1, t1;
	float	s2, t2;
} stretchPicCommand_t;

typedef struct {
	int		commandId;
	trRefdef_t	refdef;
	viewParms_t	viewParms;
	drawSurf_t* drawSurfs;
	int		numDrawSurfs;
} drawSurfsCommand_t;

typedef struct {
	int commandId;
	int x;
	int y;
	int width;
	int height;
	char *fileName;
	qboolean jpeg;
} screenshotCommand_t;

typedef struct {
	int				commandId;
	int				width;
	int				height;
	unsigned char*  captureBuffer;
	unsigned char*  encodeBuffer;
	qboolean        motionJpeg;
} videoFrameCommand_t;

typedef struct
{
	int commandId;
	GLboolean rgba[4];
} colorMaskCommand_t;

typedef struct
{
	int commandId;
} clearDepthCommand_t;

typedef enum {
	RC_END_OF_LIST,
	RC_SET_COLOR,
	RC_STRETCH_PIC,
	RC_DRAW_SURFS,
	RC_DRAW_BUFFER,
	RC_SWAP_BUFFERS,
	RC_SCREENSHOT,
	RC_VIDEOFRAME,
	RC_COLORMASK,
	RC_CLEARDEPTH
} renderCommand_t;



////////////////////////////  tr_main.c  //////////////////////////////
#define	CULL_IN		0		// completely unclipped
#define	CULL_CLIP	1		// clipped by one or more planes
#define	CULL_OUT	2		// completely outside the clipping planes


//void R_SetupProjection(viewParms_t *dest, float zProj);
void R_AddDrawSurf( surfaceType_t *surface, shader_t *shader, int fogIndex, int dlightMap );
void R_DecomposeSort( unsigned sort, int *entityNum, shader_t **shader, int *fogNum, int *dlightMap );
void R_RenderView( viewParms_t *parms );
void R_LocalNormalToWorld (vec3_t local, vec3_t world);
void R_LocalPointToWorld (vec3_t local, vec3_t world);
void R_TransformModelToClip( const vec3_t src, const float *modelMatrix, const float *projectionMatrix, vec4_t eye, vec4_t dst);
void R_TransformClipToWindow( const vec4_t clip, const viewParms_t *view, vec4_t normalized, vec4_t window );
void R_RotateForEntity( const trRefEntity_t *ent, const viewParms_t *viewParms, orientationr_t *or );

void R_InitMain(void);

/////////////////////////// tr_world //////////////////////////////////
// WORLD MAP

void R_AddBrushModelSurfaces( trRefEntity_t *e );
qboolean R_inPVS( const vec3_t p1, const vec3_t p2 );
void R_AddWorldSurfaces( void );
int R_CullPointAndRadius( vec3_t origin, float radius );
int R_CullLocalPointAndRadius( vec3_t origin, float radius ); 
void R_InitWorld(void);


/////////////////////////// tr_animation //////////////////////////////////
// ANIMATED MODELS
int R_ComputeLOD( trRefEntity_t *ent );

int R_CullLocalBox(vec3_t bounds[2]);

void R_MDRAddAnimSurfaces( trRefEntity_t *ent );
void R_AddMD3Surfaces( trRefEntity_t *e );
void R_InitAnimation(void);



/////////////////////////// tr_backend.c  ////////////////////////////

// RENDERER BACK END FUNCTIONS
// GL wrapper/helper functions
void GL_Bind( image_t *image );
void GL_SelectTexture( int unit );
void GL_Cull( int cullType );
void GL_TexEnv( int env );
void GL_State( unsigned long stateVector );


void RE_StretchRaw (int x, int y, int w, int h, int cols, int rows, const unsigned char *data, int client, qboolean dirty);
void RE_UploadCinematic (int w, int h, int cols, int rows, const unsigned char *data, int client, qboolean dirty);
void RB_ShowImages( void );
void RB_ExecuteRenderCommands( const void *data );
unsigned char* RB_ReadPixels(int x, int y, int width, int height, size_t *offset, int *padlen);
void R_InitBackend(void);

///////////////////////////////// tr_cmds ///////////////////////////////////
// all of the information needed by the back end must be contained in a backEndData_t
#define	MAX_RENDER_COMMANDS	0x40000

typedef struct {
	unsigned char cmds[MAX_RENDER_COMMANDS];
	int		used;
} renderCommandList_t;

typedef struct {
	drawSurf_t	drawSurfs[MAX_DRAWSURFS];
	dlight_t	dlights[MAX_DLIGHTS];
	trRefEntity_t	entities[MAX_REFENTITIES];
	srfPoly_t	*polys;//[MAX_POLYS];
	polyVert_t	*polyVerts;//[MAX_POLYVERTS];
	renderCommandList_t	commands;
} backEndData_t;




void* R_GetCommandBuffer( int bytes );
void  R_AddDrawSurfCmd( drawSurf_t *drawSurfs, int numDrawSurfs );

void RE_SetColor( const float *rgba );
void RE_StretchPic ( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
void RE_BeginFrame( void );
void RE_EndFrame( int *frontEndMsec, int *backEndMsec );
void RE_TakeVideoFrame( int width, int height, unsigned char *captureBuffer, unsigned char *encodeBuffer, qboolean motionJpeg );



/////////////////////////// tr_image.c //////////////////////////////////
void    GL_TextureMode( const char *string );
int		R_SumOfUsedImages( void );
image_t *R_CreateImage( const char *name, unsigned char *pic, int width, int height, imgType_t type, imgFlags_t flags);
image_t *R_FindImageFile( const char *name, imgType_t type, imgFlags_t flags );
void	R_ImageListMapOnly_f( void ); // leilei - stuff hack
void	R_ImageList_f( void );
void	R_InitImages( void );
void    R_InitSkins( void );
void    R_SetColorMappings( void );
void    R_GammaCorrect(unsigned char *buffer, int bufSize);
void	R_InitFogTable( void );
float	R_FogFactor( float s, float t );



/////////////////////////// tr_bsp.c ////////////////////////////////////
void        RE_LoadWorldMap( const char *mapname );
void        RE_SetWorldVisData( const byte *vis );
void        R_IssuePendingRenderCommands( void );
void        R_InitBSP(void);
qboolean	R_GetEntityToken( char *buffer, int size );



/////////////////////////// tr_model.c //////////////////////////////////
void		R_ModelInit(void);
void		R_ModelBounds( qhandle_t handle, vec3_t mins, vec3_t maxs );
void		R_Modellist_f(void);
model_t	*   R_AllocModel(void);
model_t	*   R_GetModelByHandle( qhandle_t hModel );
int			R_LerpTag( orientation_t *tag, qhandle_t handle, int startFrame, int endFrame, float frac, const char *tagName );
qhandle_t	RE_RegisterModel( const char *name );



//////////////////////////// tr_shade.c ////////////////////////////////
void RB_BeginSurface(shader_t *shader, int fogNum );
void RB_StageIteratorGeneric( void );
void RB_StageIteratorVertexLitTexture( void );
void RB_StageIteratorLightmappedMultitexture( void );
void RB_EndSurface(void);
void R_InitShade(void);


//////////////////////////// tr_sky.c /////////////////////////////////
void R_InitSkyTexCoords( float cloudLayerHeight );
void R_InitCloudAndSky(void);
void RB_DrawSun( float scale, shader_t *shader );
void RB_StageIteratorSky( void );


//////////////////////////// tr_flares.c ///////////////////////////////


void RB_AddFlare(srfFlare_t *surface, int fogNum, vec3_t point, vec3_t color, vec3_t normal, int radii,  int efftype, float scaled, int type);
void RB_AddDlightFlares( void );
void RB_RenderFlares (void);

void RB_SurfaceFlare(srfFlare_t *surf);

void R_InitFlares( void );



//////////////////////////// tr_scene /////////////////////////////////
//  SCENE GENERATION

void R_InitScene( void );

void RE_ClearScene( void );
void RE_AddRefEntityToScene( const refEntity_t *ent );
void RE_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int num );
void RE_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void RE_RenderScene( const refdef_t *fd );
void R_AddPolygonSurfaces( void );
void R_InitNextFrame( void );

void RE_ClearScene( void );
void RE_AddRefEntityToScene( const refEntity_t *ent );
void RE_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int num );
void RE_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void RE_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
void RE_RenderScene( const refdef_t *fd );



//////////////////////////// tr_surface //////////////////////////////

void R_InitSurface(void);
void RB_AddQuadStamp( vec3_t origin, vec3_t left, vec3_t up, byte *color );
void RB_AddQuadStampExt( vec3_t origin, vec3_t left, vec3_t up, byte *color, float s1, float t1, float s2, float t2 );
void RB_CheckOverflow( int verts, int indexes );
void RB_MDRSurfaceAnim( mdrSurface_t *surface );
extern void (*rb_surfaceTable[SF_NUM_SURFACE_TYPES])(void *);



/////////////////////////// tr_curve.c //////////////////////////////////
// CURVE TESSELATION
// max dimensions of a patch mesh in map file
#define	MAX_PATCH_SIZE		32
srfGridMesh_t *R_SubdividePatchToGrid( int width, int height, drawVert_t points[MAX_PATCH_SIZE*MAX_PATCH_SIZE] );
srfGridMesh_t *R_GridInsertColumn( srfGridMesh_t *grid, int column, int row, vec3_t point, float loderror );
srfGridMesh_t *R_GridInsertRow( srfGridMesh_t *grid, int row, int column, vec3_t point, float loderror );
void R_FreeSurfaceGridMesh( srfGridMesh_t *grid );
void R_InitCurve(void);


///////////////////////////// tr_light.c ////////////////////////////////
// LIGHTS
void R_DlightBmodel( bmodel_t *bmodel );
void R_SetupEntityLighting( const trRefdef_t *refdef, trRefEntity_t *ent );
void R_TransformDlights( int count, dlight_t *dl, orientationr_t *or );
int R_LightForsmaptexturePoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
int R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );

void R_InitDLight(void);


/////////////////////////// tr_model_iqm.c ////////////////////////////////////////
qboolean R_LoadIQM (model_t *mod, void *buffer, int filesize, const char *name );
void R_AddIQMSurfaces( trRefEntity_t *ent );
void RB_IQMSurfaceAnim( surfaceType_t *surface );
int R_IQMLerpTag( orientation_t *tag, iqmData_t *data, int startFrame, int endFrame, float frac, const char *tagName );


///////////////////////////// tr_SHADOWS  ////////////////////////////////
//
void RB_ShadowTessEnd( void );
void RB_ProjectionShadowDeform( void );


/////////////////////////////// tr_shader.c /////////////////////////////

#define GLS_SRCBLEND_ZERO					0x00000001
#define GLS_SRCBLEND_ONE					0x00000002
#define GLS_SRCBLEND_DST_COLOR				0x00000003
#define GLS_SRCBLEND_ONE_MINUS_DST_COLOR	0x00000004
#define GLS_SRCBLEND_SRC_ALPHA				0x00000005
#define GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA	0x00000006
#define GLS_SRCBLEND_DST_ALPHA				0x00000007
#define GLS_SRCBLEND_ONE_MINUS_DST_ALPHA	0x00000008
#define GLS_SRCBLEND_ALPHA_SATURATE			0x00000009
#define	GLS_SRCBLEND_BITS					0x0000000f

#define GLS_DSTBLEND_ZERO					0x00000010
#define GLS_DSTBLEND_ONE					0x00000020
#define GLS_DSTBLEND_SRC_COLOR				0x00000030
#define GLS_DSTBLEND_ONE_MINUS_SRC_COLOR	0x00000040
#define GLS_DSTBLEND_SRC_ALPHA				0x00000050
#define GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA	0x00000060
#define GLS_DSTBLEND_DST_ALPHA				0x00000070
#define GLS_DSTBLEND_ONE_MINUS_DST_ALPHA	0x00000080
#define	GLS_DSTBLEND_BITS					0x000000f0

#define GLS_DEPTHMASK_TRUE					0x00000100

#define GLS_POLYMODE_LINE					0x00001000

#define GLS_DEPTHTEST_DISABLE				0x00010000
#define GLS_DEPTHFUNC_EQUAL					0x00020000

#define GLS_ATEST_GT_0						0x10000000
#define GLS_ATEST_LT_80						0x20000000
#define GLS_ATEST_GE_80						0x40000000
#define	GLS_ATEST_BITS						0x70000000

#define GLS_DEFAULT			                GLS_DEPTHMASK_TRUE
shader_t*   R_FindShader( const char *name, int lightmapIndex, qboolean mipRawImage );
shader_t*   R_GetShaderByHandle( qhandle_t hShader );
shader_t*   R_FindShaderByName( const char *name );
void		R_InitShaders( void );
void		R_ShaderList_f( void );
void        R_RemapShader(const char *oldShader, const char *newShader, const char *timeOffset);

qhandle_t RE_RegisterShaderLightMap( const char *name, int lightmapIndex );
qhandle_t RE_RegisterShader( const char *name );
qhandle_t RE_RegisterShaderNoMip( const char *name );
qhandle_t RE_RegisterShaderFromImage(const char *name, int lightmapIndex, image_t *image, qboolean mipRawImage);


void stripExtension(const char *in, char *out, int destsize);

////////////////////////////////  tr_marks.c ////////////////////////////////////////
//      MARKERS, POLYGON PROJECTION ON WORLD POLYGONS
int R_MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection,  int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer );
void R_InitMarks(void);

/////////////////////////  tr_shade_calc  //////////////////////////////////////////
void	RB_DeformTessGeometry( void );


void	RB_CalcEnvironmentTexCoordsNew( float *dstTexCoords );
void	RB_CalcEnvironmentTexCoordsHW(void);
void	RB_CalcFogTexCoords( float *dstTexCoords );
void	RB_CalcScrollTexCoords( const float scroll[2], float *dstTexCoords );
void	RB_CalcRotateTexCoords( float rotSpeed, float *dstTexCoords );
void	RB_CalcScaleTexCoords( const float scale[2], float *dstTexCoords );
void	RB_CalcTurbulentTexCoords( const waveForm_t *wf, float *dstTexCoords );
void	RB_CalcTransformTexCoords( const texModInfo_t *tmi, float *dstTexCoords );
void	RB_CalcModulateColorsByFog( unsigned char (*dstColors)[4] );
void	RB_CalcModulateAlphasByFog( unsigned char (*dstColors)[4] );
void	RB_CalcModulateRGBAsByFog( unsigned char (*dstColors)[4] );
void	RB_CalcWaveAlpha( const waveForm_t *wf, unsigned char (*dstColors)[4] );
void	RB_CalcWaveColor( const waveForm_t *wf, unsigned int *dstColors );
void	RB_CalcStretchTexCoords( const waveForm_t *wf, float *texCoords );
void	RB_CalcLightscaleTexCoords( float *texCoords );
void	RB_CalcSpecularAlpha( unsigned char *alphas );
void    RB_CalcEnvironmentTexCoordsJO(float *st);

///////////////////////////// tr_shared.c  //////////////////////////////
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
void ByteToDir( int b, vec3_t dir );
void AxisClear( vec3_t axis[3] );
char *SkipPath(char *pathname);
void stripExtension(const char *in, char *out, int destsize);
const char* getExtension( const char *name );



//void GLimp_InitExtraExtensions(void);
/////////////////////////// rendercommon.h //////////////////////////////

//tr_noise
float R_NoiseGet4f( float x, float y, float z, float t );
void  R_NoiseInit( void );


//tr_font
void R_InitFreeType( void );
void R_DoneFreeType( void );
void RE_RegisterFont(const char *fontName, int pointSize, fontInfo_t *font);

//tr_image_jpg.c
void RE_SaveJPG(char * filename, int quality, int image_width, int image_height, unsigned char *image_buffer, int padding);
size_t RE_SaveJPGToBuffer(unsigned char *buffer, size_t bufSize, int quality, int image_width, int image_height, unsigned char* image_buffer, int padding);


// IMAGE LOADERS
void R_LoadBMP( const char *name, byte **pic, int *width, int *height );
void R_LoadJPG( const char *name, byte **pic, int *width, int *height );
void R_LoadPCX( const char *name, byte **pic, int *width, int *height );
void R_LoadPNG( const char *name, byte **pic, int *width, int *height );
void R_LoadTGA( const char *name, byte **pic, int *width, int *height );

#endif //TR_LOCAL_H
