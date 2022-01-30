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
static double vdot(vec v0, vec v1)
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}

static void vcross(vec *c, vec v0, vec v1)
{

	c->x = v0.y * v1.z - v0.z * v1.y;
	c->y = v0.z * v1.x - v0.x * v1.z;
	c->z = v0.x * v1.y - v0.y * v1.x;
}

static void vnormalize(vec *c)
{
	double length = sqrt(vdot((*c), (*c)));

	if (fabs(length) > 1.0e-17) {
		c->x /= length;
		c->y /= length;
		c->z /= length;
	}
}

static unsigned char clamp(double f)
{
	int i = (int)(f * 255.5);

	if (i < 0) i = 0;
	if (i > 255) i = 255;

	return (unsigned char)i;
}
void saveppm(const char *fname, int w, int h, unsigned char *img);

// ao_intersect.c
static void ray_sphere_intersect(Isect *isect, const Ray *ray, const Sphere *sphere)
{
	vec rs;

	rs.x = ray->org.x - sphere->center.x;
	rs.y = ray->org.y - sphere->center.y;
	rs.z = ray->org.z - sphere->center.z;

	double B = vdot(rs, ray->dir);
	double C = vdot(rs, rs) - sphere->radius * sphere->radius;
	double D = B * B - C;

	if (D > 0.0) {
		double t = -B - sqrt(D);

		if ((t > 0.0) && (t < isect->t)) {
			isect->t = t;
			isect->hit = 1;

			isect->p.x = ray->org.x + ray->dir.x * t;
			isect->p.y = ray->org.y + ray->dir.y * t;
			isect->p.z = ray->org.z + ray->dir.z * t;

			isect->n.x = isect->p.x - sphere->center.x;
			isect->n.y = isect->p.y - sphere->center.y;
			isect->n.z = isect->p.z - sphere->center.z;

			vnormalize(&(isect->n));
		}
	}
}

static void ray_plane_intersect(Isect *isect, const Ray *ray, const Plane *plane)
{
	double d = -vdot(plane->p, plane->n);
	double v = vdot(ray->dir, plane->n);

	if (fabs(v) < 1.0e-17) return;

	double t = -(vdot(ray->org, plane->n) + d) / v;

	if ((t > 0.0) && (t < isect->t)) {
		isect->t = t;
		isect->hit = 1;

		isect->p.x = ray->org.x + ray->dir.x * t;
		isect->p.y = ray->org.y + ray->dir.y * t;
		isect->p.z = ray->org.z + ray->dir.z * t;

		isect->n = plane->n;
	}
}

// ao_orthoBasis.c
static void orthoBasis(vec *basis, vec n)
{
	basis[2] = n;
	basis[1].x = 0.0; basis[1].y = 0.0; basis[1].z = 0.0;

	if ((n.x < 0.6) && (n.x > -0.6)) {
		basis[1].x = 1.0;
	} else if ((n.y < 0.6) && (n.y > -0.6)) {
		basis[1].y = 1.0;
	} else if ((n.z < 0.6) && (n.z > -0.6)) {
		basis[1].z = 1.0;
	} else {
		basis[1].x = 1.0;
	}

	vcross(&basis[0], basis[1], basis[2]);
	vnormalize(&basis[0]);

	vcross(&basis[1], basis[2], basis[0]);
	vnormalize(&basis[1]);
}

// ao_occlusion.c
#include <random>
static void ambient_occlusion(vec *col, const Isect *isect)
{
	constexpr auto PI = 3.14159265358979323846;

	// this is an implementation of drand48 as defined in 
	// https://man7.org/linux/man-pages/man3/drand48.3.html
	// (linear_congruential_engine)
	auto mydrand48 = []() {
		static std::minstd_rand generator;
		static std::uniform_real_distribution<double> distribution(0, 1);
		return distribution(generator);
	};

	int    i, j;
	int    ntheta = NAO_SAMPLES;
	int    nphi   = NAO_SAMPLES;
	double eps = 0.0001;

	vec p;

	p.x = isect->p.x + eps * isect->n.x;
	p.y = isect->p.y + eps * isect->n.y;
	p.z = isect->p.z + eps * isect->n.z;

	vec basis[3];
	orthoBasis(basis, isect->n);

	double occlusion = 0.0;

	for (j = 0; j < ntheta; j++) {
		for (i = 0; i < nphi; i++) {
			double theta = sqrt(mydrand48());
			double phi   = 2.0 * PI * mydrand48();

			double x = cos(phi) * theta;
			double y = sin(phi) * theta;
			double z = sqrt(1.0 - theta * theta);

			// local -> global
			double rx = x * basis[0].x + y * basis[1].x + z * basis[2].x;
			double ry = x * basis[0].y + y * basis[1].y + z * basis[2].y;
			double rz = x * basis[0].z + y * basis[1].z + z * basis[2].z;

			Ray ray;

			ray.org = p;
			ray.dir.x = rx;
			ray.dir.y = ry;
			ray.dir.z = rz;

			Isect occIsect;
			occIsect.t   = 1.0e+17;
			occIsect.hit = 0;

			ray_sphere_intersect(&occIsect, &ray, &spheres[0]); 
			ray_sphere_intersect(&occIsect, &ray, &spheres[1]); 
			ray_sphere_intersect(&occIsect, &ray, &spheres[2]); 
			ray_plane_intersect (&occIsect, &ray, &plane); 

			occlusion += occIsect.hit ? 1.0 : 0.0;

		}
	}

	occlusion = (ntheta * nphi - occlusion) / (double)(ntheta * nphi);

	col->x = occlusion;
	col->y = occlusion;
	col->z = occlusion;
}
// ao_render.c
void render(unsigned char *img, int w, int h, int nsubsamples);

// ao_init.c
void init_scene();

// ao.c
//int ao_bench();