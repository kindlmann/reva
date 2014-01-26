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

/*
** NULL-terminated array of unrrduCmd pointers, as ordered by
** RVA_MAP macro
*/
#define RVA_LIST(C) &rva_##C##Cmd,
static const unrrduCmd *const
cmdList[] = {
  RVA_MAP(RVA_LIST)
  NULL
};

int
main(int argc, const char **argv) {
  hestParm *hparm;
  airArray *mop;
  int ret;

  mop = airMopNew();
  hparm = hestParmNew();
  /*
  hparm->elideSingleEnumType = AIR_TRUE;
  hparm->elideSingleOtherType = AIR_TRUE;
  hparm->elideSingleOtherDefault = AIR_FALSE;
  hparm->elideSingleNonExistFloatDefault = AIR_TRUE;
  hparm->elideMultipleNonExistFloatDefault = AIR_TRUE;
  hparm->elideSingleEmptyStringDefault = AIR_TRUE;
  hparm->elideMultipleEmptyStringDefault = AIR_TRUE;
  */
  hparm->cleverPluralizeOtherY = AIR_TRUE;
  hparm->columns = 78;
  airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
  ret = unrrduCmdMain(argc, argv,
                      "reva", "Brevais Lattice Hacking",
                      cmdList, hparm, stderr);
  airMopOkay(mop);
  return ret;
}
