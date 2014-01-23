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

#define INFO "convert between lattice specifications"
static const char *longInfo =
  (INFO ".\n "
   "The output lattice definition is printed to stdout.");

int
rva_convMain(int argc, const char **argv, const char *me,
             hestParm *hparm) {
  hestOpt *hopt = NULL;
  char *err;
  airArray *mop;

  rvaLattSpec *lspA, *lspB;
  int lattB, fold, reo, ret;
  char buff[AIR_STRLEN_LARGE];

  mop = airMopNew();
  hopt = NULL;
  hestOptAdd(&hopt, NULL, "from latt", airTypeOther, 1, 1, &lspA, NULL,
             "lattice to convert from",
             NULL, NULL, rvaHestLattSpec);
  hestOptAdd(&hopt, NULL, "to meth", airTypeEnum, 1, 1, &lattB, NULL,
             "lattice definition method to convert to",
             NULL, rvaLatt);
  hestOptAdd(&hopt, "fold", NULL, airTypeInt, 0, 0, &fold, NULL,
             "apply folding to get to canonical representation, "
             "up to orientation (which is preserved without \"-reo\")");
  hestOptAdd(&hopt, "reo", NULL, airTypeInt, 0, 0, &reo, NULL,
             "if folding (with \"-fold\"), also reorient, to really "
             "get to canonical representation");
  hestParseOrDie(hopt, argc, argv, hparm,
                 me, longInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  lspB = rvaLattSpecNew();
  airMopAdd(mop, lspB, (airMopper)rvaLattSpecNix, airMopAlways);
  if (!fold) {
    ret = rvaLattSpecConvert(lspB, lattB, lspA);
  } else {
    rvaLattSpec *AABB;
    unsigned int count[RVA_FOLD_NUM];
    AABB = rvaLattSpecNew();
    airMopAdd(mop, AABB, (airMopper)rvaLattSpecNix, airMopAlways);
    ret = (rvaLattSpecConvert(AABB, rvaLattAB, lspA) ||
           (rvaVecsFold(count, AABB->parm + 0, AABB->parm + 2,
                        reo, AIR_FALSE), 0) ||
           rvaLattSpecConvert(lspB, lattB, AABB));
  }
  if (ret) {
    airMopAdd(mop, err = biffGetDone(RVA), airFree, airMopAlways);
    fprintf(stderr, "%s: error converting:\n%s", me, err);
    airMopError(mop); return 1;
  }
  rvaLattSpecSprint(buff, lspB);
  printf("%s\n", buff);

  airMopOkay(mop);
  return 0;
}

unrrduCmd rva_convCmd = { "conv", INFO, rva_convMain, AIR_FALSE };
