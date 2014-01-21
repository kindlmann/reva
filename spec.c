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

/* HEY: have to keep in synch with RVA_LATT_PARM_NUM */
const unsigned int rvaLattParmNum[RVA_LATT_MAX+1] = {
  0, /* unknown */
  4, /* ab */
  3, /* pra */
  2, /* xy */
};

const char *
rvaLattStr[RVA_LATT_MAX+1] = {
  "(unknown_lattice)",
  "ab",
  "pra",
  "xy"
};

const airEnum
_rvaLatt = {
  "lattice specification method",
  RVA_LATT_MAX,
  rvaLattStr, NULL,
  NULL,
  NULL, NULL,
  AIR_FALSE
};
const airEnum *const
rvaLatt = &_rvaLatt;

rvaLattSpec *
rvaLattSpecNew(void) {
  rvaLattSpec *lsp;
  unsigned int pi;

  lsp = AIR_CALLOC(1, rvaLattSpec);
  lsp->latt = rvaLattUnknown;
  for (pi=0; pi<RVA_LATT_PARM_NUM; pi++) {
    lsp->parm[pi] = AIR_NAN;
  }
  return lsp;
}

rvaLattSpec *
rvaLattSpecNix(rvaLattSpec *lsp) {

  if (lsp) {
    free(lsp);
  }
  return NULL;
}

void
rvaLattSpecCopy(rvaLattSpec *dst, const rvaLattSpec *src) {
  unsigned int pi;

  dst->latt = src->latt;
  for (pi=0; pi<RVA_LATT_PARM_NUM; pi++) {
    dst->parm[pi] = src->parm[pi];
  }
  return;
}

