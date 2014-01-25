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

#define INFO "generate coordinates for grid points inside radius"
static const char *longInfo =
  (INFO ".\n "
   "Points are ordered in squares of increasing size.");

int
rva_gridMain(int argc, const char **argv, const char *me,
             hestParm *hparm) {
  hestOpt *hopt = NULL;
  char *err;
  airArray *mop;

  rvaLattSpec *lsp;
  double radius;
  char *outStr;
  Nrrd *nout;

  mop = airMopNew();
  hopt = NULL;
  hestOptAdd(&hopt, NULL, "latt", airTypeOther, 1, 1, &lsp, NULL,
             "lattice definition", NULL, NULL, rvaHestLattSpec);
  hestOptAdd(&hopt, "r", "radius", airTypeDouble, 1, 1, &radius, "1",
             "radius limit");
  hestOptAdd(&hopt, "o", "fname", airTypeString, 1, 1, &outStr, "-",
             "output filename");
  hestParseOrDie(hopt, argc, argv, hparm,
                 me, longInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  if (rvaGrid(nout, lsp, radius)) {
    airMopAdd(mop, err=biffGetDone(RVA), airFree, airMopAlways);
    fprintf(stderr, "%s: problem:\n%s", me, err);
    airMopError(mop); return 1;
  }

  if (nrrdSave(outStr, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem saving:\n%s", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

unrrduCmd rva_gridCmd = { "grid", INFO, rva_gridMain, AIR_FALSE };
