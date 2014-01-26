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

#define INFO "make EPS diagram of a lattice"
static const char *longInfo =
  (INFO ".\n "
   "A current limitation is that at most a 201x201 grid will be "
   "drawn, but this is probably safely above what anyone would "
   "want to see");

typedef struct {
  FILE *file;
  const rvaLattSpec *lsp;
  double min[2], max[2], scl, rad[2];
} lpictData;

static int
lpictInit(unsigned int anum, const rvaLattSpec *lspAB, void *_data) {
  double A[2], B[2], bbox[2][2];
  char lstr[AIR_STRLEN_MED];
  lpictData *d;

  d = AIR_CAST(lpictData *, _data);
  ELL_2V_COPY(A, lspAB->parm + 0);
  ELL_2V_COPY(B, lspAB->parm + 2);
  bbox[0][0] = d->scl*d->min[0]; /* min */
  bbox[0][1] = d->scl*d->min[1];
  bbox[1][0] = d->scl*d->max[0]; /* max */
  bbox[1][1] = d->scl*d->max[1];
  fprintf(d->file, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(d->file, "%%%%Creator: reva dia\n");
  fprintf(d->file, "%%%%Title: %s\n", rvaLattSpecSprint(lstr, d->lsp));
  fprintf(d->file, "%%%%Pages: 1\n");
  fprintf(d->file, "%%%%BoundingBox: %d %d %d %d\n",
          AIR_CAST(int, floor(bbox[0][0])),
          AIR_CAST(int, floor(bbox[0][1])),
          AIR_CAST(int, ceil(bbox[1][0])),
          AIR_CAST(int, ceil(bbox[1][1])));
  fprintf(d->file, "%%%%HiResBoundingBox: %g %g %g %g\n",
          bbox[0][0], bbox[0][1], bbox[1][0], bbox[1][1]);
  fprintf(d->file, "%%%%EndComments\n");
  fprintf(d->file, "%%%%BeginProlog\n");
  fprintf(d->file, "%%%%EndProlog\n");
  fprintf(d->file, "%%%%Page: 1 1\n");
  fprintf(d->file, "gsave\n");

  fprintf(d->file, "%g %g moveto\n", bbox[0][0], bbox[0][1]);
  fprintf(d->file, "%g %g lineto\n", bbox[1][0], bbox[0][1]);
  fprintf(d->file, "%g %g lineto\n", bbox[1][0], bbox[1][1]);
  fprintf(d->file, "%g %g lineto\n", bbox[0][0], bbox[1][1]);
  fprintf(d->file, "closepath\n");
  fprintf(d->file, "clip\n");
  fprintf(d->file, "newpath\n");

  if (d->rad[0] == d->rad[1]) {
    fprintf(d->file, "0.5 setgray\n");
    fprintf(d->file, "%g setlinewidth\n", d->rad[0]);
    fprintf(d->file, "0 0 moveto\n");
    fprintf(d->file, "%g %g lineto\n", d->scl*A[0], d->scl*A[1]);
    fprintf(d->file, "stroke\n");
    fprintf(d->file, "0 0 moveto\n");
    fprintf(d->file, "%g %g lineto\n", d->scl*B[0], d->scl*B[1]);
    fprintf(d->file, "stroke\n");
  } else {
    fprintf(d->file, "1 setgray\n");
    fprintf(d->file, "%g setlinewidth\n", d->rad[0]);
    fprintf(d->file, "0 0 moveto\n");
    fprintf(d->file, "%g %g lineto\n", d->scl*A[0], d->scl*A[1]);
    fprintf(d->file, "stroke\n");
    fprintf(d->file, "0 0 moveto\n");
    fprintf(d->file, "%g %g lineto\n", d->scl*B[0], d->scl*B[1]);
    fprintf(d->file, "stroke\n");
    fprintf(d->file, "0.5 setgray\n");
    fprintf(d->file, "%g setlinewidth\n", d->rad[1]);
    fprintf(d->file, "0 0 moveto\n");
    fprintf(d->file, "%g %g lineto\n", d->scl*A[0], d->scl*A[1]);
    fprintf(d->file, "stroke\n");
    fprintf(d->file, "0 0 moveto\n");
    fprintf(d->file, "%g %g lineto\n", d->scl*B[0], d->scl*B[1]);
    fprintf(d->file, "stroke\n");
  }
  if (d->rad[0] == d->rad[1]) {
    fprintf(d->file, "0 setgray\n");
  }
  return 0;
}

static int
lpictPoint(const double xy[2], const rvaLattSpec *lspAB, void *_data) {
  lpictData *d;

  AIR_UNUSED(lspAB);
  d = AIR_CAST(lpictData *, _data);
  if (!( AIR_IN_OP(d->min[0] - 4*d->rad[0], xy[0],
                   d->max[0] + 4*d->rad[0]) &&
         AIR_IN_OP(d->min[1] - 4*d->rad[0], xy[1],
                   d->max[1] + 4*d->rad[0]) )) {
    /* nothing to draw */
    return 0;
  }
  if (d->rad[0] == d->rad[1]) {
    fprintf(d->file, "%g %g %g 0 360 arc closepath\n",
            d->scl*xy[0], d->scl*xy[1], d->rad[0]);
    fprintf(d->file, "fill\n");
  } else {
    fprintf(d->file, "%g %g %g 0 360 arc closepath\n",
            d->scl*xy[0], d->scl*xy[1], d->rad[0]);
    fprintf(d->file, "1 setgray fill\n");
    fprintf(d->file, "%g %g %g 0 360 arc closepath\n",
            d->scl*xy[0], d->scl*xy[1], d->rad[1]);
    fprintf(d->file, "0 setgray fill\n");
  }

  return 0;
}

static int
lpictDone(void *_data) {
  lpictData *d;

  d = AIR_CAST(lpictData *, _data);
  fprintf(d->file, "grestore\n");
  return 0;
}

static int
lpict(FILE *file, const rvaLattSpec *lsp,
      double min[2], double max[2], double scl,
      double rad[2]) {
  static const char me[]="lpict";
  lpictData data;
  double xy[2], rd, rr;

  data.file = file;
  data.lsp = lsp;
  data.scl = scl;
  ELL_2V_COPY(data.min, min);
  ELL_2V_COPY(data.max, max);
  ELL_2V_COPY(data.rad, rad);

  rd = 0;
  ELL_2V_SET(xy, min[0], min[1]); rr = ELL_2V_LEN(xy); rd = AIR_MAX(rr, rd);
  ELL_2V_SET(xy, min[0], max[1]); rr = ELL_2V_LEN(xy); rd = AIR_MAX(rr, rd);
  ELL_2V_SET(xy, max[0], min[1]); rr = ELL_2V_LEN(xy); rd = AIR_MAX(rr, rd);
  ELL_2V_SET(xy, max[0], max[1]); rr = ELL_2V_LEN(xy); rd = AIR_MAX(rr, rd);
  if (rvaForEach(lsp, rd, AIR_TRUE /* includeZero */,
                 lpictInit, lpictPoint, lpictDone, &data)) {
    biffAddf(RVA, "%s: problem", me);
    return 1;
  }

  return 0;
}

int
rva_diaMain(int argc, const char **argv, const char *me,
            hestParm *hparm) {
  hestOpt *hopt = NULL;
  airArray *mop;

  FILE *fout;
  double min[2], max[2], scl, rad[2];
  rvaLattSpec *lsp;
  char *outStr;

  mop = airMopNew();
  hopt = NULL;
  hestOptAdd(&hopt, NULL, "latt", airTypeOther, 1, 1, &lsp, NULL,
             "lattice definition", NULL, NULL, rvaHestLattSpec);
  hestOptAdd(&hopt, "min", "minX minY", airTypeDouble, 2, 2, min, "0 0",
             "lower left corner");
  hestOptAdd(&hopt, "max", "maxX maxY", airTypeDouble, 2, 2, max, "2 2",
             "upper right corner");
  hestOptAdd(&hopt, "scl", "scale", airTypeDouble, 1, 1, &scl, "100",
             "scaling to PostScript coordinates");
  hestOptAdd(&hopt, "rad", "radius", airTypeDouble, 2, 2, &rad, "2 2",
             "outer (white) and inner (black) radius of dots");
  hestOptAdd(&hopt, "o", "fname", airTypeString, 1, 1, &outStr, NULL,
             "output filename");
  hestParseOrDie(hopt, argc, argv, hparm,
                 me, longInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (!(fout = airFopen(outStr, stdout, "wb"))) {
    fprintf(stderr, "%s: couldn't open \"%s\" for writing\n", me, outStr);
    airMopError(mop); return 1;
  }
  airMopAdd(mop, fout, (airMopper)airFclose, airMopAlways);

  lpict(fout, lsp, min, max, scl, rad);

  airMopOkay(mop);
  return 0;
}

unrrduCmd rva_diaCmd = { "dia", INFO, rva_diaMain, AIR_FALSE };
