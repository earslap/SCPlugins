/*
 SuperCollider real time audio synthesis system
 Copyright (c) 2002 James McCartney. All rights reserved.
 http://www.audiosynth.com
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 
 */

//PV_UGens by Batuhan Bozkurt http://www.batuhanbozkurt.com

//#include "SC_PlugIn.h"
//#include "SCComplex.h"
#include "FFT_UGens.h"

#define PV_GET_BUF2_FCOMPARE \
	float fbufnum1 = ZIN0(0); \
	float fbufnum2 = ZIN0(1); \
	if (fbufnum1 < 0.f || fbufnum2 < 0.f) { ZOUT0(0) = unit->outVal; return; } \
	ZOUT0(0) = unit->outVal; \
	uint32 ibufnum1 = (int)fbufnum1; \
	uint32 ibufnum2 = (int)fbufnum2; \
	World *world = unit->mWorld; \
	SndBuf *buf1; \
	SndBuf *buf2; \
	if (ibufnum1 >= world->mNumSndBufs) { \
		int localBufNum = ibufnum1 - world->mNumSndBufs; \
		Graph *parent = unit->mParent; \
		if(localBufNum <= parent->localBufNum) { \
			buf1 = parent->mLocalSndBufs + localBufNum; \
		} else { \
			buf1 = world->mSndBufs; \
		} \
	} else { \
		buf1 = world->mSndBufs + ibufnum1; \
	} \
	if (ibufnum2 >= world->mNumSndBufs) { \
		int localBufNum = ibufnum2 - world->mNumSndBufs; \
		Graph *parent = unit->mParent; \
		if(localBufNum <= parent->localBufNum) { \
			buf2 = parent->mLocalSndBufs + localBufNum; \
		} else { \
			buf2 = world->mSndBufs; \
		} \
	} else { \
		buf2 = world->mSndBufs + ibufnum2; \
	} \
	if (buf1->samples != buf2->samples) return; \
	int numbins = buf1->samples - 2 >> 1;

InterfaceTable *ft;

struct FrameCompare : PV_Unit
{
	float outVal;
};

extern "C"
{
	void load(InterfaceTable *inTable);
	
	void FrameCompare_Ctor(FrameCompare* unit);
	void FrameCompare_Dtor(FrameCompare* unit);
	void FrameCompare_next(FrameCompare* unit, int inNumSamples);
}

SCPolarBuf* ToPolarApx(SndBuf *buf)
{
	if (buf->coord == coord_Complex) {
		SCComplexBuf* p = (SCComplexBuf*)buf->data;
		int numbins = buf->samples - 2 >> 1;
		for (int i=0; i<numbins; ++i) {
			p->bin[i].ToPolarApxInPlace();
		}
		buf->coord = coord_Polar;
	}
	return (SCPolarBuf*)buf->data;
}

SCComplexBuf* ToComplexApx(SndBuf *buf)
{
	if (buf->coord == coord_Polar) {
		SCPolarBuf* p = (SCPolarBuf*)buf->data;
		int numbins = buf->samples - 2 >> 1;
		for (int i=0; i<numbins; ++i) {
			p->bin[i].ToComplexApxInPlace();
		}
		buf->coord = coord_Complex;
	}
	return (SCComplexBuf*)buf->data;
}

//FrameCompare

void FrameCompare_Ctor(FrameCompare* unit)
{
	
	SETCALC(FrameCompare_next);
	ZOUT0(0) = ZIN0(0);
}

void FrameCompare_next(FrameCompare *unit, int inNumSamples)
{
	PV_GET_BUF2_FCOMPARE
	
	float sum = 0;
	
	SCPolarBuf *p1 = ToPolarApx(buf1);
	SCPolarBuf *p2 = ToPolarApx(buf2);
	
	for(int i = 0; i < numbins; ++i)
	{
		
		sum = sum + fabs(p1->bin[i].mag - p2->bin[i].mag);
		//Print("1: %f, 2: %f\n", p1->bin[i].mag, p2->bin[i].mag);
	}
	
	ZOUT0(0) = unit->outVal = sum;
	//Print("hmm: %f\n", sum);
}

void FrameCompare_Dtor(FrameCompare* unit)
{
}

void init_SCComplex(InterfaceTable *inTable);

void load(InterfaceTable *inTable)
{
	ft = inTable;
	
	init_SCComplex(inTable);
	
	DefineDtorUnit(FrameCompare);	
}