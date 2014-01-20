#include "rva.h"

static void
swapVec(double A[2], double B[2]) {
  double tt;
  tt = A[0]; A[0] = B[0]; B[0] = tt;
  tt = A[1]; A[1] = B[1]; B[1] = tt;
}

#define LNSQ(vec) ((vec)[0]*(vec)[0] + (vec)[1]*(vec)[1])

static void
printVec(char *prefix, double A[2], double B[2]) {
  fprintf(stderr, "%s: A=(%g,%g) (len=%g)   B=(%g,%g) (len=%g)\n", prefix,
          A[0], A[1], ELL_2V_LEN(A), B[0], B[1], ELL_2V_LEN(B));
}

void
rvaVecsFold(unsigned int count[RVA_FOLD_NUM],
            double A[2], double B[2], int verbose) {
  double P[2], M[2], thA, thB, lenA, lenB, epsilon=1e-15;

  if (verbose > 1) {
    printVec(" * hello", A, B);
  }
  int done;
  done = AIR_FALSE;
  do {
    if (LNSQ(A) < LNSQ(B)) {
      count[RVA_FOLD_SWAP] += 1;
      swapVec(A, B);
      if (verbose > 1) {
        printVec(" A <-> B", A, B);
      }
    }
    /* so |A| >= |B| */
    ELL_2V_ADD2(P, A, B);
    ELL_2V_SUB(M, A, B);
    if (LNSQ(P) < LNSQ(A)) {
      count[RVA_FOLD_A_ADD_B] += 1;
      ELL_2V_COPY(A, P);
      if (verbose > 1) {
        printVec("  A += B", A, B);
      }
    } else if (LNSQ(M) < LNSQ(A)) {
      count[RVA_FOLD_A_SUB_B] += 1;
      ELL_2V_COPY(A, M);
      if (verbose > 1) {
        printVec("  A -= B", A, B);
      }
    } else {
      done = AIR_TRUE;
    }
    if (LNSQ(B) < epsilon) {
      /* can get in an infinite loop with two parallel
         vectors (with negative dot product), slowly
         nibbling away at each other */
      done = AIR_TRUE;
    }
  } while (!done);
  if (verbose > 1) {
    printVec("    done", A, B);
  }
  /* seems to never happen? */
  if (LNSQ(A) < LNSQ(B)) {
    fprintf(stderr, "final swap!\n");
    swapVec(A, B);
  }
  if (1) {
    double rot[4], tv[2];
    thA = atan2(A[1], A[0]);
    ELL_2M_ROTATE_SET(rot, -thA);
    /* HEY: would be nice to somehow preserve the orientation
       of the A and B that came in ... */
    ELL_2MV_MUL(tv, rot, A); ELL_2V_COPY(A, tv);
    ELL_2MV_MUL(tv, rot, B); ELL_2V_COPY(B, tv);
    if (B[1] < 0) {
      ELL_2V_SCALE(B, -1, B);
    }
  } else {
    /* old code, but WRONG: can't just take abs() of B coords */
    lenA = ELL_2V_LEN(A);
    lenB = ELL_2V_LEN(B);
    thA = atan2(A[1], A[0]);
    thB = atan2(B[1], B[0]);
    A[0] = ELL_2V_LEN(A);
    A[1] = 0.0;
    B[0] = AIR_ABS(lenB*cos(thB - thA));
    B[1] = AIR_ABS(lenB*sin(thB - thA));
  }
  if (verbose > 1) {
    printVec(" postrot", A, B);
  }
  return;
}

void
rvaPhiArea2Vecs(double A[2], double B[2], double phi, double area) {
  double len, theta;

  if (!(A && B)) {
    return;
  }
  area = AIR_ABS(area);
  theta = AIR_AFFINE(0, phi, 1, AIR_PI/2, AIR_PI/3);
  len = sqrt(area/sin(theta));
  ELL_2V_SET(A, len, 0);
  ELL_2V_SET(B, len*cos(theta), len*sin(theta));
  return;
}

double
rvaPsi(unsigned int *maxRadius, const pullEnergySpec *espec,
       double AA[2], double BB[2]) {
  /* static const char me[]="rvaPsi"; */
  double pos[2], denr, oldPsi, psi;
  const pullEnergy *enr;
  const double *eparm;
  int ai, bi, rad;

  if (!(espec && AA && BB)) {
    return AIR_NAN;
  }

  eparm = espec->parm;
  enr = espec->energy;
  /*
  fprintf(stderr, "!%s: enr(%p->energy=%p)->eval(0.5,%p(%g,%g)) = ... \n",
          me, espec, enr, eparm, eparm[0], eparm[1]);
  fprintf(stderr, "!%s: ... %g\n", me,
          enr->eval(&denr, 0.5, eparm));
  */
#define PHI(ai, bi) (ELL_2V_SCALE_ADD2(pos, ai, AA, bi, BB),  \
                     enr->eval(&denr, ELL_2V_LEN(pos), eparm))
  psi = 0.0;
  rad = 1;
  do {
    oldPsi = psi;
    for (ai=-rad; ai<=rad; ai++) {
      psi += PHI(ai, -rad);
      /* fprintf(stderr, "!%s: (ai=%d,%d) -> %g\n", me, ai, -rad, psi); */
      psi += PHI(ai, +rad);
      /* fprintf(stderr, "!%s: (ai=%d,%d) -> %g\n", me, ai, +rad, psi); */
    }
    for (bi=-rad+1; bi<=rad-1; bi++) {
      psi += PHI(-rad, bi);
      /* fprintf(stderr, "!%s: (%d,bi=%d) -> %g\n", me, -rad, bi, psi); */
      psi += PHI(+rad, bi);
      /* fprintf(stderr, "!%s: (%d,bi=%d) -> %g\n", me, +rad, bi, psi); */
    }
    rad += 1;
  } while (psi != oldPsi);
#undef PHI
  if (maxRadius) {
    *maxRadius = AIR_CAST(unsigned int, rad-1);
  }
  return psi;
}
