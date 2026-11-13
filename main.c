#include <stdio.h>
#include <string.h>

#include "encode.h"
#include "decode.h"
#include "types.h"


int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;

    /* Check minimum arguments */
    if(argc < 3)
    {
        printf("Insufficient arguments\n");
        printf("Encoding: ./a.out -e source.bmp secret.txt [stego.bmp]\n");
        printf("Decoding: ./a.out -d stego.bmp [output_file]\n");

        return e_failure;
    }

    /* Check operation type */
    OperationType operation = check_operation_type(argv);

    switch(operation)
    {
        case e_encode:

            printf("Encoding selected\n");


            if(read_and_validate_encode_args(argv, &encInfo) == e_success)
            {
                if(do_encoding(&encInfo) == e_success)
                {
                    printf("Encoding completed successfully\n");
                }
                else
                {
                    printf("Encoding failed\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Invalid encode arguments\n");
                return e_failure;
            }

            break;

            
        case e_decode:

            printf("Decoding selected\n");


            if(read_and_validate_decode_args(argv, &decInfo) == e_success)
            {
                if(do_decoding(&decInfo) == e_success)
                {
                    printf("Decoding completed successfully\n");
                }
                else
                {
                    printf("Decoding failed\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Invalid decode arguments\n");
                return e_failure;
            }

            break;



        case e_unsupported:

            printf("Unsupported operation\n");
            return e_failure;


    }


    return e_success;
}



/* Check encode or decode operation */
OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1], "-e") == 0)
    {
        return e_encode;
    }
    else if(strcmp(argv[1], "-d") == 0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}