int
rvaLattSpecParse(rvaLattSpec *lsp, const char *_str) {
  static const char me[]="rvaLattSpecParse";
  char *col, *pstr, *str;
  airArray *mop;
  int smeth;
  unsigned got;

  if (!(lsp && _str)) {
    biffAddf(RVA, "%s: got NULL pointer", me);
    return 1;
  }
  if (!( str = airStrdup(_str) )) {
    biffAddf(RVA, "%s: couldn't strdup", me);
    return 1;
  }
  mop = airMopNew();
  airMopAdd(mop, str, airFree, airMopAlways);

  col = strchr(str, ':');
  if (!col) {
    biffAddf(RVA, "%s: didn't see \":\" separator in \"%s\" between "
             "lattice specification method and parameters", me, _str);
    airMopError(mop); return 1;
  }
  *col = '\0';
  lsp->latt = airEnumVal(rvaLatt, str);
  if (rvaLattUnknown == lsp->latt) {
    biffAddf(RVA, "%s: didn't recognize \"%s\" as a %s", me,
             str, rvaLatt->name);
    airMopError(mop); return 1;
  }
  pstr = col + 1;
  got = airParseStrD(lsp->parm, pstr, ",", rvaLattParmNum[lsp->latt]);
  if (got != rvaLattParmNum[lsp->latt]) {
    biffAddf(RVA, "%s: got %u (not %u) parms in \"%s\"", me,
             got, rvaLattParmNum[lsp->latt], _str);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

int
_rvaHestLattSpecParse(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  static const char me[]="_rvaHestLattSpecParse";
  rvaLattSpec **lspP;
  char *perr;

  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  lspP = (rvaLattSpec **)ptr;
  *lspP = rvaLattSpecNew();
  if (rvaLattSpecParse(*lspP, str)) {
    perr = biffGetDone(RVA);
    airStrcpy(err, AIR_STRLEN_HUGE, perr);
    free(perr);
    return 1;
  }
  return 0;
}

hestCB
_rvaHestLattSpec = {
  sizeof(rvaLattSpec*),
  "lattice specification",
  _rvaHestLattSpecParse,
  (airMopper)rvaLattSpecNix
};

hestCB *
rvaHestLattSpec = &_rvaHestLattSpec;

char *
rvaLattSpecSprint(char *str, const rvaLattSpec *lsp) {
  char buff[AIR_STRLEN_SMALL];
  unsigned int pi;

  sprintf(str, "%s:", airEnumStr(rvaLatt, lsp->latt));
  for (pi=0; pi<rvaLattParmNum[lsp->latt]; pi++) {
    sprintf(buff, "%.17g", lsp->parm[pi]);
    if (pi) {
      strcat(str, ",");
    }
    strcat(str, buff);
  }
  return str;
}

static void
getToPosY(double AA[2], double BB[2]) {
  double theta, rot[4];

  if (LNSQ(AA) < LNSQ(BB)) {
    _rvaSwap2(AA, BB);
  }
  theta = atan2(AA[1], AA[0]);
  ELL_2M_ROTATE_SET(rot, -theta);
  _rvaRot2(AA, rot, AA);
  _rvaRot2(BB, rot, BB);
  if (BB[1] < 0) {
    ELL_2V_SCALE(BB, -1, BB);
  }
}

static int
lattConv(int dstLatt, double *dstParm,
         int srcLatt, const double *srcParm) {
  double AA[2], BB[2], TT[2], theta, phase, radi, area, len,
    tparm[RVA_LATT_PARM_NUM];
  int ret;

  ret = 0;
  switch(srcLatt) {
  case rvaLattAB:
    switch(dstLatt) {
    case rvaLattPRA:   /* AB -> PRA (loss off orientation) */
      ELL_2V_COPY(AA, srcParm + 0);
      ELL_2V_COPY(BB, srcParm + 2);
      getToPosY(AA, BB);
      theta = atan2(BB[1], BB[0]);
      phase = AIR_AFFINE(AIR_PI/2, theta, AIR_PI/3, 0.0, 1.0);
      radi = _rvaLen2(BB)/_rvaLen2(AA);
      area = _rvaLen2(AA)*BB[1];
      ELL_3V_SET(dstParm, phase, radi, area);
      break;
    case rvaLattXY:   /* AB -> XY (loss of orientation and scale) */
      getToPosY(AA, BB);
      len = _rvaLen2(AA);
      ELL_2V_SET(dstParm, BB[0]/len, BB[1]/len);
      break;
    default: ret = 1; break;
    }
    break;
  case rvaLattPRA:
    switch(dstLatt) {
      double scl, uar;
    case rvaLattAB: /* PRA -> AB */
      area = AIR_ABS(srcParm[2]);
      theta = AIR_AFFINE(0, srcParm[0], 1, AIR_PI/2, AIR_PI/3);
      radi = srcParm[1];
      ELL_2V_SET(AA, 1.0, 0.0);
      ELL_2V_SET(BB, radi*cos(theta), radi*sin(theta));
      /* area from AA and BB is BB[1], but need to scale
         these to get to requested area */
      scl = sqrt(area/BB[1]);
      ELL_4V_SET(dstParm, scl*AA[0], scl*AA[1], scl*BB[0], scl*BB[1]);
      break;
    case rvaLattXY: /* PRA -> XY: PRA -> AB -> XY */
      if (lattConv(rvaLattAB,   tparm, rvaLattPRA, srcParm) ||
          lattConv(rvaLattXY, dstParm, rvaLattAB, srcParm)) {
        ret = 1; break;
      }
      break;
    default: ret = 1; break;
    }
    break;
  case rvaLattXY:
    switch(dstLatt) {
    case rvaLattAB:  /* XY -> AB */
      ELL_4V_SET(dstParm, 1.0, 0.0, srcParm[0], srcParm[1]);
      break;
    case rvaLattPRA: /* XY -> PRA: XY -> AB -> PRA */
      if (lattConv(rvaLattAB,    tparm, rvaLattXY, srcParm) ||
          lattConv(rvaLattPRA, dstParm, rvaLattAB, srcParm)) {
        ret = 1; break;
      }
      break;
    default: ret = 1; break;
    }
    break;
  default: ret = 1; break;
  }
  return ret;
}

int
rvaLattSpecConvert(rvaLattSpec *dst, int latt, const rvaLattSpec *src) {
  static const char me[]="rvaLattSpecConvert";
  int nocando;

  if (!(dst && src)) {
    biffAddf(RVA, "%s: got NULL pointer", me);
    return 1;
  }
  if (airEnumValCheck(rvaLatt, latt)) {
    biffAddf(RVA, "%s: %d not valid %s", me, latt, rvaLatt->name);
    return 1;
  }
  if (latt == src->latt) {
    rvaLattSpecCopy(dst, src);
    return 0;
  }
  /* else have work to do */
  if (lattConv(latt, dst->parm, src->latt, src->parm)) {
    dst->latt = rvaLattUnknown;
    biffAddf(RVA, "%s: %s -> %s conversion not implemented", me,
             airEnumStr(rvaLatt, src->latt), airEnumStr(rvaLatt, latt));
    return 1;
  }
  /* else */
  dst->latt = latt;

  return 0;
}
