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
   "");

int
lpict(FILE *file, const rvaLattSpec *lsp,
      double min[2], double max[2], double scl,
      double rad[2]) {
  rvaLattSpec *lspAB;
  double A[2], B[2], bbox[2][2];
  int ai, bi;

  lspAB = rvaLattSpecNew();
  rvaLattSpecConvert(lspAB, rvaLattAB, lsp);
  if (uni) {
    A[0] = lvec[2];
    A[1] = 0.0;
    B[0] = lvec[0]*lvec[1]*A[0];
    B[1] = lvec[1]*A[0];
  } else {
    A[0] = lvec[2];
    A[1] = 0.0;
    B[0] = lvec[0]; /* *A[0]; */
    B[1] = lvec[1]; /* *A[0]; */
  }
  bbox[0][0] = scl*min[0]; /* min */
  bbox[0][1] = scl*min[1];
  bbox[1][0] = scl*max[0]; /* max */
  bbox[1][1] = scl*max[1];
  /*
    bbox[0][0] = scl*min[0] - 2*rad;
    bbox[0][1] = scl*min[1] - 2*rad;
    bbox[1][0] = scl*max[0] + 2*rad;
    bbox[1][1] = scl*max[1] + 2*rad;
  */

  fprintf(file, "%%!PS-Adobe-3.0 EPSF-3.0\n");
  fprintf(file, "%%%%Creator: lattice picture\n");
  fprintf(file, "%%%%Title: blah blah blah\n");
  fprintf(file, "%%%%Pages: 1\n");
  fprintf(file, "%%%%BoundingBox: %d %d %d %d\n",
          AIR_CAST(int, floor(bbox[0][0])),
          AIR_CAST(int, floor(bbox[0][1])),
          AIR_CAST(int, ceil(bbox[1][0])),
          AIR_CAST(int, ceil(bbox[1][0])));
  fprintf(file, "%%%%HiResBoundingBox: %g %g %g %g\n",
          bbox[0][0], bbox[0][1], bbox[1][0], bbox[1][1]);
  fprintf(file, "%%%%EndComments\n");
  fprintf(file, "%%%%BeginProlog\n");
  fprintf(file, "%%%%EndProlog\n");
  fprintf(file, "%%%%Page: 1 1\n");
  fprintf(file, "gsave\n");


  fprintf(file, "%g %g moveto\n", bbox[0][0], bbox[0][1]);
  fprintf(file, "%g %g lineto\n", bbox[1][0], bbox[0][1]);
  fprintf(file, "%g %g lineto\n", bbox[1][0], bbox[1][1]);
  fprintf(file, "%g %g lineto\n", bbox[0][0], bbox[1][1]);
  fprintf(file, "closepath\n");
  fprintf(file, "clip\n");
  fprintf(file, "newpath\n");

  if (rad[0] == rad[1]) {
    fprintf(file, "0.5 setgray\n");
    fprintf(file, "%g setlinewidth\n", rad[0]);
    fprintf(file, "0 0 moveto\n");
    fprintf(file, "%g %g lineto\n", scl*A[0], scl*A[1]);
    fprintf(file, "stroke\n");
    fprintf(file, "0 0 moveto\n");
    fprintf(file, "%g %g lineto\n", scl*B[0], scl*B[1]);
    fprintf(file, "stroke\n");
  } else {
    fprintf(file, "1 setgray\n");
    fprintf(file, "%g setlinewidth\n", rad[0]);
    fprintf(file, "0 0 moveto\n");
    fprintf(file, "%g %g lineto\n", scl*A[0], scl*A[1]);
    fprintf(file, "stroke\n");
    fprintf(file, "0 0 moveto\n");
    fprintf(file, "%g %g lineto\n", scl*B[0], scl*B[1]);
    fprintf(file, "stroke\n");
    fprintf(file, "0.5 setgray\n");
    fprintf(file, "%g setlinewidth\n", rad[1]);
    fprintf(file, "0 0 moveto\n");
    fprintf(file, "%g %g lineto\n", scl*A[0], scl*A[1]);
    fprintf(file, "stroke\n");
    fprintf(file, "0 0 moveto\n");
    fprintf(file, "%g %g lineto\n", scl*B[0], scl*B[1]);
    fprintf(file, "stroke\n");
  }

  if (rad[0] == rad[1]) {
    fprintf(file, "0 setgray\n");
  }
  for (ai=-100; ai<100; ai++) {
    for (bi=-100; bi<100; bi++) {
      double xx, yy;
      xx = ai*A[0] + bi*B[0];
      yy = ai*A[1] + bi*B[1];
      if (!( AIR_IN_OP(min[0] - 4*rad[0], xx, max[0] + 4*rad[0]) &&
             AIR_IN_OP(min[1] - 4*rad[0], yy, max[1] + 4*rad[0]) )) {
        continue;
      }
      if (rad[0] == rad[1]) {
        fprintf(file, "%g %g %g 0 360 arc closepath\n",
                scl*xx, scl*yy, rad[0]);
        fprintf(file, "fill\n");
      } else {
        fprintf(file, "%g %g %g 0 360 arc closepath\n",
                scl*xx, scl*yy, rad[0]);
        fprintf(file, "1 setgray fill\n");
        fprintf(file, "%g %g %g 0 360 arc closepath\n",
                scl*xx, scl*yy, rad[1]);
        fprintf(file, "0 setgray fill\n");
      }
    }
  }

  fprintf(file, "grestore\n");
  rvaLattSpecNix(lspAB);
  return 0;
}

int
rva_diaMain(int argc, const char **argv, const char *me,
            hestParm *hparm) {
  hestOpt *hopt = NULL;
  airArray *mop;

  FILE *fout;
  double min[2], max[2], scl, rad[2];
  rvaLattSpec *lsp, *lsp
  char *outStr;
  int uni;

  mop = airMopNew();
  hopt = NULL;
  hestOptAdd(&hopt, "v", "u v w", airTypeDouble, 3, 3, lvec, NULL,
             "vector identifying lattice");
  hestOptAdd(&hopt, "uni", NULL, airTypeInt, 0, 0, &uni, NULL,
             "assuming parameterization of uniform lattice PDF");
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

  lpict(fout, lvec, uni, min, max, scl, rad);

  airMopOkay(mop);
  return 0;
}

unrrduCmd rva_diaCmd = { "dia", INFO, rva_diaMain, AIR_FALSE };
