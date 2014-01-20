
#include "rva.h"

#define INFO "Information about this program and its use"

int
rva_aboutMain(int argc, const char **argv, const char *me,
              hestParm *hparm) {
  char buff[AIR_STRLEN_LARGE], fmt[AIR_STRLEN_MED];
  char par1[] = "\t\t\t\t"
    "\"reva\" is very much under construction.\n";
  char par2[] = "\t\t\t\t"
    "Users should remain cautious.\n";

  AIR_UNUSED(argc);
  AIR_UNUSED(argv);
  AIR_UNUSED(me);

  fprintf(stderr, "\n");
  sprintf(buff, "--- reva: Brevais Lattice Hacking ---");
  sprintf(fmt, "%%%ds\n",
          (int)((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
  fprintf(stderr, fmt, buff);
  airTeemVersionSprint(buff);
  sprintf(fmt, "%%%ds\n",
          (int)((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
  fprintf(stderr, fmt, buff);
  fprintf(stderr, "\n");

  _hestPrintStr(stderr, 1, 0, 78, par1, AIR_FALSE);
  _hestPrintStr(stderr, 1, 0, 78, par2, AIR_FALSE);

  return 0;
}

unrrduCmd rva_aboutCmd = { "about", INFO, rva_aboutMain, AIR_FALSE };
