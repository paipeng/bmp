//
//  cp_bmp.h
//  CPCodeLib
//
//  Created by Pai Peng on 2021/8/12.
//

#ifndef cp_bmp_h
#define cp_bmp_h

#include <stdio.h>

#if defined(__GNUC__)
#define CP_EXPORTF
#else
#define CP_EXPORTF __declspec(dllexport)
#endif

#pragma pack(1)

typedef struct _cp_mat {
    unsigned int width;
    unsigned int height;
    unsigned int channel;
    unsigned char** data;
} cp_mat;


#pragma pack()

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif
CP_EXPORTF cp_mat* init_cp_mat(unsigned int width, unsigned int height, unsigned int channel);
CP_EXPORTF void free_cp_mat(cp_mat* mat);
// Read BMP images
CP_EXPORTF cp_mat* cp_read_bmp_image(char* file);
CP_EXPORTF int cp_write_bmp_image(char* file, cp_mat* mat, int dpi);
#if defined(c_plusplus) || defined(__cplusplus)
} /* extern "C" */
#endif


#endif /* cp_bmp_h */
