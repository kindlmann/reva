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
