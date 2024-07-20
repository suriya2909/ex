#include<stdio.h>
#include<stdlib.h>
#include "uType.h"
#include "encoder.h"
#include<string.h>

extern uchar decode_filename[MAX_FILENAME_SIZE];
extern uint default_ex;


Status start_decoding(EncoderInfo *enInfo){
    uchar_ptr get_letter=NULL;


    //decode file extension size
    uint dec_file_ext_size= decode_a_int(enInfo);
    if(dec_file_ext_size){
        printf("INFO: Extension size was decoded successfully.\n\n");

    }else{
        printf("ERROR: Error in decoding extention size\n\n");
        exit(1);
    }

    //decode dot
    get_letter=decode_string(CHAR_SIZE,enInfo);
    if(!strcmp((const char*)get_letter,(const char*)".")){
        printf("INFO: Decoded the dot successfully\n\n");
    }else{
        printf("ERROR: Error unable to decode the dot\n\n");
        exit(1);
    }

    //decode extention
    get_letter=decode_string(dec_file_ext_size,enInfo);

    if(get_letter){
        if(default_ex){
            // no extenstion provided in cmd arg
            strcpy((char*)enInfo->decode_filename,(const char*)"decoded.");
            strcat((char*)enInfo->decode_filename,(const char*)get_letter);
            printf("INFO: Creating default filename.\n\n");
        }else{
        
            if(validate_file_extn(decode_filename,enInfo))
                {
                    printf("INFO: Valid Secret filename ext.\n");
                }
            else{
                    printf("INFO: Invalid Secret filename ext.\n");
					return fail;
            }
            
            strcpy((char*)enInfo->decode_filename,(const char*)decode_filename);
            // read_val fun save the given file extention in eninfo
            if(!strcmp((const char*)enInfo->secret_file_ext,(const char*)get_letter)){
                printf("INFO: Extension matched\n\n");
            }else{
                printf("ERROR: Extension not matched\n\n");
				return fail;
            }
        }

    }else{

        printf("ERROR: Error in decoding file ext\n\n");
        exit(1);
    }

    //Decode secret data size
    if(enInfo->secret_file_size=decode_a_int(enInfo)){
        printf("INFO: secret data size decode successfully %d\n\n",enInfo->secret_file_size);
    }else{
        printf("ERROR: Failed to decode secret data size\n\n");
        return fail;
    }

    //open decode file for writing
    if((enInfo->fp_decode_file=fopen((const char*)enInfo->decode_filename,"wb"))!= NULL){
        printf("INFO: File opend successfully\n\n");

    }else{
        printf("ERROR: Unable to open the file %s",enInfo->decode_filename);
        return fail;
    }

    //decode secret text
    if(decode_secret_data(enInfo,enInfo->secret_file_size)){
        printf("INFO: Sceret data was decoded successfully\n\n");
    
    }else{
        printf("ERROR: Error in decoding secret data\n\n");
        return fail;
    }
    free(get_letter);
    return done;


}

uint decode_a_int(EncoderInfo *enInfo){
    uint decoded_int =0;
    for(int i=0;i<INT_SIZE;i++){
    uchar each_char=0;

        for(uint j=0;j<8;j++){
            //read one pixel
            fread(&each_char,sizeof(uchar),1,enInfo->fp_output_image_file);
            if (ferror(enInfo->fp_output_image_file))
            {
                printf("ERROR: Error in Reading File\n\n");
                exit(1);
            }
            //get the lsb of the pixel
            each_char &=01;
            //free space lsb
            decoded_int<<=1;
            //set lsb
            decoded_int |= (uint) each_char;
        }
    }
    return decoded_int;
    
}

Status decode_secret_data(EncoderInfo *enInfo,uint size){
    uchar_ptr decode_text=(uchar_ptr) malloc(sizeof(uchar)*size);
    if(decode_text==NULL) {
        printf("ERROR: Unable to create memory\n\n");
        exit(1);
    }
    uchar each_char;
    for(int i=0; i<size;i++){
        uchar one_byte=0;//consider as a string of 8 bit

        for(uint j=0;j<8;j++){

            fread(&each_char,sizeof(each_char),1,enInfo->fp_output_image_file);
            if(ferror(enInfo->fp_output_image_file)){
                printf("ERROR: Error in reading image file\n\n");
                return fail;
            }
            each_char &= 01;// get least bit
            one_byte<<=1;//free least bit to store each_char bit
            one_byte |= each_char;
        }
        decode_text[i]=one_byte;
    }
    fwrite(decode_text,size,1,enInfo->fp_decode_file);
    if(ferror(enInfo->fp_decode_file)){
        printf("ERROR: Error in writing file\n\n");
        return fail;
        }
    free(decode_text);
    return done;

}


uchar_ptr decode_string(uint size,EncoderInfo *enInfo){
    //create memory of decode string
    uchar_ptr decode_text=(uchar_ptr) malloc(sizeof(uchar)*size);
    if(decode_text==NULL) {
        printf("ERROR: Unable to create memory\n\n");
        exit(1);
    }
    uint i;
    uchar each_char;
    for( i=0; i<size;i++){
        uchar one_byte=0;//consider as a string of 8 bit

        for(uint j=0;j<8;j++){

            fread(&each_char,sizeof(each_char),1,enInfo->fp_output_image_file);
            each_char &= 01;// get least bit
            one_byte<<=1;//free least bit to store each_char bit
            one_byte |= each_char;
        }
        decode_text[i]=one_byte;
    }
    decode_text[i]='\0';
    return decode_text;
}