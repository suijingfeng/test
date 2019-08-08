#ifndef TR_BACKEND_H_
#define TR_BACKEND_H_

#include "viewParms.h"
#include "trRefDef.h"
#include <stdint.h>

typedef struct {
	int	c_surfaces;
    int c_shaders;
    int c_vertexes;
    int c_indexes;
    int c_totalIndexes;
	int	c_dlightVertexes;
	int	c_dlightIndexes;
	int	msec;			// total msec for backend run
} backEndCounters_t;


// all state modified by the back end is seperated
// from the front end state
typedef struct {
	trRefdef_t refdef;
	viewParms_t	viewParms;
	orientationr_t or;
	backEndCounters_t pc;
	trRefEntity_t entity2D;	// currentEntity will point at this when doing 2D rendering
	trRefEntity_t*  currentEntity;

    unsigned char Color2D[4];
    qboolean projection2D;	// if qtrue, drawstretchpic doesn't need to change modes
} backEndState_t;

extern backEndState_t backEnd;

void R_SetRefFloatTime(float t);
float R_GetRefFloatTime(void);
void R_SetRefTime(int t);
uint32_t R_GetRefRdTime(void);

void R_ClearBackendState(void);
void R_PrintBackEnd_OR_f(void);
void R_UpdatePerformanceCounters(uint32_t nVerts, uint32_t nIdxes, uint32_t nPasses);
void Bepc_UpdateNumDrawSurfs(uint32_t val);

#endif
