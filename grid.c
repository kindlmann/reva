/*
  rva: experiments in computing in the space of Brevais Lattices
  Copyright (C) 2014  University of Chicago

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must
     not claim that you wrote the original software. If you use this
     software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/

#include "rva.h"
#include "rvaPrivate.h"

/*
** Allows you to do something for each point in a lattice, for those
** points within some radius.  Operates with two callbacks. If the
** callbacks use biff, they should use the RVA key
*/
int
rvaForEach(const rvaLattSpec *lsp, double radius,
           int includeZero,
           int (*initCB)(unsigned int num,
                        const rvaLattSpec *lspAB,
                        void *data),
           int (*pointCB)(const double xy[2],
                          const rvaLattSpec *lspAB,
                          void *data),
           int (*doneCB)(void *data),
           void *dataCB) {
  static const char me[]="rvaForEachInRadius";
  rvaLattSpec *lspAB;
  airArray *mop;
  double AA[2], BB[2], rsqd, area;
  unsigned int approxNum;

  if (!(lsp)) {
    biffAddf(RVA, "%s: got NULL pointer", me);
    return 1;
  }
  if (!(radius > 0)) {
    biffAddf(RVA, "%s: radius %g not > 0", me, radius);
    return 1;
  }
  mop = airMopNew();
  lspAB = rvaLattSpecNew();
  airMopAdd(mop, lspAB, (airMopper)rvaLattSpecNix, airMopAlways);
  if (rvaLattSpecConvert(lspAB, rvaLattAB, lsp)) {
    biffAddf(RVA, "%s: couldn't convert latt to %s", me,
             airEnumStr(rvaLatt, rvaLattAB));
    airMopError(mop); return 1;
  }
  ELL_2V_COPY(AA, lspAB->parm + 0);
  ELL_2V_COPY(BB, lspAB->parm + 2);
  rsqd = radius*radius;
  area = ELL_2V_CROSS(AA, BB);
  area = AIR_ABS(area);
  if (!area) {
    biffAddf(RVA, "%s: lattice unit has zero area", me);
    airMopError(mop); return 1;
  }
  approxNum = AIR_ROUNDUP_UI(AIR_PI*rsqd/area);
  if (initCB && initCB(approxNum, lspAB, dataCB)) {
    biffAddf(RVA, "%s: initCB(%u) error", me, approxNum);
    airMopError(mop); return 1;
  }
  if (pointCB) {
    unsigned int tried, out;
    int ri;
    double XY[2];
    if (includeZero) {
      ELL_2V_SET(XY, 0, 0);
      if (pointCB(XY, lspAB, dataCB)) {
        biffAddf(RVA, "%s pointCB((0,0)) error", me);
        airMopError(mop); return 1;
      }
    }
    ri = 0;
    do {
      int ai, bi;
      ri++;
      tried = out = 0;
#define PNT_PROC(AI, BI)                                            \
      ELL_2V_SCALE_ADD2(XY, (AI), AA, (BI), BB);                    \
      tried++;                                                      \
      if (LNSQ(XY) < rsqd) {                                        \
        if (pointCB(XY, lspAB, dataCB)) {                           \
          biffAddf(RVA, "%s pointCB(%d*A + %d*B = (%g,%g)) error",  \
                   me, AI, BI, XY[0], XY[1]);                       \
          airMopError(mop); return 1;                               \
        }                                                           \
      } else {                                                      \
        out++;                                                      \
      }
      for (ai=-ri; ai<=ri; ai++) {
        PNT_PROC(ai, -ri);
        PNT_PROC(ai, ri);
      }
      for (bi=-ri+1; bi<=ri-1; bi++) {
        PNT_PROC(-ri, bi);
        PNT_PROC(ri, bi);
      }
#undef PNT_PROC
    } while (out < tried);
  }

  if (doneCB && doneCB(dataCB)) {
    biffAddf(RVA, "%s: doneCB() error", me);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

typedef struct {
  airArray *mop;
  airArray *ptsArr;
  double *pts;
  unsigned int ptsNum;
} gridData;

static int
gridInit(unsigned int anum, const rvaLattSpec *lsp, void *_data) {
  static const char me[]="gridInit";
  gridData *data;
  unsigned int bigIncr = 100000;
  airPtrPtrUnion appu;
  int ret;

  AIR_UNUSED(lsp);
  if (anum > bigIncr) {
    biffAddf(RVA, "%s: approxNum %u > %u", me, anum, bigIncr);
    return 1;
  }
  data = AIR_CAST(gridData*, _data);
  appu.d = &(data->pts);
  data->ptsArr = airArrayNew(appu.v, &(data->ptsNum),
                             2*sizeof(double), anum);
  airMopAdd(data->mop, data->ptsArr,
            (airMopper)airArrayNix /* NOT Nuke, since we will wrap
                                      this data in a Nrrd */,
            airMopAlways);
  return 0;
}

static int
gridPoint(const double xy[2], const rvaLattSpec *lsp, void *_data) {
  static const char me[]="gridPoint";
  gridData *data;
  unsigned int pi;

  AIR_UNUSED(lsp);
  data = AIR_CAST(gridData*, _data);
  pi = airArrayLenIncr(data->ptsArr, 1);
  ELL_2V_COPY(data->pts + 2*pi, xy);

  return 0;
}

int
rvaGrid(Nrrd *nout, const rvaLattSpec *lsp, double radius) {
  static const char me[]="rvaGrid";
  airArray *mop;
  airArray *ptsArr;
  double AA[2], BB[2], *pts, rsqd;
  unsigned int ptsNum, incr, ptsIdx;
  gridData data;

  data.mop = airMopNew();
  if (rvaForEach(lsp, radius, AIR_TRUE /* includeZero */,
                 gridInit, gridPoint, NULL, &data)) {
    biffAddf(RVA, "%s: problem", me);
    airMopError(data.mop); return 1;
  }

  if (nrrdWrap_va(nout, data.pts, nrrdTypeDouble, 2,
                  AIR_CAST(size_t, 2),
                  AIR_CAST(size_t, data.ptsNum))) {
    biffMovef(RVA, NRRD, "%s: trouble wrapping data", me);
    airMopError(data.mop); return 1;
  }

  airMopOkay(data.mop);
  return 0;
}
