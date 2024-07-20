#include<stdio.h>
#include<stdlib.h>
#include "uType.h"
#include "encoder.h"
#include<string.h>


extern uint pixel_data;
//function defenition

Status validate_bmp_format(char *argv[]) {
    //pointer to store the address of .bmp
    const char* bmp_holder= strstr(*argv,".bmp");
    if(bmp_holder)
    {
        return(!(strcmp(bmp_holder,".bmp"))) ? done : fail;
    }
    return fail;

}

Status validate_file_extn(uchar_ptr sec_filename,EncoderInfo *encInfo )
{   
    //pointer to hold file name 
    uchar_ptr sec_ = (uchar_ptr) malloc(strlen((const char*)sec_filename)+1);
    //checking memory is created
    if(sec_==NULL)
    {
        printf("ERROR: Unable to create dynamic memory ");
        return fail;
    }

    //store file name in the memory heap created
    strcpy((char*)sec_,(const char*)sec_filename);
    //length of the file name
    uint secret_filename_len=strlen((const char*)sec_);
    //get the file name without extension
    char* ext =strtok((char*)sec_,".");
    //check for the presence of entension by checking dot is present
    if(strlen(ext)==secret_filename_len){
        //no dot is present
        printf("ERROR:Invalid file extension");
        return fail;
    }

    //get the file extension
    ext=strtok(NULL,".");//NULL contains the remaining text after the dot
    
    strcpy((char*)(encInfo->secret_file_ext),(const char*)ext);
    encInfo->secret_extn_len=strlen((const char*)encInfo->secret_file_ext);

    //check for valid .ext
    if(encInfo->secret_extn_len>MAX_FILE_SUFFIX)
    {
        printf("ERROR: Unsupported and Invalid file extension");
        return fail;
    }

    free(sec_);// free dynamically accol memory
    return done;
}


Status open_f(EncoderInfo *enInfo) {
     //open source file
     if((enInfo->fp_src_image = fopen((const char*)enInfo->src_image_fname,"rb"))==NULL){
        perror("openf ERROR ");
        fprintf(stderr,("ERROR: Unable to open %s",enInfo->src_image_fname));
        return fail;
     }

     //open secret file 
     if((enInfo->fp_secret_text=fopen((const char*)enInfo->secret_fname,"rb"))==NULL) {
        perror("openf ERROR ");
        fprintf(stderr,("ERROR: Unable to open %s",enInfo->secret_fname));
        return fail;     
     }

     //open output image file
     if((enInfo->fp_output_image_file=fopen((const char*)enInfo->output_image_file_name,"wb"))==NULL){
        perror("openf ERROR ");
        fprintf(stderr,("ERROR: Unable to open %s",enInfo->output_image_file_name));
        return fail;
     }
     // no error all files open succesfully
     return done;
} 

Status cp_bmp_header(FILE *fp_src_image,FILE *fp_output_image){
    //create a memory block to store image information
    uchar_ptr header_data= (uchar_ptr) malloc(sizeof(uchar)*(pixel_data));
    if(header_data==NULL){
        printf("ERROR: unable to create memory for header data");
        exit(1);
    }
    
    // size of one header file
    fread(header_data,pixel_data,1,fp_src_image);
    if(ferror(fp_src_image)){
        printf("ERROR: Error in reading source image");
        return fail;
    }
    //write data to the output image
    fwrite(header_data,pixel_data,1,fp_output_image);
    if(ferror(fp_output_image)){
        printf("ERROR: Error in writing output image");
        return fail;
    }

    free(header_data);
    return done;
}

// size of bmp image
Status find_image_size_bmp(FILE* src_image){
    uint image_size;
    //image size info is in the offest of a0022h , which is 34 byte
    fseek(src_image,34L,SEEK_SET);
    
    // image information is four byte
    fread(&image_size,sizeof(image_size),1,src_image);
    if(ferror(src_image)){
        printf("ERROR: Error in reading source image(size)");
        return fail;
    }
    // no error

    return image_size;
}

// size of a file
Status find_file_size(FILE* file){
    //go to the end of the file
    fseek(file,0L,SEEK_END);
    //ftell says how many byte it moved

    return (uint)ftell(file);
}

Status check_capacity(EncoderInfo *enInfo){
    return (enInfo->magic_string_size>enInfo->image_size? fail:done);

}

