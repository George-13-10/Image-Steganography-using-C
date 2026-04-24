#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"

//Read and validate decode arguments
 
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strstr(argv[2], ".bmp") != NULL)
    {
        decInfo->stego_image_fname = argv[2];
    }
    else
    {
        return e_failure;
    }

    if(argv[3] == NULL)
    {
        decInfo->output_fname = "decoded.txt";
    }
    else
    {
        decInfo->output_fname = argv[3];
    }

    return e_success;
}

// Open stego image and output file
 
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if(decInfo->fptr_stego_image == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to open file %s\n", decInfo->stego_image_fname);
        return e_failure;
    }

    decInfo->fptr_output = fopen(decInfo->output_fname, "wb");
    if(decInfo->fptr_output == NULL)
    {
        perror("fopen");
        printf("ERROR: Unable to open file %s\n", decInfo->output_fname);
        return e_failure;
    }

    return e_success;
}

// Perform decoding
 
Status do_decoding(DecodeInfo *decInfo)
{
    if(open_decode_files(decInfo) != e_success)
    {
        printf("Failed to open files\n");
        return e_failure;
    }

    printf("Files opened successfully\n");

    /* Skip 54-byte BMP header */
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    /* Decode Magic String */
    if(decode_magic_string(decInfo) != e_success)
    {
        printf("Magic string mismatch\n");
        return e_failure;
    }
    printf("Magic string decoded successfully\n");

    /* Decode extension size */
    decode_secret_file_extn_size(decInfo);
    printf("Extension size decoded successfully\n");

    /* Decode extension */
    decode_secret_file_extn(decInfo);
    printf("Extension decoded successfully\n");

    /* Decode secret file size */
    decode_secret_file_size(decInfo);
    printf("Secret file size decoded successfully\n");

    /* Decode secret file data */
    decode_secret_file_data(decInfo);
    printf("Secret file data decoded successfully\n");

    printf("Decoding is successful\n");

    return e_success;
}

// Decode Magic String
 
Status decode_magic_string(DecodeInfo *decInfo)
{
    char buffer[8];
    char magic[10];

    for(int i = 0; i < strlen(MAGIC_STRING); i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&magic[i], buffer);
    }

    magic[strlen(MAGIC_STRING)] = '\0';

    if(strcmp(magic, MAGIC_STRING) == 0)
        return e_success;
    else
        return e_failure;
}

// Decode extension size
 
Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    decode_size_from_lsb(&decInfo->extn_size,decInfo->fptr_stego_image);
    return e_success;
}

// Decode extension

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];

    for(int i = 0; i < decInfo->extn_size; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&decInfo->extn_secret_file[i], buffer);
    }

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    return e_success;
}

//Decode secret file size
 
Status decode_secret_file_size(DecodeInfo *decInfo)
{
    decode_size_from_lsb((int *)&decInfo->size_secret_file,decInfo->fptr_stego_image);
    return e_success;
}

// Decode secret file data
 
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;

    for(int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_stego_image);
        decode_byte_from_lsb(&ch, buffer);
        fwrite(&ch, 1, 1, decInfo->fptr_output);
    }

    return e_success;
}

//Decode 32-bit integer from LSB
Status decode_size_from_lsb(int *size, FILE *fptr)
{
    char buffer[32];

    fread(buffer, 32, 1, fptr);

    *size = 0;

    for(int i = 0; i < 32; i++)
    {
        *size |= ((buffer[i] & 1) << i);
    }

    return e_success;
}

// Decode single byte from LSB
 
Status decode_byte_from_lsb(char *ch, char *buffer)
{
    *ch = 0;

    for(int i = 0; i < 8; i++)
    {
        *ch |= ((buffer[i] & 1) << i);
    }

    return e_success;
}
