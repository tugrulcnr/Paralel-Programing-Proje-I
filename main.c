/**
*
CENG342 Project-1
Downscaling
Usage: mpirun -n <num_processes> main <input.jpg> <output.jpg>
@group_id 00
@author your names
@version 1.0, 02 April 2022
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION


#define CHANNEL_NUM 1
#define FILTER_SIZE 5

void get_downscale_block(int rank, int num_processes, int height, int width, int *row_start, int *row_end)
{
    int block_height = height / num_processes;
    int remainder = height % num_processes;
    *row_start = rank * block_height;
    if (rank < remainder)
    {
        *row_start += rank;
        *row_end = *row_start + block_height + 1;
    }
    else
    {
        *row_start += remainder;
        *row_end = *row_start + block_height;
    }
}

void apply_filter(uint8_t *input_block, int width, int height, float filter[FILTER_SIZE][FILTER_SIZE], uint8_t *output_block)
{
    int output_width = width / 2;
    int output_height = height / 2;
    int filter_offset = FILTER_SIZE / 2;
    for (int i = 0; i < output_height; i++)
    {
        for (int j = 0; j < output_width; j++)
        {
            float pixel_value = 0.0;
            for (int k = 0; k < FILTER_SIZE; k++)
            {
                for (int l = 0; l < FILTER_SIZE; l++)
                {
                    int x = j * 2 + l - filter_offset;
                    int y = i * 2 + k - filter_offset;
                    if (x >= 0 && x < width && y >= 0 && y < height)
                    {
                        pixel_value += input_block[y * width + x] * filter[k][l];
                    }
                }
            }
            output_block[i * output_width + j] = (uint8_t)roundf(pixel_value);
        }
    }
}
void seq_downscaling(uint8_t *input_image, int width, int height, uint8_t *output_image)
{
    float filter[FILTER_SIZE][FILTER_SIZE] = {
        {0.0625, 0.125, 0.0625},
        {0.125, 0.25, 0.125},
        {0.0625, 0.125, 0.0625}};
    for (int i = 0; i < height / 2; i++)
    {
        for (int j = 0; j < width / 2; j++)
        {
            float pixel_value = 0.0;
            for (int k = 0; k < FILTER_SIZE; k++)
            {
                for (int l = 0; l < FILTER_SIZE; l++)
                {
                    int x = j * 2 + l - 1;
                    int y = i * 2 + k - 1;
                    pixel_value += input_image[y * width + x] * filter[k][l];
                }
            }
            output_image[i * (width / 2) + j] = (uint8_t)roundf(pixel_value);
        }
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int width, height, bpp;
    uint8_t *input_image;

    if (rank == 0)
    {
        // Read the input image
        input_image = stbi_load(argv[1], &width, &height, &bpp, CHANNEL_NUM);

        printf("Width: %d  Height: %d \n", width, height);
        printf("Input: %s , Output: %s  \n", argv[1], argv[2]);
    }

    // Broadcast the image dimensions to all processes
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Calculate the new image dimensions
    int new_width = width / 2;
    int new_height = height / 2;

    // Allocate memory for the downsampled image and output image on each process
    uint8_t *downsampled_image = (uint8_t *)malloc(sizeof(uint8_t) * new_width * new_height * CHANNEL_NUM);
    uint8_t *output_image = (uint8_t *)malloc(sizeof(uint8_t) * new_width * new_height * CHANNEL_NUM);

    // Scatter the input image to all processes
    MPI_Scatter(input_image, new_width * new_height * CHANNEL_NUM, MPI_UINT8_T,
                downsampled_image, new_width * new_height * CHANNEL_NUM, MPI_UINT8_T,
                0, MPI_COMM_WORLD);

    // Call the sequential downscaling algorithm
    seq_downscaling(downsampled_image, new_width, new_height, output_image);

    // Gather the output image from all processes to the root process
    MPI_Gather(output_image, new_width * new_height * CHANNEL_NUM, MPI_UINT8_T,
               input_image, new_width * new_height * CHANNEL_NUM, MPI_UINT8_T,
               0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        // Store the output image
        stbi_write_jpg(argv[2], new_width, new_height, CHANNEL_NUM, input_image, 100);
        stbi_image_free(input_image);
    }

    MPI_Finalize();
    return 0;
}
