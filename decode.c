#include <stdio.h>
#include <string.h>

#include "decode.h"
#include "types.h"
#include "common.h"
/* Validate decode arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *ext;
    ext = strrchr(argv[2], '.');
    if (ext == NULL || strcmp(ext, ".bmp") != 0)
    {
        printf("ERROR: Stego image should be .bmp file\n");
        return e_failure;
    }
    decInfo->stego_image_fname = argv[2];
    if (argv[3] != NULL)
    {
        decInfo->output_fname = argv[3];
    }
    else
    {
        printf("INFO: Output File not mentioned. Creating decoded.txt as default\n");
        decInfo->output_fname = "decoded.txt";
    }
    return e_success;
}

/* Open files */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");

    if (decInfo->fptr_stego_image == NULL)
    {
        perror("Stego image");
        return e_failure;
    }

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    decInfo->fptr_output = fopen(decInfo->output_fname, "w");

    if (decInfo->fptr_output == NULL)
    {
        perror("Output file");
        return e_failure;
    }
    return e_success;
}

/* Decode one byte */
char decode_byte_from_lsb(char *image_buffer)
{
    char data = 0;

    for (int i = 0; i < 8; i++)
    {
        data |= ((image_buffer[i] & 1) << i);
    }

    return data;
}

/* Decode integer */
int decode_int_from_lsb(char *image_buffer)
{
    int data = 0;

    for (int i = 0; i < 32; i++)
    {
        data |= ((image_buffer[i] & 1) << i);
    }

    return data;
}

/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char magic[10];

    for (int i = 0; i < strlen(MAGIC_STRING); i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        magic[i] = decode_byte_from_lsb(image_buffer);
    }

    magic[strlen(MAGIC_STRING)] = '\0';

    if (strcmp(magic, MAGIC_STRING) == 0)
        return e_success;

    return e_failure;
}

/* Decode extension size */
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo)
{
    char image_buffer[32];

    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);

    *size = decode_int_from_lsb(image_buffer);

    return e_success;
}

/* Decode extension */
Status decode_secret_file_extn(char *extn, int size, DecodeInfo *decInfo)
{
    char image_buffer[8];

    for (int i = 0; i < size; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);
        extn[i] = decode_byte_from_lsb(image_buffer);
    }

    extn[size] = '\0';

    return e_success;
}

/* Decode file size */
Status decode_secret_file_size(long *size, DecodeInfo *decInfo)
{
    char image_buffer[32];

    fread(image_buffer, 32, 1, decInfo->fptr_stego_image);

    *size = decode_int_from_lsb(image_buffer);

    return e_success;
}
/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;

    for(long i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image);

        ch = decode_byte_from_lsb(image_buffer);

        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }
    return e_success;
}

/* Perform decoding */

Status do_decoding(DecodeInfo *decInfo)
{
    int extn_size;

    printf("\nINFO: ## Decoding Procedure Started ##\n");

    printf("INFO: Opening required files\n");

    if(open_decode_files(decInfo) == e_failure)
    {
        printf("ERROR: Unable to open files\n");
        return e_failure;
    }

    printf("INFO: Opened %s\n", decInfo->stego_image_fname);
    printf("INFO: Opened %s\n", decInfo->output_fname);
    printf("INFO: Done. Opened all required files\n");

    printf("INFO: Decoding Magic String Signature\n");

    if(decode_magic_string(decInfo) == e_failure)
    {
        printf("ERROR: Invalid Magic String\n");
        return e_failure;
    }

    printf("INFO: Done\n");

    printf("INFO: Decoding Output File Extension\n");

    if(decode_secret_file_extn_size(&extn_size, decInfo) == e_failure)
        return e_failure;

    if(decode_secret_file_extn(decInfo->extn_secret_file,extn_size,decInfo) == e_failure)
        return e_failure;

    printf("INFO: Done\n");

    printf("INFO: Decoding %s File Size\n",decInfo->output_fname);

    if(decode_secret_file_size(&decInfo->size_secret_file,decInfo) == e_failure)
        return e_failure;

    printf("INFO: Done\n");

    printf("INFO: Decoding %s File Data\n",decInfo->output_fname);

    if(decode_secret_file_data(decInfo) == e_failure)
        return e_failure;

    printf("INFO: Done\n");

    fclose(decInfo->fptr_stego_image);
    fclose(decInfo->fptr_output);

    printf("\nINFO: ## Decoding Done Successfully ##\n");

    return e_success;
}