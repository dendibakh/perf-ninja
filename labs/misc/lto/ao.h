#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#define WIDTH        256
#define HEIGHT       256
#define NSUBSAMPLES  2
#define NAO_SAMPLES  8

typedef struct _vec
{
    double x;
    double y;
    double z;
} vec;


typedef struct _Isect
{
    double t;
    vec    p;
    vec    n;
    int    hit; 
} Isect;

typedef struct _Sphere
{
    vec    center;
    double radius;

} Sphere;

typedef struct _Plane
{
    vec    p;
    vec    n;

} Plane;

typedef struct _Ray
{
    vec    org;
    vec    dir;
} Ray;

extern Sphere spheres[3];
extern Plane  plane;

// ao_helpers.c
double vdot(vec v0, vec v1);
void vcross(vec *c, vec v0, vec v1);
void vnormalize(vec *c);
unsigned char clamp(double f);
void saveppm(const char *fname, int w, int h, unsigned char *img);

// ao_intersect.c
void ray_sphere_intersect(Isect *isect, const Ray *ray, const Sphere *sphere);
void ray_plane_intersect(Isect *isect, const Ray *ray, const Plane *plane);

// ao_orthoBasis.c
void orthoBasis(vec *basis, vec n);

// ao_occlusion.c
void ambient_occlusion(vec *col, const Isect *isect);

// ao_render.c
void render(unsigned char *img, int w, int h, int nsubsamples);

// ao_init.c
void init_scene();

// ao.c
//int ao_bench();