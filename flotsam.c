
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
** If it is generalized right, this may find its way into the unrrdu library.
** One issue is whether the hparm struct should be passed here
** versus trusting all the settings below.
*/
int
rvaMain(int argc, const char **argv,
        const char *cmd, const char *title,
        const unrrduCmd *const *cmdList, FILE *fusage) {
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
