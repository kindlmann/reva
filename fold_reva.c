
#include "rva.h"

#define INFO "fold given vector(s) into unique Brevais space"
static const char *longInfo =
  (INFO ".\n "
   "* Uses nrrdCrop");

int
rva_foldMain(int argc, const char **argv, const char *me,
             hestParm *hparm) {
  hestOpt *hopt = NULL;
  char *err;
  airArray *mop;

  double A[2], B[2], S[2], min[2], max[2], xx, yy;
  Nrrd *_nin, *nin, *nout;
  char *outStr;
  unsigned int xi, yi, sx, sy, fxi, fyi, count[3];
  float *fin, *fout;
  int back, color, scaleDo, zeroFold, verbose;

  mop = airMopNew();
  hopt = NULL;
  hestOptAdd(&hopt, "v", "verb", airTypeInt, 1, 1, &verbose, "0",
             "verbosity");
  hestOptAdd(&hopt, "i", "nin", airTypeOther, 1, 1, &_nin, "",
             "input image", NULL, NULL, nrrdHestNrrd);
  hestOptAdd(&hopt, "back", NULL, airTypeInt, 0, 0, &back, NULL,
             "backwards mapping");
  hestOptAdd(&hopt, "s", "x y", airTypeDouble, 2, 2, S, "nan nan",
             "single vector to fold w/ (1,0); "
             "overrides previous options");
  hestOptAdd(&hopt, "zf", "bool", airTypeInt, 0, 0, &zeroFold, NULL,
             "only see when there is zero folding");
  hestOptAdd(&hopt, "scl", NULL, airTypeInt, 0, 0, &scaleDo, NULL,
             "do intensity scaling to visualize cell area");
  hestOptAdd(&hopt, "o", "fname", airTypeString, 1, 1, &outStr, "-",
             "output filename");
  hestParseOrDie(hopt, argc, argv, hparm,
                 me, longInfo, AIR_TRUE, AIR_TRUE, AIR_TRUE);
  airMopAdd(mop, hopt, (airMopper)hestOptFree, airMopAlways);
  airMopAdd(mop, hopt, (airMopper)hestParseFree, airMopAlways);

  if (ELL_2V_EXISTS(S)) {
    /* special case; just do folding of single vector */
    double scl;
    A[0] = 1.0;
    A[1] = 0.0;
    fprintf(stderr, "----- pre-fold\n");
    fprintf(stderr, "A = (%g,%g)  |%g|\n", A[0], A[1], ELL_2V_LEN(A));
    fprintf(stderr, "S = (%g,%g)  |%g|\n", S[0], S[1], ELL_2V_LEN(S));
    ELL_3V_SET(count, 0, 0, 0);
    rvaVecsFold(count, A, S, verbose);
    fprintf(stderr, "----- post-fold\n");
    fprintf(stderr, "A = (%g,%g)  |%g|\n", A[0], A[1], ELL_2V_LEN(A));
    fprintf(stderr, "S = (%g,%g)  |%g|\n", S[0], S[1], ELL_2V_LEN(S));
    fprintf(stderr, "counts: swap:%u  A-=B:%u  A+=B:%u\n",
            count[0], count[1], count[2]);
    fprintf(stderr, "----- post-fold and post-normalization\n");
    scl = 1/A[0];
    ELL_2V_SCALE(A, scl, A);
    ELL_2V_SCALE(S, scl, S);
    fprintf(stderr, "A = (%g,%g)  |%g|\n", A[0], A[1], ELL_2V_LEN(A));
    fprintf(stderr, "S = (%g,%g)  |%g|\n", S[0], S[1], ELL_2V_LEN(S));
    airMopOkay(mop);
    exit(0);
  }

  if (!( 2 == _nin->dim
         || (3 == _nin->dim && 3 == _nin->axis[0].size) )) {
    fprintf(stderr, "%s: need a 2-D image or 3-D RGB image\n", me);
    airMopError(mop); return 1;
  }
  color = (3 == _nin->axis[0].size);
  min[0] = _nin->axis[color + 0].min;
  max[0] = _nin->axis[color + 0].max;
  min[1] = _nin->axis[color + 1].min;
  max[1] = _nin->axis[color + 1].max;
  if (!( AIR_EXISTS(min[0]) && AIR_EXISTS(max[0]) &&
         AIR_EXISTS(min[1]) && AIR_EXISTS(max[1]) )) {
    fprintf(stderr, "%s: didn't get min/max on axes %u,%u of input\n", me,
            color+0, color+1);
    airMopError(mop); return 1;
  }

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);
  nin = nrrdNew();
  airMopAdd(mop, nin, (airMopper)nrrdNuke, airMopAlways);
  if (nrrdConvert(nout, _nin, nrrdTypeFloat)
      || nrrdConvert(nin, _nin, nrrdTypeFloat)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem allocating arrays:\n%s", me, err);
    airMopError(mop); return 1;
  }
  nrrdZeroSet(nout);

  sx = nin->axis[color + 0].size;
  sy = nin->axis[color + 1].size;
  fin = AIR_CAST(float *, nin->data);
  fout = AIR_CAST(float *, nout->data);
  for (yi=0; yi<sy; yi++) {
    yy = AIR_AFFINE(-0.5, yi, sy-0.5, min[1], max[1]);
    for (xi=0; xi<sx; xi++) {
      double oldA[2], oldB[2], scl;
      xx = AIR_AFFINE(-0.5, xi, sx-0.5, min[0], max[0]);
      A[0] = 1.0;
      A[1] = 0.0;
      B[0] = xx;
      B[1] = yy;
      ELL_2V_COPY(oldA, A);
      ELL_2V_COPY(oldB, B);
      ELL_3V_SET(count, 0, 0, 0);
      rvaVecsFold(count, A, B, verbose);
      if (zeroFold) {
        if (count[0] || count[1] || count[2]) {
          if (color) {
            ELL_3V_SET(fout + 3*(xi + sx*yi), AIR_NAN, AIR_NAN, AIR_NAN);
          } else {
            fout[xi + sx*yi] = AIR_NAN;
          }
          continue;
        }
      }
      if (1) {
        fxi = airIndexClamp(min[0], B[0]/A[0], max[0], sx);
        fyi = airIndexClamp(min[1], B[1]/A[0], max[1], sy);
        /*
        fprintf(stderr, "fxi %u = indexclamp(%g, %g/%g = %g, %g, %u)\n",
                fxi, min[0], B[0], A[0], B[0]/A[0], max[0], sx);
        fprintf(stderr, "fyi %u = indexclamp(%g, %g/%g = %g, %g, %u)\n",
                fyi, min[1], B[1], A[0], B[1]/A[0], max[1], sy);
        */
      } else {
        fxi = airIndexClamp(min[0], B[0], max[0], sx);
        fyi = airIndexClamp(min[1], B[1], max[1], sy);
      }
      if (verbose) {
        fprintf(stderr, "(%u,%u) --> (%u,%u)\n", xi, yi, fxi, fyi);
        fprintf(stderr, "(%g,%g),(%g,%g) --> (%g,%g),(%g,%g)\n",
                oldA[0], oldA[1], oldB[0], oldB[1], A[0], A[1], B[0], B[1]);
      }
      if (scaleDo) {
        scl = A[0]/1.46;  /* where did this constant come from? */
      } else {
        scl = 1.0;
      }
      if (back) {
        if (color) {
          ELL_3V_SCALE(fout + 3*(xi + sx*yi), scl, fin + 3*(fxi + sx*fyi));
        } else {
          fout[xi + sx*yi] = scl*fin[fxi + sx*fyi];
        }
      } else {
        if (color) {
          ELL_3V_INCR(fout + 3*(xi + sx*yi), fin + 3*(fxi + sx*fyi));
        } else {
          fout[fxi + sx*fyi] += fin[xi + sx*yi];
        }
      }
    }
  }

  if (nrrdSave(outStr, nout, NULL)) {
    airMopAdd(mop, err=biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: problem saving:\n%s", me, err);
    airMopError(mop); return 1;
  }

  airMopOkay(mop);
  return 0;
}

unrrduCmd rva_foldCmd = { "fold", INFO, rva_foldMain, AIR_FALSE };
