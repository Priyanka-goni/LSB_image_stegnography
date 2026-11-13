#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include "types.h"


#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;


    /* Output File Info */
    char *output_fname;
    FILE *fptr_output;


    /* Secret File Info */
    char extn_secret_file[MAX_FILE_SUFFIX];
    long size_secret_file;


} DecodeInfo;

/* Read and validate decode arguments */
Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo);


/* Open decode files */
Status open_decode_files(DecodeInfo *decInfo);


/* Perform decoding */
Status do_decoding(DecodeInfo *decInfo);


/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo);


/* Decode secret file extension size */
Status decode_secret_file_extn_size(int *extn_size, DecodeInfo *decInfo);


/* Decode secret file extension */
Status decode_secret_file_extn(char *extn,int size,DecodeInfo *decInfo);


/* Decode secret file size */
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo);


/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo);


/* Decode byte from LSB */
char decode_byte_from_lsb(char *image_buffer);


/* Decode integer from LSB */
int decode_int_from_lsb(char *image_buffer);

#endif