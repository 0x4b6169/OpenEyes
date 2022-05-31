#include <stdio.h>
#include <string.h>

int capture_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
    FILE *f;
    int i;
    char file[512];
    strcpy(file, ""); // change this to output to file external drive
    strcat(file, filename);
    f = fopen(file,"w");

    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}
