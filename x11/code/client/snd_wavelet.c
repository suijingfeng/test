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

#include "snd_local.h"

#define C0 0.4829629131445341
#define C1 0.8365163037378079
#define C2 0.2241438680420134
#define C3 -0.1294095225512604

static void daub4(float* b, unsigned long n, int isign)
{
	float wksp[4097] = { 0.0f };

	unsigned long i,j;

	if (n < 4)
		return;
	
	unsigned long nh = n >> 1;
	unsigned long nh1 = nh + 1;
	if (isign >= 0)
	{
		for (i=1,j=0;j<n-3;j+=2,i++)
		{
			wksp[i]	   = C0*b[j]+C1*b[j+1]+C2*b[j+2]+C3*b[j+3];
			wksp[i+nh] = C3*b[j]-C2*b[j+1]+C1*b[j+2]-C0*b[j+3];
		}
		wksp[i   ] = C0*b[n-2]+C1*b[n-1]+C2*b[0]+C3*b[1];
		wksp[i+nh] = C3*b[n-2]-C2*b[n-1]+C1*b[0]-C0*b[1];
	}
	else
	{
		wksp[1] = C2*b[nh-1]+C1*b[n-1]+C0*b[0]+C3*b[nh1-1];
		wksp[2] = C3*b[nh-1]-C0*b[n-1]+C1*b[0]-C2*b[nh1-1];
		
		for (i=0,j=3;i<nh-1;i++)
		{
			wksp[j++] = C2*b[i]+C1*b[i+nh]+C0*b[i+1]+C3*b[i+nh1];
			wksp[j++] = C3*b[i]-C0*b[i+nh]+C1*b[i+1]-C2*b[i+nh1];
		}
	}
	
	for (i=0; i<n; i++)
		b[i] = wksp[i+1];

}

static void wt1(float* a, unsigned long n, int isign)
{
	unsigned long nn;
	int inverseStartLength = n/4;
	if (n < inverseStartLength) return;
	if (isign >= 0) {
		for (nn=n;nn>=inverseStartLength;nn>>=1)
			daub4(a,nn,isign);
	}
	else
	{
		for (nn=inverseStartLength;nn<=n;nn<<=1)
			daub4(a,nn,isign);
	}
}

/* The number of bits required by each value */
static unsigned char numBits[] = {
   0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
   6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
};

byte MuLawEncode(short s) {
	unsigned long adjusted;
	byte sign, exponent, mantissa;

	sign = (s<0)?0:0x80;

	if (s<0) s=-s;
	adjusted = (long)s << (16-sizeof(short)*8);
	adjusted += 128L + 4L;
	if (adjusted > 32767) adjusted = 32767;
	exponent = numBits[(adjusted>>7)&0xff] - 1;
	mantissa = (adjusted>>(exponent+3))&0xf;
	return ~(sign | (exponent<<4) | mantissa);
}

short MuLawDecode(byte uLaw) {
	signed long adjusted;
	byte exponent, mantissa;

	uLaw = ~uLaw;
	exponent = (uLaw>>4) & 0x7;
	mantissa = (uLaw&0xf) + 16;
	adjusted = (mantissa << (exponent +3)) - 128 - 4;

	return (uLaw & 0x80)? adjusted : -adjusted;
}

short mulawToShort[256];
static qboolean madeTable = qfalse;

static	int	NXStreamCount;

void NXPutc(NXStream *stream, char out) {
	stream[NXStreamCount++] = out;
}


void encodeWavelet( sfx_t *sfx, short *packets) {
	float	wksp[4097] = {0}, temp;
	int		i, samples, size;
	sndBuffer		*newchunk, *chunk;
	byte			*out;

	if (!madeTable) {
		for (i=0;i<256;i++) {
			mulawToShort[i] = (float)MuLawDecode((byte)i);
		}
		madeTable = qtrue;
	}
	chunk = NULL;

	samples = sfx->soundLength;
	while(samples>0) {
		size = samples;
		if (size>(SND_CHUNK_SIZE*2)) {
			size = (SND_CHUNK_SIZE*2);
		}

		if (size<4) {
			size = 4;
		}

		newchunk = SND_malloc();
		if (sfx->soundData == NULL) {
			sfx->soundData = newchunk;
		} else if (chunk != NULL) {
			chunk->next = newchunk;
		}
		chunk = newchunk;
		for(i=0; i<size; i++) {
			wksp[i] = *packets;
			packets++;
		}
		wt1(wksp, size, 1);
		out = (byte *)chunk->sndChunk;

		for(i=0;i<size;i++) {
			temp = wksp[i];
			if (temp > 32767) temp = 32767; else if (temp<-32768) temp = -32768;
			out[i] = MuLawEncode((short)temp);
		}

		chunk->size = size;
		samples -= size;
	}
}

void decodeWavelet(sndBuffer *chunk, short *to) {
	float			wksp[4097] = {0};
	int				i;
	byte			*out;

	int size = chunk->size;
	
	out = (byte *)chunk->sndChunk;
	for(i=0;i<size;i++) {
		wksp[i] = mulawToShort[out[i]];
	}

	wt1(wksp, size, -1);
	
	if (!to) return;

	for(i=0; i<size; i++) {
		to[i] = wksp[i];
	}
}


void encodeMuLaw( sfx_t *sfx, short *packets) {
	int		i, samples, size, grade, poop;
	sndBuffer		*newchunk, *chunk;
	byte			*out;

	if (!madeTable) {
		for (i=0;i<256;i++) {
			mulawToShort[i] = (float)MuLawDecode((byte)i);
		}
		madeTable = qtrue;
	}

	chunk = NULL;
	samples = sfx->soundLength;
	grade = 0;

	while(samples>0) {
		size = samples;
		if (size>(SND_CHUNK_SIZE*2)) {
			size = (SND_CHUNK_SIZE*2);
		}

		newchunk = SND_malloc();
		if (sfx->soundData == NULL) {
			sfx->soundData = newchunk;
		} else if (chunk != NULL) {
			chunk->next = newchunk;
		}
		chunk = newchunk;
		out = (byte *)chunk->sndChunk;
		for(i=0; i<size; i++) {
			poop = packets[0]+grade;
			if (poop>32767) {
				poop = 32767;
			} else if (poop<-32768) {
				poop = -32768;
			}
			out[i] = MuLawEncode((short)poop);
			grade = poop - mulawToShort[out[i]];
			packets++;
		}
		chunk->size = size;
		samples -= size;
	}
}

void decodeMuLaw(sndBuffer *chunk, short *to) {
	int				i;
	byte			*out;

	int size = chunk->size;
	
	out = (byte *)chunk->sndChunk;
	for(i=0;i<size;i++) {
		to[i] = mulawToShort[out[i]];
	}
}


