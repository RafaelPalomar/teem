/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

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

/* volumePull.c */
extern pullVolume *_pullVolumeCopy(pullVolume *pvol);

/* infoPull.c */
extern unsigned int _pullInfoAnswerLen[PULL_INFO_MAX+1];
extern void (*_pullInfoAnswerCopy[10])(double *, const double *);
extern int _pullInfoSetup(pullContext *pctx);

/* contextPull.c */
extern int _pullContextCheck(pullContext *pctx);

/* taskPull.c */
extern pullTask *_pullTaskNew(pullContext *pctx, int threadIdx);
extern pullTask *_pullTaskNix(pullTask *task);
extern int _pullTaskSetup(pullContext *pctx);

/* pointPull.c */
extern unsigned int _pullPointTotal(pullContext *pctx);
extern int _pullProbe(pullTask *task, pullPoint *point);

/* binningPull.c */
extern pullBin *_pullBinLocate(pullContext *pctx, double *pos);
extern void _pullBinPointAdd(pullContext *pctx,
                             pullBin *bin, pullPoint *point);
extern void _pullBinPointRemove(pullContext *pctx, pullBin *bin, int loseIdx);
extern void _pullBinNeighborSet(pullBin *bin, pullBin **nei, unsigned int num);
extern int _pullBinSetup(pullContext *pctx);

/* actionPull.c */
extern int _pullProbe(pullTask *task, pullPoint *point);

/* pointPull.c */
extern int _pullPointSetup(pullContext *pctx);

/* corePull.c */
extern int _pullProcess(pullTask *task);
extern void *_pullWorker(void *_task);

#ifdef __cplusplus
}
#endif