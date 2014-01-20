
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

#define RVA_FOLD_SWAP     0
#define RVA_FOLD_A_SUB_B  1
#define RVA_FOLD_A_ADD_B  2
#define RVA_FOLD_NUM      3

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
  F(fold)
RVA_MAP(RVA_DECLARE)
RVA_EXPORT const unrrduCmd *const rvaCmdList[];
RVA_EXPORT int rvaMain(int argc, const char **argv,
                       const char *cmd, const char *title,
                       const unrrduCmd *const *cmdList, FILE *fusage);
