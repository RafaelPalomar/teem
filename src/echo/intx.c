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

#include "echo.h"
#include "privateEcho.h"

#define SET_POS(intx, ray) \
  intx->pos[0] = ray->from[0] + intx->t*ray->dir[0]; \
  intx->pos[1] = ray->from[1] + intx->t*ray->dir[1]; \
  intx->pos[2] = ray->from[2] + intx->t*ray->dir[2]
  
int
_echoRayIntxSphere(INTX_ARGS(Sphere)) {
  echoPos_t t, A, B, C, r[3], dscr, pos[3];

  ELL_3V_SUB(r, ray->from, obj->pos);
  A = ELL_3V_DOT(ray->dir, ray->dir);
  B = 2*ELL_3V_DOT(ray->dir, r);
  C = ELL_3V_DOT(r, r) - obj->rad*obj->rad;
  dscr = B*B - 4*A*C;
  if (dscr <= 0) {
    /* grazes or misses (most common case) */
    return AIR_FALSE;
  }
  /* else */
  dscr = sqrt(dscr);
  t = (-B - dscr)/(2*A);
  if (!AIR_IN_CL(ray->neer, t, ray->faar)) {
    t = (-B + dscr)/(2*A);
    if (!AIR_IN_CL(ray->neer, t, ray->faar)) {
      return AIR_FALSE;
    }
  }
  /* else one of the intxs is in [neer,faar] segment */
  intx->t = t;
  ELL_3V_SCALEADD(pos, 1, ray->from, t, ray->dir);
  ELL_3V_SUB(intx->norm, pos, obj->pos);
  intx->obj = (EchoObject *)obj;
  /* set in intx:
     yes: t, norm
     no: u, v, view, pos
  */
  return AIR_TRUE;
}

void
_echoRayIntxUVSphere(EchoIntx *intx) {
  echoPos_t len, u, v;

  ELL_3V_NORM(intx->norm, intx->norm, len);
  if (intx->norm[0] || intx->norm[1]) {
    u = atan2(intx->norm[1], intx->norm[0]);
    intx->u = AIR_AFFINE(-M_PI, u, M_PI, 0.0, 1.0);
    v = -asin(intx->norm[2]);
    intx->v = AIR_AFFINE(-M_PI/2, v, M_PI/2, 0.0, 1.0);
  }
  else {
    intx->u = 0;
    intx->v = AIR_AFFINE(1.0, intx->norm[2], -1.0, 0.0, 1.0);
  }
}

void
_echoRayIntxUVTriMesh(EchoIntx *intx) {
  echoPos_t u, v, norm[3], len;
  EchoTriMesh *trim;

  trim = TRIM(intx->obj);
  ELL_3V_SUB(norm, intx->pos, trim->origin);
  ELL_3V_NORM(norm, norm, len);
  if (norm[0] || norm[1]) {
    u = atan2(norm[1], norm[0]);
    intx->u = AIR_AFFINE(-M_PI, u, M_PI, 0.0, 1.0);
    v = -asin(norm[2]);
    intx->v = AIR_AFFINE(-M_PI/2, v, M_PI/2, 0.0, 1.0);
  }
  else {
    intx->u = 0;
    intx->v = AIR_AFFINE(1.0, norm[2], -1.0, 0.0, 1.0);
  }
}

