#include <stdio.h>
#include <string.h>

#include "encode.h"
#include "types.h"
#include "common.h"


/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;

    fseek(fptr_image, 18, SEEK_SET);

    fread(&width, sizeof(int), 1, fptr_image);

    fread(&height, sizeof(int), 1, fptr_image);


    printf("Width = %u\n", width);
    printf("Height = %u\n", height);

    return width * height * 3;
}


Status open_files(EncodeInfo *encInfo)
{
    printf("Source : %s\n", encInfo->src_image_fname);
    printf("Secret : %s\n", encInfo->secret_fname);
    printf("Stego  : %s\n", encInfo->stego_image_fname);

    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    if (encInfo->fptr_src_image == NULL)
    {
        perror("Source image");
        return e_failure;
    }

    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    if (encInfo->fptr_secret == NULL)
    {
        perror("Secret file");
        return e_failure;
    }

    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    if (encInfo->fptr_stego_image == NULL)
    {
        perror("Stego image");
        return e_failure;
    }

    return e_success;
}

/* Check image capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    uint image_size;


    image_size = get_image_size_for_bmp(encInfo->fptr_src_image);

    fseek(encInfo->fptr_secret, 0, SEEK_END);

    encInfo->size_secret_file = ftell(encInfo->fptr_secret);

    rewind(encInfo->fptr_secret);

    int required_size;

    required_size = (strlen(MAGIC_STRING) + sizeof(int) + strlen(encInfo->extn_secret_file) + sizeof(long)+ encInfo->size_secret_file) * 8;


    if(image_size >= required_size)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}


/* Copy BMP header */
Status copy_bmp_header(FILE *fptr_src_image,FILE *fptr_dest_image)
{
    char header[54];

    rewind(fptr_src_image);
    fread(header,54,1,fptr_src_image);
    fwrite(header,54,1,fptr_dest_image);

    return e_success;
}

/* Encode byte into LSB */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i = 0; i < 8; i++)
    {
        image_buffer[i] = (image_buffer[i] & ~1) |((data >> i) & 1);
    }

    return e_success;
}


/* Encode string into image */
Status encode_string_to_image(char *str, int size,FILE *fptr_src_image,FILE *fptr_dest_image)
{
    char image_buffer[8];


    for(int i = 0; i < size; i++)
    {
        fread(image_buffer, 8, 1, fptr_src_image);


        encode_byte_to_lsb(str[i], image_buffer);


        fwrite(image_buffer, 8, 1, fptr_dest_image);
    }

    return e_success;
}



/* Encode magic string */
Status encode_magic_string(const char *magic_string,EncodeInfo *encInfo)
{
    encode_string_to_image((char *)magic_string,strlen(magic_string),encInfo->fptr_src_image,encInfo->fptr_stego_image);

    return e_success;
}


/* Encode integer into image */
Status encode_int_to_image(int data,FILE *fptr_src_image,FILE *fptr_dest_image)
{
    char image_buffer[32];

    fread(image_buffer, 32, 1, fptr_src_image);

    for(int i = 0; i < 32; i++)
    {
        image_buffer[i] = (image_buffer[i] & ~1) |((data >> i) & 1);
    }

    fwrite(image_buffer, 32, 1, fptr_dest_image);

    return e_success;
}


/* Encode secret file extension size */
Status encode_secret_file_extn_size(int extn_size,EncodeInfo *encInfo)
{
    encode_int_to_image(extn_size,encInfo->fptr_src_image,encInfo->fptr_stego_image);

    return e_success;
}



/* Encode secret file extension */
Status encode_secret_file_extn(const char *file_extn,EncodeInfo *encInfo)
{
    encode_string_to_image((char *)file_extn,strlen(file_extn),encInfo->fptr_src_image,encInfo->fptr_stego_image);

    return e_success;
}

