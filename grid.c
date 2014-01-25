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

int
rvaGrid(Nrrd *nout, const rvaLattSpec *_lsp, double radius) {
  static const char me[]="rvaGrid";
  airArray *mop;
  rvaLattSpec *lsp;
  airArray *ptsArr;
  double AA[2], BB[2], *pts, rsqd;
  unsigned int ptsNum, incr, bigIncr = 100000, ptsIdx;
  int ri, ai, bi, done;

  mop = airMopNew();
  lsp = rvaLattSpecNew();
  airMopAdd(mop, lsp, (airMopper)rvaLattSpecNix, airMopAlways);
  if (rvaLattSpecConvert(lsp, rvaLattAB, _lsp)) {
    biffAddf(RVA, "%s: couldn't convert", me);
    airMopError(mop); return 1;
  }
  ELL_2V_COPY(AA, lsp->parm + 0);
  ELL_2V_COPY(BB, lsp->parm + 2);
  rsqd = radius*radius;
  {
    double bigarea, litarea;
    bigarea = AIR_PI*radius*radius;
    litarea = ELL_2V_CROSS(AA, BB);
    incr = AIR_CAST(unsigned int, bigarea/AIR_ABS(litarea));
    if (!litarea || incr > bigIncr) {
      biffAddf(RVA, "%s: grid unit size %g too small", me, litarea);
      airMopError(mop); return 1;
    }
  }
  ptsArr = airArrayNew(&pts, &ptsNum, 2*sizeof(double), incr);
  airMopAdd(mop, ptsArr, (airMopper)airArrayNix /* NOT Nuke */,
            airMopAlways);
  ptsIdx = airArrayLenIncr(ptsArr, 1);
  ELL_2V_SET(pts + 2*ptsIdx, 0.0, 0.0);

  ri = 0;
  do {
    unsigned int tried, out;
    double XY[2];
    ri++;
    tried = out = 0;
    for (ai=-ri; ai<=ri; ai++) {
#define PNT_ADD(AI, BI)                                 \
      ELL_2V_SCALE_ADD2(XY, (AI), AA, (BI), BB);        \
      tried++;                                          \
      if (LNSQ(XY) < rsqd) {                            \
        ptsIdx = airArrayLenIncr(ptsArr, 1);            \
        ELL_2V_SET(pts + 2*ptsIdx, XY[0], XY[1]);       \
      } else {                                          \
        out++;                                          \
      }
      PNT_ADD(ai, -ri);
      PNT_ADD(ai, ri);
    }
    for (bi=-ri+1; bi<=ri-1; bi++) {
      PNT_ADD(-ri, bi);
      PNT_ADD(ri, bi);
    }
#undef PNT_ADD
    done = (tried == out);
  } while (!done);
  if (nrrdWrap_va(nout, ptsArr->data, nrrdTypeDouble, 2,
                  AIR_CAST(size_t, 2),
                  AIR_CAST(size_t, ptsNum))) {
    biffMovef(RVA, NRRD, "%s: trouble wrapping data", me);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}