int
_echoRayIntxCubeTest(echoPos_t *tP, int *axP, int *dirP,
		     echoPos_t xmin, echoPos_t xmax,
		     echoPos_t ymin, echoPos_t ymax,
		     echoPos_t zmin, echoPos_t zmax,
		     EchoRay *ray) {
  echoPos_t txmin, tymin, tzmin, txmax, tymax, tzmax,
    dx, dy, dz, ox, oy, oz, tmin, tmax;
  int axmin, axmax, sgn[3];

  ELL_3V_GET(dx, dy, dz, ray->dir);
  ELL_3V_GET(ox, oy, oz, ray->from);
  if (dx >= 0) { txmin = (xmin - ox)/dx; txmax = (xmax - ox)/dx; sgn[0] = -1; }
  else         { txmin = (xmax - ox)/dx; txmax = (xmin - ox)/dx; sgn[0] =  1; }
  if (dy >= 0) { tymin = (ymin - oy)/dy; tymax = (ymax - oy)/dy; sgn[1] = -1; }
  else         { tymin = (ymax - oy)/dy; tymax = (ymin - oy)/dy; sgn[1] =  1; }
  if (dz >= 0) { tzmin = (zmin - oz)/dz; tzmax = (zmax - oz)/dz; sgn[2] = -1; }
  else         { tzmin = (zmax - oz)/dz; tzmax = (zmin - oz)/dz; sgn[2] =  1; }
  if (txmin > tymin) { tmin = txmin; axmin = 0; }
  else               { tmin = tymin; axmin = 1; }
  if (tzmin > tmin)  { tmin = tzmin; axmin = 2; }
  if (txmax < tymax) { tmax = txmax; axmax = 0; }
  else               { tmax = tymax; axmax = 1; }
  if (tzmax < tmax)  { tmax = tzmax; axmax = 2; }
  if (0 && echoVerbose) {
    printf("%s: dir = (%g,%g,%g); tx: %g,%g ; ty: %g,%g ; tz: %g,%g \n"
	   "  ---> %g,%g (%d)\n"
	   "  axmin = %d, axmax = %d\n",
	   "_echoRayIntxCubeTest",
	   dx, dy, dz,
	   txmin, txmax, tymin, tymax, tzmin, tzmax,
	   tmin, tmax, tmin < tmax, axmin, axmax);
  }
  if (tmin >= tmax)
    return AIR_FALSE;
  *tP = tmin;
  *axP = axmin;
  *dirP = sgn[axmin];
  if (!AIR_IN_CL(ray->neer, *tP, ray->faar)) {
    *tP = tmax;
    *axP = axmax;
    *dirP = sgn[axmax];
    if (!AIR_IN_CL(ray->neer, *tP, ray->faar)) {
      return AIR_FALSE;
    }
  }
  return AIR_TRUE;
}

int
_echoRayIntxCube(INTX_ARGS(Cube)) {
  echoPos_t t;
  int ax, dir;

  if (!_echoRayIntxCubeTest(&t, &ax, &dir,
			    -0.5, 0.5,
			    -0.5, 0.5,
			    -0.5, 0.5, ray)) 
    return AIR_FALSE;
  intx->obj = (EchoObject *)obj;
  intx->t = t;
  switch(ax) {
  case 0: ELL_3V_SET(intx->norm, dir, 0, 0); break;
  case 1: ELL_3V_SET(intx->norm, 0, dir, 0); break;
  case 2: ELL_3V_SET(intx->norm, 0, 0, dir); break;
  }
  intx->face = ax;
  if (0 && echoVerbose) {
    printf("%s: ax = %d --> norm = (%g,%g,%g)\n",
	   "_echoRayIntxCube", ax,
	   intx->norm[0], intx->norm[1], intx->norm[2]);
  }
  /* set in intx:
     yes: t, norm, face
     no: u, v, view, pos
  */
  return AIR_TRUE;
}

void
_echoRayIntxUVCube(EchoIntx *intx) {
  echoPos_t x, y, z;

  ELL_3V_GET(x, y, z, intx->pos);
  switch(intx->face) {
  case 0:
    intx->u = AIR_AFFINE(-0.5, y, 0.5, 0.0, 1.0);
    intx->v = AIR_AFFINE(-0.5, z, 0.5, 0.0, 1.0);
    break;
  case 1:
    intx->u = AIR_AFFINE(-0.5, x, 0.5, 0.0, 1.0);
    intx->v = AIR_AFFINE(-0.5, z, 0.5, 0.0, 1.0);
    break;
  case 2:
    intx->u = AIR_AFFINE(-0.5, x, 0.5, 0.0, 1.0);
    intx->v = AIR_AFFINE(-0.5, y, 0.5, 0.0, 1.0);
    break;
  }
}

