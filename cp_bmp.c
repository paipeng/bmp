//
//  cp_bmp.c
//  CPCodeLib
//
//  Created by Pai Peng on 2021/8/12.
//

#include "cp_bmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



#pragma pack(2)
typedef struct _bitmap_file_header
{
   short fileType;
   int fileSize;
   int reserved;
   int pixelDataOffset;
} BitmapFileHeader;

#pragma pack()
typedef struct _bitmap_info_header
{
   int headerSize;
   int imageWidth;
   int imageHeight;
   short planes;
   short bitsPerPixel;
   int compression;
   int imageSize;
   int xPixelsPerMeter;
   int yPixelsPerMeter;
   int totalColors;
   int importantColors;
} BitmapInfoHeader;




cp_mat* init_cp_mat(unsigned int width, unsigned int height, unsigned int channel) {
    unsigned int i;
    cp_mat* mat = NULL;
    if (width <= 0 || height <= 0 || channel <= 0) {
        return mat;
    }
    mat = (cp_mat*)malloc(sizeof(cp_mat));
    if (mat != NULL) {
        mat->width = width;
        mat->height = height;
        mat->channel = channel;
        mat->data = (unsigned char**)malloc(sizeof(unsigned char*) * height);
        if (mat->data != NULL) {
            mat->data[0] = (unsigned char*)malloc(sizeof(unsigned char) * width * height * channel);
            if (mat->data[0] != NULL) {
                for (i = 1; i < height; i++) {
                    mat->data[i] = mat->data[i - 1] + width * channel;
                }
            }
        }
    }
    return mat;
}

void free_cp_mat(cp_mat* mat) {
    if (mat != NULL) {
        if (mat->data != NULL) {
            free(mat->data[0]);
            free(mat->data);
            mat->data = NULL;
        }
        free(mat);
        mat = NULL;
    }
}

cp_mat* cp_read_bmp_image(char* file) {
    cp_mat* mat = NULL;
    FILE *f = NULL;
    BitmapFileHeader bitmapFileHeader;
    BitmapInfoHeader bitmapInfoHeader;
    unsigned char *buf = NULL;
    int row_size, i, j;//, pos = 0;
    int padding = 0;
    char padding_buf[8];
    size_t read_size;
    int ret;
#if WIN32
    ret = fopen_s(&f, file, "rb+");
#else
    f = fopen(file, "rb+");
#endif
    if (f != NULL) {
        read_size = fread(&bitmapFileHeader, sizeof(BitmapFileHeader), 1, f);
        read_size = fread(&bitmapInfoHeader, sizeof(BitmapInfoHeader), 1, f);
        mat = init_cp_mat(bitmapInfoHeader.imageWidth, bitmapInfoHeader.imageHeight, bitmapInfoHeader.bitsPerPixel==1?1:bitmapInfoHeader.bitsPerPixel/8);
        if (mat != NULL) {
            // define row size for reading
            row_size = 4*((bitmapInfoHeader.imageWidth*bitmapInfoHeader.bitsPerPixel+31)/32);
            padding = row_size - bitmapInfoHeader.imageWidth;
            
            //printf("offsetBits: %d\n", bitmapFileHeader.pixelDataOffset);
            fseek(f, bitmapFileHeader.pixelDataOffset, SEEK_SET);
            
            // read grayscale images
            if (bitmapInfoHeader.bitsPerPixel == 8) {
                for (i=0;i<bitmapInfoHeader.imageHeight;i++) {
                    read_size = fread(&mat->data[bitmapInfoHeader.imageHeight - i - 1][0], 1, bitmapInfoHeader.imageWidth, f);
                    read_size = fread(&padding_buf, 1, padding, f);
                }
            } else if (bitmapInfoHeader.bitsPerPixel == 24) {
                for (i=0;i<bitmapInfoHeader.imageHeight;i++) {
                    read_size = fread(&mat->data[bitmapInfoHeader.imageHeight - i - 1][0], 1, row_size, f);
                }
            } else if (bitmapInfoHeader.bitsPerPixel==1) {
                printf("w: %d\n", row_size);
                buf = (unsigned char*)malloc(row_size);
                for (i=0;i<bitmapInfoHeader.imageHeight;i++) {
                    read_size = fread(&buf[0], 1, row_size, f);
                    for (j=0;j<bitmapInfoHeader.imageWidth;j++) {
                        mat->data[bitmapInfoHeader.imageHeight - i - 1][j] = (1 - ((buf[j/8]>>(7-j%8))&0x01)) * 0xFF;
                    }
                }
                free(buf);
            }
        }
        fclose(f);
    }
    return mat;;
}

int cp_write_bmp_image(char* file, cp_mat* mat, int dpi) {
    FILE *f = NULL;
    BitmapFileHeader bitmapFileHeader;
    BitmapInfoHeader bitmapInfoHeader;
    int i, j, bitsPerPixel, row_size, remainder;
    unsigned char value;
    unsigned char coltab[256][4]; // color table
    int color_palatte_size = 0;
    size_t write_size;
    int ret;

    if (mat == NULL)
        return -1;
    
    if (mat->channel == 1) {
        color_palatte_size = 256 * 4;
    }

    bitsPerPixel = mat->channel * 8;
    row_size = 4*((mat->width * bitsPerPixel + 31)/32);

    bitmapFileHeader.fileType = 0x4D42;
    bitmapFileHeader.fileSize = 54 + color_palatte_size + row_size * mat->height;
    bitmapFileHeader.reserved = 0;
    bitmapFileHeader.pixelDataOffset = 54 + color_palatte_size;
    
    bitmapInfoHeader.headerSize = 40;
    bitmapInfoHeader.imageWidth = mat->width;
    bitmapInfoHeader.imageHeight = mat->height;
    bitmapInfoHeader.planes = 1;
    bitmapInfoHeader.bitsPerPixel = bitsPerPixel;
    bitmapInfoHeader.compression = 0;
    bitmapInfoHeader.imageSize = row_size * bitmapInfoHeader.imageHeight;
    bitmapInfoHeader.xPixelsPerMeter = dpi*10000/254;
    bitmapInfoHeader.yPixelsPerMeter = bitmapInfoHeader.xPixelsPerMeter;
    if (mat->channel == 1) {
        bitmapInfoHeader.totalColors = 256;
    } else {
        bitmapInfoHeader.totalColors = 0;
    }
    bitmapInfoHeader.importantColors = 0;

#if WIN32
    ret = fopen_s(&f, file, "rb+");
#else
    f = fopen(file, "rb+");
#endif
    if (f!=NULL) {
        write_size = fwrite(&bitmapFileHeader, sizeof(BitmapFileHeader), 1, f);
        write_size = fwrite(&bitmapInfoHeader, sizeof(BitmapInfoHeader), 1, f);
        
        if (mat->channel == 1) {
            for (i=0;i<256;i++)
                for (j=0;j<4;j++)
                    coltab[i][j] = (unsigned char)(i*(j!=3));
            write_size = fwrite(&coltab[0][0], 1, 256*4, f);
        }
        value = 0;
        remainder = row_size - mat->width * mat->channel;
        
        for (i=0;i<mat->height;i++) {
            write_size = fwrite(mat->data[mat->height-i-1], 1, mat->width * mat->channel, f);
            // add padding
            for (j=0;j<remainder;j++) {
                write_size = fwrite(&value, 1, 1, f);
            }
        }
        fclose(f);
        return 0;
    } else {
        return -2;
    }
}
