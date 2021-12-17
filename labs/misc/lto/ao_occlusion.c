#include "ao.h"

void ambient_occlusion(vec *col, const Isect *isect)
{
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
            double theta = sqrt(drand48());
            double phi   = 2.0 * M_PI * drand48();

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

            if (occIsect.hit) occlusion += 1.0;
            
        }
    }

    occlusion = (ntheta * nphi - occlusion) / (double)(ntheta * nphi);

    col->x = occlusion;
    col->y = occlusion;
    col->z = occlusion;
}