/*
** TRI_INTX
**
** given a triangle in terms of origin, edge0, edge1, this will
** begin the intersection calculation:
** - sets pvec, tvec, qvec (all of them if intx is not ruled out )
** - sets u, and rules out intx based on (u < 0.0 || u > 1.0)
** - sets v, and rules out intx based on COND
** - sets t, and rules out intx based on (t < neer || t > faar)
*/
#define TRI_INTX(ray, origin, edge0, edge1, pvec, qvec, tvec,                \
                 det, t, u, v, COND, NOPE)                                   \
  ELL_3V_CROSS(pvec, ray->dir, edge1);                                       \
  det = ELL_3V_DOT(pvec, edge0);                                             \
  if (det > -ECHO_EPSILON && det < ECHO_EPSILON) {                           \
    NOPE;                                                                    \
  }                                                                          \
  /* now det is the reciprocal of the determinant */                         \
  det = 1.0/det;                                                             \
  ELL_3V_SUB(tvec, ray->from, origin);                                       \
  u = det * ELL_3V_DOT(pvec, tvec);                                          \
  if (u < 0.0 || u > 1.0) {                                                  \
    NOPE;                                                                    \
  }                                                                          \
  ELL_3V_CROSS(qvec, tvec, edge0);                                           \
  v = det * ELL_3V_DOT(qvec, ray->dir);                                      \
  if (COND) {                                                                \
    NOPE;                                                                    \
  }                                                                          \
  t = det * ELL_3V_DOT(qvec, edge1);                                         \
  if (t < ray->neer || t > ray->faar) {                                      \
    NOPE;                                                                    \
  }

int
_echoRayIntxRectangle(INTX_ARGS(Rectangle)) {
  echoPos_t pvec[3], qvec[3], tvec[3], det, t, u, v, *edge0, *edge1;
  
  if (echoMatterLight == obj->matter
      && (ray->shadow || !parm->renderLights)) {
    return AIR_FALSE;
  }
  edge0 = obj->edge0;
  edge1 = obj->edge1;
  TRI_INTX(ray, obj->origin, edge0, edge1,
	   pvec, qvec, tvec, det, t, u, v,
	   (v < 0.0 || v > 1.0), return AIR_FALSE);
  intx->t = t;
  intx->u = u;
  intx->v = v;
  ELL_3V_CROSS(intx->norm, edge0, edge1);
  intx->obj = OBJECT(obj);
  /* set in intx:
     yes: t, u, v, norm
     no: pos, view
  */
  return AIR_TRUE;
}

int
_echoRayIntxTriangle(INTX_ARGS(Triangle)) {
  echoPos_t pvec[3], qvec[3], tvec[3], det, t, u, v, edge0[3], edge1[3];
  
  ELL_3V_SUB(edge0, obj->vert[1], obj->vert[0]);
  ELL_3V_SUB(edge1, obj->vert[2], obj->vert[0]);
  TRI_INTX(ray, obj->vert[0], edge0, edge1,
	   pvec, qvec, tvec, det, t, u, v,
	    (v < 0.0 || u + v > 1.0), return AIR_FALSE);
  intx->t = t;
  intx->u = u;
  intx->v = v;
  ELL_3V_CROSS(intx->norm, edge0, edge1);
  intx->obj = (EchoObject *)obj;
  /* set in intx:
     yes: t, u, v, norm
     no: pos, view
  */
  return AIR_TRUE;
}

int
_echoRayIntxTriMesh(INTX_ARGS(TriMesh)) {
  echoPos_t *pos, vert0[3], edge0[3], edge1[3], pvec[3], qvec[3], tvec[3],
    det, t, u, v;
  EchoTriMesh *trim;
  int i, ax, dir, ret;

  trim = TRIMESH(obj);
  if (!_echoRayIntxCubeTest(&t, &ax, &dir,
			    trim->min[0], trim->max[0],
			    trim->min[1], trim->max[1],
			    trim->min[2], trim->max[2], ray)) {
    if (echoVerbose) {
      printf("(trimesh bbox (%g,%g,%g) --> (%g,%g,%g) not hit)\n",
	     trim->min[0], trim->min[1], trim->min[2],
	     trim->max[0], trim->max[1], trim->max[2]);
    }
    return AIR_FALSE;
  }
  /* stupid lineer search for now */
  ret = AIR_FALSE;
  for (i=0; i<trim->numF; i++) {
    pos = trim->pos + 3*trim->vert[0 + 3*i];
    ELL_3V_COPY(vert0, pos);
    pos = trim->pos + 3*trim->vert[1 + 3*i];
    ELL_3V_SUB(edge0, pos, vert0);
    pos = trim->pos + 3*trim->vert[2 + 3*i];
    ELL_3V_SUB(edge1, pos, vert0);
    TRI_INTX(ray, vert0, edge0, edge1,
	     pvec, qvec, tvec, det, t, u, v,
	     (v < 0.0 || u + v > 1.0), continue);
    if (ray->shadow) {
      return AIR_TRUE;
    }
    intx->t = ray->faar = t;
    intx->u = u;
    intx->v = v;
    ELL_3V_CROSS(intx->norm, edge0, edge1);
    intx->obj = (EchoObject *)obj;
    intx->face = i;
    ret = AIR_TRUE;
  }
  return ret;
}

