#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "seamcarving.h"
#include "c_img.h"

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int height = im -> height;
    int width  = im -> width;
    create_img(grad, im->height, im->width);
        
    for (int i = 0; i < height; i++){
        for (int j = 0; j < width ; j++){
            float dx = 0, dy = 0;
            for (int col = 0; col<3; col++){
                if (i == 0  && j == 0){
                    dy = dy + pow((get_pixel(im, 1, 0, col) - get_pixel(im, height-1, 0, col)), 2.0);
                    dx = dx + pow((get_pixel(im, 0, 1, col) - get_pixel(im, 0, width-1, col)), 2.0);
                }else if (i == 0 && j == width - 1){
                    dy = dy + pow((get_pixel(im, i+1, j, col) - get_pixel(im, height-1, j, col)), 2.0);
                    dx = dx + pow((get_pixel(im, i, 0, col) - get_pixel(im, i, width-2, col)), 2.0);
                }else if (i == height - 1 && j ==0){
                    dy = dy + pow((get_pixel(im, 0, 0, col) - get_pixel(im, height-2, j, col)), 2.0);
                    dx = dx + pow((get_pixel(im, i, 1, col) - get_pixel(im, i, width-1, col)), 2.0);
                }else if (i == height - 1 && j == width - 1){
                    dy = dy + pow((get_pixel(im, 0, width - 1, col) - get_pixel(im, height-2, width - 1, col)), 2.0);
                    dx = dx + pow((get_pixel(im, i, 0, col) - get_pixel(im, i, width-2, col)), 2.0);
                }else if(i == 0){
                    dy = dy + pow((get_pixel(im, 1, j, col) - get_pixel(im, height-1, j, col)), 2.0);
                    dx = dx + pow((get_pixel(im, 0, j+1, col) - get_pixel(im, 0, j-1, col)), 2.0);
                }else if(i == height-1){
                    dy = dy + pow((get_pixel(im, 0, j, col) - get_pixel(im, height-2, j, col)), 2.0);
                    dx = dx + pow((get_pixel(im, i, j+1, col) - get_pixel(im, i, j-1, col)), 2.0);
                }else if (j == 0){
                    dy = dy + pow((get_pixel(im, i+1, j, col) - get_pixel(im, i-1, j, col)), 2.0);
                    dx = dx + pow((get_pixel(im, i, j+1, col) - get_pixel(im, i, width-1, col)), 2.0);
                }else if (j == width - 1){
                    dy = dy + pow((get_pixel(im, i+1, j, col) - get_pixel(im, i-1, j, col)), 2.0);
                    dx = dx + pow((get_pixel(im, i, 0, col) - get_pixel(im, i, width-2, col)), 2.0);
                }else{
                    dy = dy + pow((get_pixel(im, i+1, j, col) - get_pixel(im, i-1, j, col)), 2.0);
                    dx = dx + pow((get_pixel(im, i, j+1, col) - get_pixel(im, i, j-1, col)), 2.0);
                }
            }
            float n = sqrt(dx + dy);
            uint8_t gradnum = (uint8_t)(n/10);
            set_pixel(*grad, i, j, gradnum, gradnum, gradnum);
        }
    }
}

int find_min(double a, double b, double c){
    double min;
    if (b < c){
        min = b;
    }else{
        min = c;
    }
    if (a < min){
        return a;
    }else{
        return min;
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    *best_arr = (double *)malloc(sizeof(double) * grad->height * grad->width);
    for (int i = 0; i < grad->width; i++){
        (*best_arr) [i] = (double)get_pixel(grad, 0, i, 0);
    }
    for (int y = 1; y < grad->height; y++){
        for (int x = 0; x < grad->width; x++){
            if (x == 0){
               (*best_arr)[y * grad->width + x] = (double)get_pixel(grad, y, x, 0) + find_min((*best_arr)[(y-1) * grad->width + x], (*best_arr)[(y-1) * grad->width + x + 1], 1000000);
            }else if(x == grad->width -1){
                (*best_arr)[y * grad->width + x] = (double)get_pixel(grad, y, x, 0) +find_min((*best_arr)[(y-1) * grad->width + x], (*best_arr)[(y-1) * grad->width + x -1], 1000000);
            }else{
                (*best_arr)[y * grad->width + x] = (double)get_pixel(grad, y, x, 0) + find_min((*best_arr)[(y-1) * grad->width + x], (*best_arr)[(y-1) * grad->width + x-1], (*best_arr)[(y-1) * grad->width + x+1]);
            }
        }
    }

    // for (int y = 0; y < grad->height; y++){
    //     for (int x = 0; x < grad->width; x++){
    //         printf("%f\t", (*best_arr)[y*grad->width + x]);
    //     }
    //     printf("\n");
    // }
}

void recover_path(double *best, int height, int width, int **path){
    int pos;
    *path = (int *)malloc(sizeof(int) * height);
    for (int i = height - 1; i > -1; i--){
        float min = 100000;
        if (i == height - 1){
            for (int j = 0; j < width; j++){
                if (best[i * width + j] < min){
                    min = best[i * width + j];
                    pos = j;
                }
            }
        }else if (pos == 0){
            if (best[i * width] < best[i * width + 1]){
                min = best[i*width];
                pos = 0;
            }else{
                min = best[i*width + 1];
                pos = 1;
            }
        }else if(pos == width-1){
            if (best[(i+1)*width-1] < best[(i+1)*width-2]){
                min = best[(i + 1) * width - 1];
                pos = width - 1;
            }else{
                min = best[i * width - 2];
                pos = width - 2;
            }
        }else{
            if (best[(i) * width + pos -1] < best[(i) * width + pos]){
                if (best[(i) * width + pos -1] < best[(i) * width + pos+1]){
                    min = best[i * width + pos -1];
                    pos = pos - 1;
                }else{
                    min = best[i * width + pos + 1];
                    pos = pos + 1;
                }
            }else{
                if(best[(i) * width + pos] < best[(i) * width + pos +1]){
                    min = best[i * width + pos];
                }else{
                    min = best[i * width + pos + 1];
                    pos = pos + 1;
                }
            }
        }
        (*path)[i] = pos;
        //printf("%d\t", pos);
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    create_img(dest, src->height, src->width - 1);
    for (int y = 0; y < (*dest)->height - 1; y++){
        int cut = path[y];
        for (int x = 0; x <(*dest)-> width - 1; x++){
            if (x != cut){
                int pix[3];
                for (int col = 0; col < 3; col++){
                    pix[col] = get_pixel(src, y, x, col);
                }
                set_pixel(*dest, y, x, pix[0], pix[1], pix[2]);
            }else{
                break;
            }
        }
        for (int x = cut; x < src->width - 1; x++){
            int pix[3];
            for (int col = 0; col < 3; col++){
                pix[col] = get_pixel(src, y, x + 1, col);
            }
            set_pixel(*dest, y, x, pix[0], pix[1], pix[2]);
        }
    }
}