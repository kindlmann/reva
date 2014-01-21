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

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(rva_EXPORTS) || defined(teem_EXPORTS)
#    define RVA_EXPORT extern __declspec(dllexport)
#  else
#    define RVA_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define RVA_EXPORT extern
#endif

#include <teem/air.h>
#include <teem/hest.h>
#include <teem/biff.h>
#include <teem/ell.h>
#include <teem/nrrd.h>
#include <teem/unrrdu.h>
#include <teem/pull.h>

#define RVA "rva" /* used as biff key */

#define RVA_FOLD_SWAP     0
#define RVA_FOLD_A_SUB_B  1
#define RVA_FOLD_A_ADD_B  2
#define RVA_FOLD_NUM      3

/* different ways of identifying a lattice */
enum {
  rvaLattUnknown, /* 0: unknown or not set */
  rvaLattAB,      /* 1: "ab" 4 values: 2 vectors in (x,y): (Ax,Ay) (Bx,By) */
  rvaLattPRA,     /* 2: "pra" 3 values: phase in [0,1] --> [pi/2,pi/3],
                     radius in [0,1], and area */
  rvaLattLast
};
#define RVA_LATT_MAX 2
#define RVA_LATT_PARM_NUM 4  /* has to be max of rvaLattParmNum[] */

typedef struct {
  int latt;                          /* from rvaLatt* enum */
  double parm[RVA_LATT_PARM_NUM];    /* lattice definition,
                                        as interpreted by latt */
} rvaLattSpec;

/* spec.c */
RVA_EXPORT const airEnum *const rvaLatt;
RVA_EXPORT const unsigned int rvaLattParmNum[RVA_LATT_MAX+1];
RVA_EXPORT rvaLattSpec *rvaLattSpecNew();
RVA_EXPORT rvaLattSpec *rvaLattSpecNix(rvaLattSpec *lsp);
RVA_EXPORT void rvaLattSpecCopy(rvaLattSpec *dst, const rvaLattSpec *src);
RVA_EXPORT int rvaLattSpecConvert(rvaLattSpec *dst, int latt,
                                  const rvaLattSpec *src);
RVA_EXPORT int rvaLattSpecParse(rvaLattSpec *lsp, const char *str);
RVA_EXPORT hestCB *rvaHestLattSpec;
RVA_EXPORT char *rvaLattSpecSprint(char *str, const rvaLattSpec *lsp);

/* util.c */
RVA_EXPORT void rvaVecsFold(unsigned int count[RVA_FOLD_NUM],
                            double A[2], double B[2], int verbose);
RVA_EXPORT void rvaPhiArea2Vecs(double A[2], double B[2],
                                double phi, double area);
RVA_EXPORT double rvaPsi(unsigned int *maxRadius,
                         const pullEnergySpec *espec,
                         double AA[2], double BB[2]);

/* flotsam.c */
#define RVA_DECLARE(C) RVA_EXPORT unrrduCmd rva_##C##Cmd;
#define RVA_LIST(C) &rva_##C##Cmd,
/* F(vwflip) \ */
/* F(vwfix) */
/* this is the list of per-command source files */
#define RVA_MAP(F) \
  F(about) \
  F(fold) \
  F(conv) \
  F(dia)
RVA_MAP(RVA_DECLARE)
RVA_EXPORT const unrrduCmd *const rvaCmdList[];
RVA_EXPORT int rvaMain(int argc, const char **argv,
                       const char *cmd, const char *title,
                       const unrrduCmd *const *cmdList, FILE *fusage);
