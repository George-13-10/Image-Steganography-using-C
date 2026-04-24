#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

Status read_and_validate_encode_args(char *argv[],EncodeInfo *encInfo)
{
    if(strstr(argv[2],".bmp") != NULL)
    {
        encInfo->src_image_fname=argv[2];
    }
    else
    {
        return e_failure;
    }

    if(strstr(argv[2],"bmp") != NULL)
    {
        encInfo->secret_fname=argv[3];

    }
    else
    {
        return e_failure;
    }
    if(argv[4]==NULL)
    {
        encInfo->stego_image_fname="default.bmp";

    }
    else
    {
        encInfo->stego_image_fname = argv[4];
    }
    
return e_success;
}

uint get_file_size(FILE *fptr)
{
/*find size*/
uint size;

    fseek(fptr, 0, SEEK_END);   // Move to end
    size = ftell(fptr);         // Get file size
    rewind(fptr);               // Go back to beginning

    return size;

}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    if(encInfo->image_capacity > 16+32+32+32+ ( encInfo->size_secret_file * 8))
    return e_success;
    else
    return e_failure;
}

Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo)==e_success)
    {
        printf("Files are opened sucessfully\n");
        if(check_capacity(encInfo) == e_success)
        {
            printf("Secret data can fit in image file\n");
        }
        else
        {
            printf("Image size is not sufficient enough to fit secret data\n");
            return e_failure;
        }
    }
    else
    {
        printf("Failed to open file\n");
        return e_failure;
    }
    if((copy_bmp_header(encInfo -> fptr_src_image,encInfo -> fptr_stego_image)) == e_success)
    {
        printf("Copy bmpheader is success.\n");
    }
    else
    {
        printf("Copy bmp header is a failure\n");
    }
    if(encode_magic_string(MAGIC_STRING, encInfo)== e_success)
    {
        printf("Magic string is a success\n");
    }
    else
    {
        printf("Magic string is a failure\n");
    }

    if(encode_secret_file_extn_size(4 , encInfo) == e_success)
    {
        printf("Encoding Extn. size is a success\n");
    }
    else
    {
        printf("Encoding Extn. size is a failure\n");

    }
    if(encode_secret_file_extn(".txt", encInfo) == e_success)
    {
        printf("Encoding secret file extension is a success\n");

    }
    else
    {
        printf("Encoding secret file extension is a failure\n");

    }
    if(encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
    {
        printf("Encoding secret file is a success\n");
    }
    else
    {
        printf("Encoding secret file is a failure\n");

    }
    if(encode_secret_file_data(encInfo)== e_success)
    {
        printf("Encoding secret file data is a sucess\n");
    }
    else
    {
        printf("Encoding secret file data is a failure\n");
        return e_failure;

    }
    if(copy_remaining_img_data(encInfo -> fptr_src_image, encInfo-> fptr_stego_image)==e_success)
    {
        printf("Copy remaining data is a success\n");
    }
    else
    {
        printf("Copy remaining data is a failure\n");
        return e_failure;

    }

    
    return e_success;

}

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while(fread(&ch,1,1,fptr_src) > 0)
    {
        fwrite(&ch,1,1,fptr_dest);
    } 
    return e_success;
}

/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char str[encInfo -> size_secret_file];
    fseek(encInfo -> fptr_secret, 0,SEEK_SET);
    fread(str,1,encInfo -> size_secret_file,encInfo -> fptr_secret);
    encode_data_to_image(str,encInfo->size_secret_file,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;

}

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    encode_size_to_lsb(file_size,encInfo);
    return e_success;
}


/* Encode secret file extenstion */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image(file_extn,strlen(file_extn),encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success;
}

/* Encode secret file extenstion  size*/
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    encode_size_to_lsb(4,encInfo);
    return e_success;
}

/* Encode a size into LSB of image data array */
Status encode_size_to_lsb(int data, EncodeInfo*encInfo)
{
    char buffer[32];
    fread(buffer, 32,1, encInfo -> fptr_src_image);
    for(int i=0;i<32;i++)
    {
        buffer[i] = (buffer[i] & 0xFE) | ((data >> i) & 1);
    }
    fwrite(buffer,32 ,1, encInfo-> fptr_stego_image);

    return e_success;
}

/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(magic_string, strlen(magic_string), encInfo -> fptr_src_image, encInfo -> fptr_stego_image);
    return e_success;

}

/* Encode function, which does the real encoding */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char arr[8];
    for(int i=0;i<size;i++)
    {
        fread(arr,8,1, fptr_src_image);
        encode_byte_to_lsb(data[i], arr);
        fwrite(arr,8,1,fptr_stego_image);
    }
    return e_success;
}

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int i=0;i<8;i++)
    {
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data >> i) & 1);
    }
    return e_success;

}

/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char arr[54];
    fseek(fptr_src_image,0,SEEK_SET);
    fread(arr,54,1,fptr_src_image);
    fwrite(arr,54,1,fptr_dest_image);
    return e_success;

}
