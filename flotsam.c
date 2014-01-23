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
******** rvaCmdList[]
**
** NULL-terminated array of unrrduCmd pointers, as ordered by
** RVA_MAP macro
*/
const unrrduCmd *const
rvaCmdList[] = {
  RVA_MAP(RVA_LIST)
  NULL
};

/*
** the "main" function for the reva command, which is like
** teem/src/limn/test/lpu.c:main(), which is like
** teem/src/bin/tend.c
**
** If it is generalized right, this may find its way into the unrrdu library.
** A sneaky but basic issue is the const-correctness of how the hestParm
** is used; we'd like to take a const hestParm* to communicate parameters
** the caller has set, but the show-stopper is that unrrduCmd->main()
** takes a non-const hestParm, and it has to be that way, because some
** unu commands alter the given hparm (which probably shouldn't happen).
** Until that's fixed, we have a non-const hestParm* coming in here.
*/
int
rvaMain(int argc, const char **argv,
        const char *cmd, const char *title,
        const unrrduCmd *const *cmdList,
        hestParm *_hparm, FILE *fusage) {
  int i, ret;
  const char *me;
  char *argv0 = NULL;
  hestParm *hparm;
  airArray *mop;

  me = argv[0];

  /* parse environment variables first, in case they break nrrdDefault*
     or nrrdState* variables in a way that nrrdSanity() should see */
  nrrdDefaultGetenv();
  nrrdStateGetenv();

  nrrdSanityOrDie(me);

  mop = airMopNew();
  if (_hparm) {
    hparm = _hparm;
  } else {
    hparm = hestParmNew();
    airMopAdd(mop, hparm, (airMopper)hestParmFree, airMopAlways);
    hparm->elideSingleEnumType = AIR_TRUE;
    hparm->elideSingleOtherType = AIR_TRUE;
    hparm->elideSingleOtherDefault = AIR_FALSE;
    hparm->elideSingleNonExistFloatDefault = AIR_TRUE;
    hparm->elideMultipleNonExistFloatDefault = AIR_TRUE;
    hparm->elideSingleEmptyStringDefault = AIR_TRUE;
    hparm->elideMultipleEmptyStringDefault = AIR_TRUE;
    hparm->cleverPluralizeOtherY = AIR_TRUE;
    /* this would be the place to learn the actual number of columns
       in the current terminal */
    hparm->columns = 78;
  }

  /* if there are no arguments, then we give general usage information */
  if (1 >= argc) {
    unsigned int ii, maxlen = 0;
    char *buff, *fmt, tdash[] = "--- %s ---";
    for (ii=0; cmdList[ii]; ii++) {
      maxlen = AIR_MAX(maxlen, AIR_UINT(strlen(cmdList[ii]->name)));
    }
    buff = AIR_CALLOC(strlen(tdash) + strlen(title) + 1, char);
    airMopAdd(mop, buff, airFree, airMopAlways);
    sprintf(buff, tdash, title);
    fmt = AIR_CALLOC(hparm->columns + strlen(buff) + 1, char); /* generous */
    airMopAdd(mop, buff, airFree, airMopAlways);
    sprintf(fmt, "%%%us\n",
            AIR_UINT((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
    fprintf(fusage, fmt, buff);

    for (ii=0; cmdList[ii]; ii++) {
      unsigned int cc, len;
      len = AIR_UINT(strlen(cmdList[ii]->name));
      strcpy(buff, "");
      for (cc=len; cc<maxlen; cc++)
        strcat(buff, " ");
      strcat(buff, cmd);
      strcat(buff, " ");
      strcat(buff, cmdList[ii]->name);
      strcat(buff, " ... ");
      len = strlen(buff);
      fprintf(fusage, "%s", buff);
      _hestPrintStr(fusage, len, len, hparm->columns,
                    cmdList[ii]->info, AIR_FALSE);
    }
    airMopError(mop);
    return 1;
  }
  /* else, we should see if they're asking for a command we know about */
  for (i=0; cmdList[i]; i++) {
    if (!strcmp(argv[1], cmdList[i]->name)) {
      break;
    }
  }
  if (cmdList[i]) {
    /* yes, we have that command */
    /* initialize variables used by the various commands */
    argv0 = AIR_CALLOC(strlen(cmd) + strlen(argv[1]) + 2, char);

    airMopMem(mop, &argv0, airMopAlways);
    sprintf(argv0, "%s %s", cmd, argv[1]);

    /* run the individual command, saving its exit status */
    ret = cmdList[i]->main(argc-2, argv+2, argv0, hparm);
  } else {
    fprintf(stderr, "%s: unrecognized command: \"%s\"; type \"%s\" for "
            "complete list\n", me, argv[1], me);
    ret = 1;
  }

  airMopDone(mop, ret);
  return ret;
}
