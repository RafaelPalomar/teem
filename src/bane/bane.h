/*
  teem: Gordon Kindlmann's research software
  Copyright (C) 2002, 2001, 2000, 1999, 1998 University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifdef __cplusplus
extern "C" {
#endif


#ifndef BANE_HAS_BEEN_INCLUDED
#define BANE_HAS_BEEN_INCLUDED

#define BANE "bane"

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include <air.h>
#include <biff.h>
#include <nrrd.h>
#include <gage.h>

/*
** The idea is that the baneRange, baneInc, baneClip, and baneMeasr,
** structs, and the pointers to them declared below, are glorified
** constants.  All the parameters to these things (the various double
** *xxxParm arrays) are kept in things which are dynamically allocated
** by the user.
*/

/* -------------------------- ranges -------------------------- */

/*
******** baneRange..._e enum
**
** Range: nature of the values generated by a measure- are they
** strictly positive (such as gradient magnitude), should they be
** considered to be centered around zero (2nd directional
** derivative) or could they be anywhere (data
** value).
**
** The job of the ans() function in the range is not to exclude
** any data.  Indeed, if the range is set correctly for the type
** of data used, then range->ans() should always return a range
** that is as large or larger than the one which was passed.  
** Doing otherwise would make ranges too complicated (such as
** requiring a parm array), and besides, its the job of the
** inclusion methods to be smart about things like this.
*/
enum {
  baneRangeUnknown=-1, /* -1: nobody knows */
  baneRangePos_e,      /*  0: always positive: enforce that min == 0 */
  baneRangeNeg_e,      /*  1: always negative: enforce that max == 0 */
  baneRangeZeroCent_e, /*  2: positive and negative, centered around zero:
			      enforce (conservative) centering of interval around 0 */
  baneRangeFloat_e,    /*  3: anywhere: essentially a no-op */
  baneRangeLast_e
};
#define BANE_RANGE_MAX     3
/*
******** baneRange struct
**
** things used to operate on ranges
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];
  int which;
  void (*ans)(double *ominP, double *omaxP,
	      double imin, double imax);
} baneRange;

/* -------------------------- inc -------------------------- */

/*
******** baneInc..._e enum
**
** Inc: methods for determing what range of measured values deserves
** to be included along one axes of a histogram volume.  Each
** inclusion method has some parameters (at most BANE_INC_NUM_PARM)
** which are (or can be harmlessly cast to) floats.  Some of them need
** a histogram (a Nrrd) in order to determine the new min and max,
** some just use a Nrrd as a place to store some information.
**
** To make matters confusing, the behavior of some of these varies with
** the baneRange they are associated with... 
*/
enum {
  baneIncUnknown_e=-1,   /* -1: nobody knows */
  baneIncAbsolute_e,     /*  0: within explicitly specified bounds */
  baneIncRangeRatio_e,   /*  1: some fraction of the total range */
  baneIncPercentile_e,   /*  2: exclude some percentile */
  baneIncStdv_e,         /*  3: some multiple of the standard deviation */
  baneIncLast_e
};
#define BANE_INC_MAX         3
#define BANE_INC_PARM_NUM 3
/*
******** baneInc struct
**
** things used to calculate and describe inclusion ranges.  The return
** from histNew should be eventually passed to nrrdNuke.
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];
  int which;
  int numParm;           /* assumed length of incParm in this ans() */
  Nrrd *(*histNew)(double *incParm);
  void (*passA)(Nrrd *hist, double val, double *incParm);
  void (*passB)(Nrrd *hist, double val, double *incParm);
  void (*ans)(double *minP, double *maxP,
	      Nrrd *hist, double *incParm,
	      baneRange *range);
} baneInc;

/* -------------------------- clip -------------------------- */

/*
******** baneClip..._e enum
**
** Clip: how to map values in the "raw" histogram volume to the more
** convenient 8-bit version.  The number of hits for the semi-constant
** background of a large volume can be huge, so some scheme for dealing
** with this is needed.
*/
enum {
  baneClipUnknown_e=-1, /* -1: nobody knows */
  baneClipAbsolute_e,    /* 0: clip at explicitly specified bin count */
  baneClipPeakRatio_e,   /* 1: some fraction of maximum #hits in any bin */
  baneClipPercentile_e,  /* 2: percentile of values, sorted by hits */
  baneClipTopN_e,        /* 3: ignore the N bins with the highest counts */
  baneClipLast
};
#define BANE_CLIP_MAX       3
#define BANE_CLIP_PARM_NUM 1
/*
******** baneClip struct
**
** things used to calculate and describe clipping
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];
  int which;
  int numParm;           /* assumed length of clipParm in this ans() */
  int (*ans)(Nrrd *hvol, double *clipParm);
} baneClip;

/* -------------------------- measr -------------------------- */