/* Encode secret file size */
Status encode_secret_file_size(long file_size,EncodeInfo *encInfo)
{
    encode_int_to_image(file_size,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;
}

  /* Read and validate encode arguments */
Status read_and_validate_encode_args(char *argv[],EncodeInfo *encInfo)
{
    char *ext;


    /* Source image validation */
    ext = strrchr(argv[2], '.');

    if(ext == NULL || strcmp(ext, ".bmp") != 0)
    {
        printf("Source image should be .bmp file\n");
        return e_failure;
    }
    encInfo->src_image_fname = argv[2];

    /* Secret file */
    encInfo->secret_fname = argv[3];
    ext = strrchr(argv[3], '.');


    if(ext == NULL)
    {
        printf("Secret file extension missing\n");
        return e_failure;
    }


    strcpy(encInfo->extn_secret_file, ext);

    /* Output file */
    if(argv[4] != NULL)
    {
        ext = strrchr(argv[4], '.');


        if(ext == NULL || strcmp(ext, ".bmp") != 0)
        {
            printf("Output file should be .bmp\n");
            return e_failure;
        }


        encInfo->stego_image_fname = argv[4];

    }
    else
    {
        encInfo->stego_image_fname = "stego.bmp";
    }



    return e_success;
}


/* Encode secret file data */
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;


    rewind(encInfo->fptr_secret);


    for(long i = 0; i < encInfo->size_secret_file; i++)
    {
        fread(&ch, 1, 1, encInfo->fptr_secret);


        encode_string_to_image(&ch,1,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    }


    return e_success;
}

/* Copy remaining image data */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;

    while(fread(&ch, 1, 1, fptr_src))
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }


    return e_success;
}
Status do_encoding(EncodeInfo *encInfo)
{
    printf("INFO: ## Encoding Procedure Started ##\n");

    printf("INFO: Opening required files\n");

    if(open_files(encInfo) == e_failure)
    {
        printf("ERROR: Unable to open required files\n");
        return e_failure;
    }

    printf("INFO: Opened %s\n", encInfo->src_image_fname);
    printf("INFO: Opened %s\n", encInfo->secret_fname);
    printf("INFO: Opened %s\n", encInfo->stego_image_fname);
    printf("INFO: Done\n");

    printf("INFO: Checking for %s size\n", encInfo->secret_fname);

    if(encInfo->size_secret_file == 0)
    {
        fseek(encInfo->fptr_secret, 0, SEEK_END);
        encInfo->size_secret_file = ftell(encInfo->fptr_secret);
        rewind(encInfo->fptr_secret);
    }

    if(encInfo->size_secret_file == 0)
    {
        printf("ERROR: Secret file is empty\n");
        return e_failure;
    }

    printf("INFO: Done. Not Empty\n");

    printf("INFO: Checking for %s capacity to handle %s\n",encInfo->src_image_fname,encInfo->secret_fname);

    if(check_capacity(encInfo) == e_failure)
    {
        printf("ERROR: Insufficient Image Capacity\n");
        return e_failure;
    }

    printf("INFO: Done. Found OK\n");

    printf("INFO: Copying Image Header\n");
    copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image);
    printf("INFO: Done\n");

    printf("INFO: Encoding Magic String Signature\n");
    encode_magic_string(MAGIC_STRING, encInfo);
    printf("INFO: Done\n");

    printf("INFO: Encoding %s File Extension\n",
            encInfo->secret_fname);
    encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo);
    encode_secret_file_extn(encInfo->extn_secret_file,encInfo);
    printf("INFO: Done\n");

    printf("INFO: Encoding %s File Size\n",encInfo->secret_fname);
    encode_secret_file_size(encInfo->size_secret_file,encInfo);
    printf("INFO: Done\n");

    printf("INFO: Encoding %s File Data\n",encInfo->secret_fname);
    encode_secret_file_data(encInfo);
    printf("INFO: Done\n");

    printf("INFO: Copying Left Over Data\n");
    copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image);
    printf("INFO: Done\n");

    printf("INFO: ## Encoding Done Successfully ##\n");

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}


