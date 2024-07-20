#include<stdio.h>
#include<string.h>
#include <libgen.h>
#include<stdlib.h>
#include "uType.h"
#include "encoder.h"

//passcode
uint default_ex=0;
//process image data
uint pixel_data;
//decode 
uchar decode_filename[MAX_FILENAME_SIZE];

op_type check_operation(char *argv[]){
    if(!(strcmp(*argv,"-e")))
            return enco;
    else if (!(strcmp(*argv,"-d")))
            return deco;
    else
            return unsupported;

}

int main(int argc, char* argv[]){
    EncoderInfo enInfo;// creating variable for structure

    if(argc<3){
        printf("ERROR: invalid number of inputs\n\n");
        exit(1);//default value of enum variable is 0
    }

    if (check_operation(argv + 1)==enco){
        
        if (argc < 4 || argc > 6) {
        printf("ERROR: invalid number of inputs\n\n");
			exit(1);
		} 
        
        //read and validate src_img filename
        argv[2]=basename(argv[2]);//remove the unwanted / and validate it
        if (validate_bmp_format(argv + 2)==fail){
				printf("ERROR: Invalid filename provided .It must be a '.bmp' file\n\n");
				exit(1);
        }
		printf("INFO: Valid source image filename.\n");
        strcpy((char *)enInfo.src_image_fname,argv[2]);


		//Extract only filename 
        argv[3]=basename(argv[3]);

        if(validate_file_extn((uchar_ptr)argv[3],&enInfo))
            {
		        printf("INFO: Valid Secret filename.\n");
            }
        else{
		        printf("INFO: Invalid Secret filename.\n");
                exit(1);
        }
        //store Secret filename
        strcpy((char*)enInfo.secret_fname,argv[3]);

        //reamain argument
        switch (argc)
        {
        case 4:
            
            // 4th argument is output file name is not specified
            // default filename is assigned
            strcpy((char*)enInfo.output_image_file_name,"output_image.bmp");
            printf("INFO: No output filename given. Creating default output image file %s in current directory\n", enInfo.output_image_file_name);
            break;
        case 5:
            argv[4]=basename(argv[4]);
            if(validate_bmp_format(argv+4)== fail){
                printf("ERROR: Invalid output image filename provided \n\n");
                exit(1);
            } 
			printf("INFO: Valid output image filename.\n");
            strcpy((char*)enInfo.output_image_file_name,argv[4]);
            break;

        default:
            //all the arguments are given
            // read valid and extract file name and .ext
            argv[4]=basename(argv[4]);
            if(validate_bmp_format(argv+4)==fail){
                printf("ERROR: Invalid output image filename provided.\n\n");
                exit(1);
            }
            strcpy((char*)enInfo.output_image_file_name,argv[4]);

        }

        //handiling files
        //opening all the necessary files
        if(open_f(&enInfo)==done){
            printf("All the required files are opend successfully.\n\n\n");
        }
        else{
            exit(1);
        }
        //process image
        printf("INFO: Processing image\n\n");
        //first 10 byte contains bitmap image information
        //going to 10 position by seting offset to 10 position 

        fseek(enInfo.fp_src_image,10L,SEEK_SET);
        // from 10 to 14 byte image file has Offset from beginning of file to the beginning of the bitmap data

        fread(&pixel_data,sizeof(pixel_data),1,enInfo.fp_src_image);//read 1 data and store in raster_data
        //fread does not distinguish between end-of-file and error.
        // so checking for errorn in reading and writing

        if(ferror(enInfo.fp_src_image)) {
            printf("ERROR: error in reading image file\n\n");
            exit(1);
        }

        rewind(enInfo.fp_src_image);
        //now copy image header information
        printf("INFO: Copying image header to %s\n\n",enInfo.output_image_file_name);

        if(cp_bmp_header((FILE*)enInfo.fp_src_image,(FILE*)enInfo.fp_output_image_file))
        {
            printf("INFO: Image header information was copied successfuly\n\n");
        }else{
            printf("ERROR: Image header information is not copied successfuly\n\n");
            exit(1);
        }

        // check secret string size is less than image size
        enInfo.image_size=find_image_size_bmp((FILE*)enInfo.fp_src_image);
        
        if(!enInfo.image_size){
            printf("ERROR: Source image is empty\n\n");
            exit(1);
        }

        enInfo.secret_file_size=find_file_size((FILE*)enInfo.fp_secret_text);
        //ckeck secret file

        if(!enInfo.secret_file_size){
            printf("ERROR: Secret file is empty\n\n");
            exit(1);
        }
        // enInfo.secret_file_size-CHAR_SIZE;

        
            //secret_file_ext_size+ . +length of secret_ext+data_size+secret_data
        enInfo.magic_string_size=(INT_SIZE+CHAR_SIZE+enInfo.secret_extn_len+INT_SIZE+enInfo.secret_file_size-CHAR_SIZE) *8;           
        enInfo.secret_file_size=enInfo.secret_file_size+1;

        if(!(check_capacity(&enInfo))){
            printf("ERROR: Image data size is insufficient to encode the secret data\n\n");
        }else{
            printf("INFO: Image data size is sufficient to encode the secret data\n\n");
        }

        printf("\t\t\t----------Encoding on process---------\n\n");
        if(start_encoding(&enInfo))
        {   
            printf("\t\t\t----------Encoded Successfully---------\n\n");
        }else{
            printf("\t\t\t----------Encoding Failed---------\n\n");
            exit(1);
        }

        fclose(enInfo.fp_src_image);
        fclose(enInfo.fp_secret_text);
    }
    else if (check_operation(argv+1)==deco){
        
        if(argc>6){
            printf("ERROR: Invalid arguments\n\n");
            exit (1);
        }

        argv[2]=basename(argv[2]);
        if(validate_bmp_format(argv+2)==fail){
            printf("ERROR: Invalid file format\n\n");
            exit(1);
        }
        printf("INFO: Valid image name\n\n");

        strcpy((char*)enInfo.output_image_file_name,(const char*)argv[2]);
        switch (argc)
        {
        case 3:
            //only image name is give so default text file extention be 1
            default_ex=1;
            break;
        case 4:
            //user give text file name
            argv[3]=basename(argv[3]);
            strcpy((char*)decode_filename,(const char*)argv[3]);
            break;
        default:
            break;
        }
        
        // open the encode image
        if((enInfo.fp_output_image_file=fopen((const char*)enInfo.output_image_file_name,"rb"))==NULL){
            printf("ERROR: unable to open %s\n\n",(char*)enInfo.output_image_file_name);
            exit(1);
        }

        printf("INFO: Input file is opened successfully\n\n");
        fseek(enInfo.fp_output_image_file,10L,SEEK_SET);
        fread(&pixel_data,sizeof(pixel_data),1,enInfo.fp_output_image_file);

        if(ferror(enInfo.fp_output_image_file)){
            printf("ERROR: Error in reading the file\n\n");
            exit(1);
        }

        //go to the position where image data starts
        fseek(enInfo.fp_output_image_file,pixel_data,SEEK_SET);

        printf("\t\t\t----------Decoding on process---------\n\n");
    
        if(start_decoding(&enInfo))
        {   
            printf("\t\t\t----------Decoded Successfully---------\n\n");
        }else{
            printf("\t\t\t----------Decoded Failed---------\n\n");
            exit(1);
        }
    }    

}