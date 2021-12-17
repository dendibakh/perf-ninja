#include "ao.h"

int ao_bench()
{
    unsigned char *img = (unsigned char *)malloc(WIDTH * HEIGHT * 3);

    init_scene();

    render(img, WIDTH, HEIGHT, NSUBSAMPLES);

    saveppm("ao.ppm", WIDTH, HEIGHT, img); 

    free(img);

    return 0;
}