#ifndef ENCODER_H
#define ENCODER_H

#include "uType.h"


#define MAX_FILENAME_SIZE  50
#define MAX_FILE_SUFFIX    4
#define INT_SIZE           sizeof(int)
#define CHAR_SIZE          sizeof(char)
#define MAX_PASSCODE_LEN   4

typedef struct Encoder_Info
{   /*input image info*/
    uchar src_image_fname[MAX_FILENAME_SIZE];
    FILE *fp_src_image;
    uint image_size;
    /*secret filename info*/
    uchar secret_fname[MAX_FILENAME_SIZE];
    uchar secret_file_ext[MAX_FILE_SUFFIX + CHAR_SIZE];
    uint  secret_extn_len;
    FILE *fp_secret_text;
    uint secret_file_size;
    /*output file info*/
    uchar output_image_file_name[MAX_FILENAME_SIZE];
    FILE *fp_output_image_file;
    /*magic string size*/
    uint magic_string_size;
    // decode info
    FILE* fp_decode_file;
    uchar decode_filename[MAX_FILENAME_SIZE];
}EncoderInfo;

//function protypes

/* Read and validate Encode args from argv */
Status validate_bmp_format(char *argv[]);
Status validate_file_extn(uchar_ptr sec_filename,EncoderInfo *encInfo );
Status vaid_nondigit_pass(const char* ch);
Status open_f(EncoderInfo *enInfo);
Status cp_bmp_header(FILE *fp_src_image,FILE *fp_output_image);
Status find_image_size_bmp(FILE* src_image);
Status find_file_size(FILE* file);
Status check_capacity(EncoderInfo *enInfo);
Status start_encoding(EncoderInfo *enInfo);
Status encode_magic_string(const char *magic_string,EncoderInfo *enInfo);
Status encode_int_size(uint secret_len,EncoderInfo *enInfo);
Status copy_image_data(FILE* input_image,FILE* output_image,uint data_size);
Status start_decoding(EncoderInfo *enInfo);
uchar_ptr decode_string(uint size,EncoderInfo *enInfo);
Status decode_a_int(EncoderInfo *enInfo);
Status decode_secret_data(EncoderInfo *enInfo,uint size);



#endif