int
_echoRayIntxSplit(INTX_ARGS(Split)) {
  EchoObject *a, *b;
  echoPos_t *mina, *minb, *maxa, *maxb, t;
  int ret, ax, dir;

  if (ray->dir[obj->axis] > 0) {
    a = obj->obj0;
    mina = obj->min0;
    maxa = obj->max0;
    b = obj->obj1;
    minb = obj->min1;
    maxb = obj->max1;
  }
  else {
    a = obj->obj1;
    mina = obj->min1;
    maxa = obj->max1;
    b = obj->obj0;
    minb = obj->min0;
    maxb = obj->max0;
  }

  if (echoVerbose) {
    printf("_echoRayIntxSplit (shadow = %d):\n", ray->shadow);
    printf("_echoRayIntxSplit: 1st: (%g,%g,%g) -- (%g,%g,%g) (obj %d)\n", 
	   mina[0], mina[1], mina[2],
	   maxa[0], maxa[1], maxa[2], a->type);
    printf("_echoRayIntxSplit: 2nd: (%g,%g,%g) -- (%g,%g,%g) (obj %d)\n",
	   minb[0], minb[1], minb[2],
	   maxb[0], maxb[1], maxb[2], b->type);
  }

  ret = AIR_FALSE;
  if (_echoRayIntxCubeTest(&t, &ax, &dir,
			   mina[0], maxa[0],
			   mina[1], maxa[1],
			   mina[2], maxa[2], ray)) {
    intx->boxhits++;
    if (_echoRayIntx[a->type](intx, ray, parm, a)) {
      if (ray->shadow) {
	return AIR_TRUE;
      }
      ray->faar = intx->t;
      ret = AIR_TRUE;
    }
  }
  if (_echoRayIntxCubeTest(&t, &ax, &dir,
			   minb[0], maxb[0],
			   minb[1], maxb[1],
			   minb[2], maxb[2], ray)) {
    intx->boxhits++;
    if (_echoRayIntx[b->type](intx, ray, parm, b)) {
      ray->faar = intx->t;
      ret = AIR_TRUE;
    }
  }
  return ret;
}

int
_echoRayIntxList(INTX_ARGS(List)) {
  int i, ret, ax, dir;
  EchoObject *kid;
  EchoAABBox *box;
  echoPos_t t;

  if (echoAABBox == obj->type) {
    box = AABBOX(obj);
    if (!_echoRayIntxCubeTest(&t, &ax, &dir,
			      box->min[0], box->max[0],
			      box->min[1], box->max[1],
			      box->min[2], box->max[2], ray)) {
      if (0 && echoVerbose) {
	printf("_echoRayIntxList: missed box (%g,%g,%g)--(%g,%g,%g)\n",
	       box->min[0], box->min[1], box->min[2],
	       box->max[0], box->max[1], box->max[2]);
      }
      return AIR_FALSE;
    }
    intx->boxhits++;
  }
  
  ret = AIR_FALSE;
  if (echoVerbose) {
    printf("_echoRayIntxList(d=%d): have %d kids to test\n",
	   ray->depth, obj->objArr->len);
  }
  for (i=0; i<obj->objArr->len; i++) {
    kid = obj->obj[i];
    if (0 && echoVerbose) {
      printf("_echoRayIntxList: testing a %d ... ", kid->type);
    }
    if (_echoRayIntx[kid->type](intx, ray, parm, kid)) {
      ray->faar = intx->t;
      ret = AIR_TRUE;
      if (0 && echoVerbose) {
	printf("YES\n");
      }
    }
    else {
      if (0 && echoVerbose) {
	printf("YES\n");
      }
    }
  }
  
  return ret;
}

