/*
  The contents of this file are subject to the University of Utah Public
  License (the "License"); you may not use this file except in
  compliance with the License.
  
  Software distributed under the License is distributed on an "AS IS"
  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
  the License for the specific language governing rights and limitations
  under the License.

  The Original Source Code is "teem", released March 23, 2001.
  
  The Original Source Code was developed by the University of Utah.
  Portions created by UNIVERSITY are Copyright (C) 2001, 1998 University
  of Utah. All Rights Reserved.
*/

#include "private.h"

char *padName = "pad";
char *padInfo = "Pad along each axis to make a bigger nrrd";

int
unuParseBoundary(void *ptr, char *str, char err[AIR_STRLEN_HUGE]) {
  char me[]="unuParseBoundary";
  int *typeP;

  if (!(ptr && str)) {
    sprintf(err, "%s: got NULL pointer", me);
    return 1;
  }
  typeP = ptr;
  *typeP = nrrdEnumStrToVal(nrrdEnumBoundary, str);
  if (nrrdTypeUnknown == *typeP) {
    sprintf(err, "%s: \"%s\" is not a recognized boundary behavior", me, str);
    return 1;
  }
  return 0;
}

hestCB unuBoundaryHestCB = {
  sizeof(int),
  "boundary behavior",
  unuParseBoundary,
  NULL
};

int
padMain(int argc, char **argv, char *me) {
  hestOpt *opt = NULL;
  char *out, *err;
  Nrrd *nin, *nout;
  int *minOff, numMin, *maxOff, numMax, ax, bb, ret,
    min[NRRD_DIM_MAX], max[NRRD_DIM_MAX];
  double padVal;
  airArray *mop;

  OPT_ADD_NIN(nin, "input");
  OPT_ADD_BOUND("min", minOff, "low corner of bounding box", numMin);
  OPT_ADD_BOUND("max", maxOff, "high corner of bounding box", numMax);
  hestOptAdd(&opt, "b|boundary", "bb", airTypeOther, 1, 1, &bb, "bleed",
	     "behavior at boundary: pad, bleed, or wrap");
  hestOptAdd(&opt, "v|value", "val", airTypeDouble, 1, 1, &padVal, "0.0",
	     "for \"pad\" boundary behavior, pad with this value");
  OPT_ADD_NOUT(out, "output nrrd");

  mop = airMopInit();
  airMopAdd(mop, opt, (airMopper)hestOptFree, airMopAlways);

  USAGE(padInfo);
  PARSE();

  if (!( numMin == nin->dim && numMax == nin->dim )) {
    fprintf(stderr,
	    "%s: # min coords (%d) or max coords (%d) != nrrd dim (%d)\n",
	    me, numMin, numMax, nin->dim);
    airMopError(mop);
    return 1;
  }
  for (ax=0; ax<=nin->dim-1; ax++) {
    min[ax] = minOff[0 + 2*ax]*(nin->axis[ax].size-1) + minOff[1 + 2*ax];
    max[ax] = maxOff[0 + 2*ax]*(nin->axis[ax].size-1) + maxOff[1 + 2*ax];
    fprintf(stderr, "%s: ax %2d: min = %4d, max = %4d\n",
	    me, ax, min[ax], max[ax]);
  }

  nout = nrrdNew();
  airMopAdd(mop, nout, (airMopper)nrrdNuke, airMopAlways);

  if (nrrdBoundaryPad == bb)
    ret = nrrdPad(nout, nin, min, max, bb, padVal);
  else
    ret = nrrdPad(nout, nin, min, max, bb);
  if (ret) {
    airMopAdd(mop, err = biffGetDone(NRRD), airFree, airMopAlways);
    fprintf(stderr, "%s: error padding nrrd:\n%s", me, err);
    airMopError(mop);
    return 1;
  }

  SAVE();

  airMopOkay(mop);
  return 0;
}
