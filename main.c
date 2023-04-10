/**
 *
 * CENG342 Project-1
 *
 * Downscaling 
 *
 * Usage:  main <input.jpg> <output.jpg> 
 *
 * @group_id 00
 * @author  your names
 *
 * @version 1.0, 02 April 2022
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#define CHANNEL_NUM 1

//Do not use global variables

// void seq_downscaling(uint8_t* input_image,int width, int height, ....);


int main(int argc,char* argv[]) 
{	
	MPI_Init(&argc,&argv);
    int width, height, bpp;
	
	// Reading the image in grey colors
    uint8_t* input_image = stbi_load(argv[1], &width, &height, &bpp, CHANNEL_NUM);
	
    printf("Width: %d  Height: %d \n",width,height);
	printf("Input: %s , Output: %s  \n",argv[1],argv[2]);
	
	// start the timer
	double time1= MPI_Wtime();	
	 
	//seq_downscaling(input_image,width, height,...);
	    
	double time2= MPI_Wtime();	
	printf("Elapsed time: %lf \n",time2-time1);	
	
	// Storing the image 
    stbi_write_jpg(argv[2], width, height, CHANNEL_NUM, input_image, 100);
    stbi_image_free(input_image);

	MPI_Finalize();
    return 0;
}

/*
void seq_downscaling(uint8_t* rgb_image,int width, int height)
{	
	
	// to access a pixel in 1-D image you can use the following for loop structure
	for(int i=0; i<height ; i++){
		for(int j=0; j<width; j++){
			// ... rgb_image[i*width + j] ...
		}
	}
}
*/