/*
******** baneMeasr..._e enum
**
** Measr: one of the kind of measurement which determines location along
** one of the axes of the histogram volume.
**
** In this latest version of bane, I nixed the "gradient of magnitude
** of gradient" (GMG) based measure of the second directional
** derivative.  I felt that the benefits of using gage for value and
** derivative measurement (allowing arbitrary kernels), combined with
** the fact that doing GMG can't be done directly in gage (because its
** a derivative of pre-computed derivatives), outweighed the loss of
** GMG.  Besides, according to Appendix C of my Master's thesis the
** only thing its really good at avoiding is quantization noise in
** 8-bit data, but gage isn't limited to 8-bit data anyway.
**
** Eventually I'll use the parameters to the "ans" method in order to
** do something useful...
*/
enum {
  baneMeasrUnknown_e=-1, /* -1: nobody knows */
  baneMeasrVal_e,        /*  0: the data value */
  baneMeasrGradMag_e,    /*  1: gradient magnitude */
  baneMeasrLapl_e,       /*  2: Laplacian */
  baneMeasrHess_e,       /*  3: Hessian-based measure of 2nd DD along
                                gradient */
  baneMeasrCurvedness_e, /*  4: L2 norm of K1, K2 principal curvatures
			       (gageSclCurvedness) */
  baneMeasrShapeTrace_e, /*  5: shape indicator (gageSclShapeTrace) */
  baneMeasrLast
};
#define BANE_MEASR_MAX       5
#define BANE_MEASR_PARM_NUM 1
/*
******** baneMeasr struct
**
** things used to calculate and describe measurements
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];
  int which;
  int numParm;           /* assumed length of measrParm in this ans() */
  baneRange *range;
  float (*ans)(gageSclAnswer *, double *measrParm);
} baneMeasr;

/* -------------------- histogram volumes, etc. ---------------------- */

/*
******** baneAxis struct
** 
** Information for how to do measurement and inclusion along each axis
** of the histogram volume.
**
** No dynamically allocated stuff in here
*/
typedef struct {
  int res;                            /* resolution = number of bins */
  baneMeasr *measr;
  double measrParm[BANE_MEASR_PARM_NUM];
  baneInc *inc;
  double incParm[BANE_INC_PARM_NUM];
} baneAxis;

/*
******** baneHVolParm struct
** 
** Information for how to create a histogram volume
**
*/
typedef struct {
  int verbose;                         /* status messages to stderr */
  baneAxis *ax[3];
  NrrdKernel *k[GAGE_KERNEL_NUM];
  double kparm[GAGE_KERNEL_NUM][NRRD_KERNEL_PARMS_NUM];
  baneClip *clip;
  double clipParm[BANE_CLIP_PARM_NUM];
  double incLimit;                     /* lowest permissible fraction of the
					  data remaining after new inclusion
					  has been determined */
} baneHVolParm;

/* defaults.c */
extern float baneDefIncLimit;
extern int baneDefHistEqBins;

/* range.c */
extern baneRange *baneRangePos;
extern baneRange *baneRangeNeg;
extern baneRange *baneRangeZeroCent;
extern baneRange *baneRangeFloat;
extern baneRange *baneRangeArray[BANE_RANGE_MAX+1]; 

/* inc.c */
extern baneInc *baneIncAbsolute;
extern baneInc *baneIncRangeRatio;
extern baneInc *baneIncPercentile;
extern baneInc *baneIncStdv;
extern baneInc *baneIncArray[BANE_INC_MAX+1];

/* measr.c */
extern baneMeasr *baneMeasrVal;
extern baneMeasr *baneMeasrGradMag;
extern baneMeasr *baneMeasrLapl;
extern baneMeasr *baneMeasrHess;
extern baneMeasr *baneMeasrCurvedness;
extern baneMeasr *baneMeasrShadeTrace;
extern baneMeasr *baneMeasrArray[BANE_MEASR_MAX+1];

/* clip.c */
extern baneClip *baneClipAbsolute;
extern baneClip *baneClipPeakRatio;
extern baneClip *baneClipPercentile;
extern baneClip *baneClipTopN;
extern baneClip *baneClipArray[BANE_CLIP_MAX+1];

/* methods.c */
extern baneHVolParm *baneHVolParmNew();
extern baneHVolParm *baneHVolParmNix(baneHVolParm *hvp);
extern void baneHVolParmGKMSInit(baneHVolParm *hvp);

/* hvol.c */
extern int baneMakeHVol(Nrrd *hvol, Nrrd *nin, baneHVolParm *hvp);
extern int baneApplyMeasr(Nrrd *nout, Nrrd *nin, baneMeasr *measr,
		  NrrdKernel *k[GAGE_KERNEL_NUM],
		  double kparm[GAGE_KERNEL_NUM][NRRD_KERNEL_PARMS_NUM]);
extern Nrrd *baneGKMSHVol(Nrrd *nin, float perc);

/* valid.c */
extern int baneValidHVol(Nrrd *hvol);
extern int baneValidInfo(Nrrd *info2D, int wantDim);
extern int baneValidPos(Nrrd *pos, int wantDim);
extern int baneValidBcpts(Nrrd *Bcpts);

/* trnsf.c */
extern int baneOpacInfo(Nrrd *info, Nrrd *hvol, int dim, int measr);
extern int bane1DOpacInfoFrom2D(Nrrd *info1D, Nrrd *info2D);
extern int baneSigmaCalc(float *sP, Nrrd *info);
extern int banePosCalc(Nrrd *pos, float sigma, float gthresh, Nrrd *info);
extern void _baneOpacCalcA(int lutLen, float *opacLut, 
			   int numCpts, float *xo,
			   float *pos);
extern void _baneOpacCalcB(int lutLen, float *opacLut, 
			   int numCpts, float *x, float *o,
			   float *pos);
extern int baneOpacCalc(Nrrd *opac, Nrrd *Bcpts, Nrrd *pos);

/* trex.c */
extern float *_baneTRexRead(char *fname);
extern void _baneTRexDone();

/* scat.c */
extern int baneRawScatterplots(Nrrd *nvg, Nrrd *nvh, Nrrd *hvol, int histEq);

#endif /* BANE_HAS_BEEN_INCLUDED */

#ifdef __cplusplus
}
#endif
