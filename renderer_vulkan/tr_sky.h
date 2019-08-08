#ifndef TR_SKY_H_
#define TR_SKY_H_
#include "R_ShaderCommands.h"
#include "tr_image.h"


typedef struct {
	float       cloudHeight;
	struct image_s *   outerbox[6];
    struct image_s *   innerbox[6];
} skyParms_t;

struct shaderCommands_s;

void R_InitSkyTexCoords( float cloudLayerHeight );
void RB_StageIteratorSky( struct shaderCommands_s * const pTess );
#endif