Status start_encoding(EncoderInfo *enInfo){
    //encode magic signature
    
    printf("INFO: Encoding magic signature");
    fseek(enInfo->fp_src_image,pixel_data,SEEK_SET);

    //encode secret file extension len
    printf("INFO: Enoding secret file extension length\n\n");
    if(encode_int_size(enInfo->secret_extn_len,enInfo)){
        printf("INFO: Secret file extension encode succesfully\n\n");
    }else{
        printf("ERROR: Secret file extension is not encode succesfully\n\n");
        return fail;   
    }

    //encode dot
    if(encode_magic_string(".",enInfo)){
        printf("INFO: Dot encoded successfully\n\n");
    }else{
        printf("ERROR: Dot encoding failed\n\n");
        return fail;
    }

    //encode secret file extension
    if(encode_magic_string((const char*)enInfo->secret_file_ext,enInfo)){
        printf("INFO: secret file entension encoded successfully\n\n");
    }else{
        printf("ERROR: secret file entension encoding failed\n\n");
        return fail;
    }

    //encode secret file size
    printf("INFO: Enoding secret file size\n\n");
    if(encode_int_size(enInfo->secret_file_size-CHAR_SIZE,enInfo)){
        printf("INFO: Secret file size encode succesfully\n\n");
    }else{
        printf("ERROR: Secret file size is not encode succesfully\n\n");
        return fail;   
    }

    //encode secret text
    //create memory to store secret data
    uchar_ptr secret_data= (uchar_ptr)malloc((sizeof(uchar)*enInfo->secret_file_size));

    if(secret_data==NULL){
        printf("ERROR: unable to create memory");
        return fail;
    }
    rewind(enInfo->fp_secret_text);
    fread(secret_data,sizeof(uchar)*enInfo->secret_file_size-CHAR_SIZE,1,enInfo->fp_secret_text);
    secret_data[enInfo->secret_file_size+CHAR_SIZE]='\0';

    if(encode_magic_string((const char*)secret_data,enInfo)){
        printf("INFO: Secret text was encoded successfully\n\n");
    }else{
        printf("ERROR: Error while encoding secret text\n\n");
        return fail;
    }
    free(secret_data);

    // copy image data                                                                     orginal image - (all the above encode data in img)+'\0'           
    if(copy_image_data((FILE*)enInfo->fp_src_image,(FILE*)enInfo->fp_output_image_file,enInfo->image_size-enInfo->magic_string_size+CHAR_SIZE)){
        printf("INFO: Image data was encoded successfully\n\n");
    }else{
        printf("ERROR: Error while encoding Image data\n\n");
        return fail;
    }

return done;
}

Status copy_image_data(FILE* input_image,FILE* output_image,uint data_size){
    uchar_ptr data= (uchar_ptr) malloc(sizeof(uchar)*data_size);
    if(data==NULL)
    {
        printf("ERROR: Unable to create memory\n\n");
        return fail;
    } 

    fread(data,(data_size),1,input_image);
    if(ferror(input_image)){
        printf("ERROR: Error while Reading file\n\n");
        return fail;
    }

    //write the reaming data
    fwrite(data,data_size,1,output_image);
    if(ferror(output_image)){
        printf("ERROR: while writing file\n\n");
        return fail;
    }
    free(data);
    return done;

}
    







Status encode_int_size(uint secret_len,EncoderInfo *enInfo){
    uchar each_char;

    // read every byte
    // uint has 4 byte each byte has 8 bit total 32bit 
    //              31         0
    for(int n=(INT_SIZE*8-1);n>=0;n--){// runs 32 times to read each bit of secert len

        fread(&each_char,sizeof(each_char),1,enInfo->fp_src_image);
        if(ferror(enInfo->fp_src_image)){
                printf("ERROR: Error while reading from source image file.\n\n");
				return fail;
        }

        //in 1 byte of image stores 1 bit of secret_len
        each_char &= 0xFE;//delete last bit
            
    // to get the current bit
        if(secret_len & (1<<n)){
            each_char |= 01;
        }else{
            each_char |= 00;
        }

    //write the image byte in the output file
        fwrite(&each_char,sizeof(each_char),1,enInfo->fp_output_image_file);
        if (ferror(enInfo->fp_output_image_file))
        {
            printf("ERROR: Error while writing into output image file.\n\n");
            return fail;
        }
    }
    return done;
}


Status encode_magic_string( const char *magic_string,EncoderInfo *enInfo){
    uchar each_char;
    //for each letter
    for(uint i=0;i<strlen(magic_string);i++){
        //for each bit of letter
        //8 bit in a character
        for(int j=7;j>=0;j--){

            //read the image file of each byte(8bit)
            fread(&each_char,sizeof(each_char),1,enInfo->fp_src_image);
            if(ferror(enInfo->fp_src_image)){
                printf("ERROR: Error in reading input image\n\n");
                return fail;
            }

            //clear the last bit 
            //0xFE=11111110 using it with & clear the least significant bit
            each_char &= 0xFE;

            //when j=8 and else part execute 
                                    //0
            if(magic_string[i] & (01 << j)){
                each_char |=01;
            }else{
                each_char |=00;
            }

            //write the byte into output image

            fwrite(&each_char,sizeof(each_char),1,enInfo->fp_output_image_file);

            if(ferror(enInfo->fp_output_image_file)){
				printf("ERROR: Error while writing onto output image file.\n\n");
				return fail;                
            }

        }

    }
    return done;
}