int
_echoRayIntxInstance(INTX_ARGS(Instance)) {
  echoPos_t a[4], b[4];
  EchoRay iray;

  /*
  ELL_3V_COPY(iray.from, ray->from);
  ELL_3V_COPY(iray.dir, ray->dir);
  */
  ELL_4V_SET(a, ray->from[0], ray->from[1], ray->from[2], 1);
  ELL_4MV_MUL(b, obj->Mi, a);  ELL_34V_HOMOG(iray.from, b);
  if (0 && echoVerbose) {
    ell4mPrint_p(stdout, obj->Mi);
    printf("from (%g,%g,%g)\n   -- Mi --> (%g,%g,%g,%g)\n   --> (%g,%g,%g)\n",
	   a[0], a[1], a[2],
	   b[0], b[1], b[2], b[3], 
	   iray.from[0], iray.from[1], iray.from[2]);
  }
  ELL_4V_SET(a, ray->dir[0], ray->dir[1], ray->dir[2], 0);
  ELL_4MV_MUL(b, obj->Mi, a);   ELL_3V_COPY(iray.dir, b);
  if (0 && echoVerbose) {
    printf("dir (%g,%g,%g)\n   -- Mi --> (%g,%g,%g,%g)\n   --> (%g,%g,%g)\n",
	   a[0], a[1], a[2],
	   b[0], b[1], b[2], b[3], 
	   iray.dir[0], iray.dir[1], iray.dir[2]);
  }
  
  iray.neer = ray->neer;
  iray.faar = ray->faar;
  iray.depth = ray->depth;
  iray.shadow = ray->shadow;
  
  if (_echoRayIntx[obj->obj->type](intx, &iray, parm, obj->obj)) {
    ELL_4V_SET(a, intx->norm[0], intx->norm[1], intx->norm[2], 0);
    ELL_4MV_TMUL(b, obj->Mi, a);
    ELL_3V_COPY(intx->norm, b);
    if (echoVerbose) {
      printf("hit a %d with M == \n", obj->obj->type);
      ell4mPrint_p(stdout, obj->M);
      printf(" (det = %f), and Mi == \n", ell4mDet_p(obj->M));
      ell4mPrint_p(stdout, obj->Mi);
    }
    return AIR_TRUE;
  }
  /* else */
  return AIR_FALSE;
}

void
_echoRayIntxUVNoop(EchoIntx *intx) {

}
	      

_echoRayIntx_t
_echoRayIntx[ECHO_OBJECT_MAX+1] = {
  NULL,
  (_echoRayIntx_t)_echoRayIntxSphere,
  (_echoRayIntx_t)_echoRayIntxCube,
  (_echoRayIntx_t)_echoRayIntxTriangle,
  (_echoRayIntx_t)_echoRayIntxRectangle,
  (_echoRayIntx_t)_echoRayIntxTriMesh,
  (_echoRayIntx_t)NULL,
  (_echoRayIntx_t)_echoRayIntxList,  /* trickery */
  (_echoRayIntx_t)_echoRayIntxSplit,
  (_echoRayIntx_t)_echoRayIntxList,
  (_echoRayIntx_t)_echoRayIntxInstance,
};

_echoRayIntxUV_t
_echoRayIntxUV[ECHO_OBJECT_MAX+1] = {
  NULL,
  _echoRayIntxUVSphere,
  _echoRayIntxUVCube,
  _echoRayIntxUVNoop,
  _echoRayIntxUVNoop,
  _echoRayIntxUVTriMesh,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};


int
echoRayIntx(EchoIntx *intx, EchoRay *ray,
	    EchoRTParm *parm, EchoObject *obj) {

  return _echoRayIntx[obj->type](intx, ray, parm, obj);
}
