#include "ao.h"

static void saveppm(const char *fname, int w, int h, unsigned char *img)
{
    FILE *fp;

    fp = fopen(fname, "wb");
    assert(fp);

    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", w, h);
    fprintf(fp, "255\n");
    fwrite(img, w * h * 3, 1, fp);
    fclose(fp);
}

int ao_bench()
{
    unsigned char *img = (unsigned char *)malloc(WIDTH * HEIGHT * 3);

    init_scene();

    render(img, WIDTH, HEIGHT, NSUBSAMPLES);

    saveppm("ao.ppm", WIDTH, HEIGHT, img); 

    free(img);

    return 0;
}