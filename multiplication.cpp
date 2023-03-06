#include <iostream>
#include "include/matrix.hpp"
#include "include/utils.hpp"
#include <cstring>
#include <math.h>

inline uint8_t validate_args(int, char **);

int main(int argc, char **argv)
{
    try
    {
        if (validate_args(argc, argv))
        {
            std::cout << "invalid params were set" << std::endl;
            return -1;
        }

        const char *filename_0 = argv[1],
                   *filename_1 = argv[2],
                   *filename_result = argv[3];
        Matrix matrix_0, matrix_1;

        try
        {
            MatrixUtils::load(matrix_0, filename_0);
            MatrixUtils::load(matrix_1, filename_1);
        }
        catch (std::exception e)
        {
            std::cout << "cannot open matrix file" << std::endl;
        }

        if (matrix_0.get_width() != matrix_1.get_height())
        {
            std::cout << "matrixes are incompatible" << std::endl;
        }

        uint64_t matrix_0_width = matrix_0.get_width(),
                 matrix_0_height = matrix_0.get_height(),
                 matrix_0_length = matrix_0_height * matrix_0_width,
                 matrix_1_width = matrix_1.get_width(),
                 matrix_1_height = matrix_1.get_height(),
                 matrix_1_length = matrix_1_height * matrix_1_width;

        uint64_t result_matrix_height = matrix_0.get_height(),
                 result_matrix_width = matrix_1.get_width(),
                 result_matrix_depth = matrix_0.get_width();
        uint64_t
            buffer_size = result_matrix_width * result_matrix_height;
        uint8_t *data_0 = (uint8_t *)matrix_0.get_data(),
                *data_1 = (uint8_t *)matrix_1.get_data(),
                *data_buffer = new uint8_t[buffer_size]{0};

        std::cout << "meta0 : \n"
                  << "height: " << matrix_0.get_height() << "\nwidth: " << matrix_0.get_width() << std::endl;
        std::cout << "meta1 : \n"
                  << "height: " << matrix_1.get_height() << "\nwidth: " << matrix_1.get_width() << std::endl;
        if (result_matrix_height >= UINT64_MAX / result_matrix_width ||
            matrix_0_height >= UINT64_MAX / matrix_0_width ||
            matrix_1_height >= UINT64_MAX / matrix_1_width)
        {
            std::cout << "matrixes are too big to be multiplied" << std::endl;
        }

        if (matrix_0_width != matrix_1_height)
        {
            std::cout << "matrixes are incompatible" << std::endl;
            return -5;
        }
        uint8_t *buffer;
        int retVal = 0;

        for (uint64_t i = 0; i < result_matrix_height; i++)
        {
            for (uint64_t j = 0; j < result_matrix_width; j++)
            {
                for (uint64_t n = 0; n < result_matrix_depth; n++)
                {
                    data_buffer[i * result_matrix_width + j] += data_0[i * result_matrix_depth + n] * data_1[n * result_matrix_width + j];
                }
            }
        }
        Matrix result(result_matrix_height, result_matrix_width);
        result.set_data(data_buffer);
        MatrixUtils::save(result, filename_result);
        delete[] data_buffer, data_0, data_1;
        return retVal;
    }
    catch (std::exception ex)
    {
        std::cout << "expt: " << ex.what() << std::endl;
    }
}

uint8_t validate_args(int argc, char **argv)
{
    int ret_val = 0;
    if (argc != 4 || argv[1] == nullptr || argv[2] == nullptr || argv[3] == nullptr)
    {
        ret_val = 1;
    }
    return ret_val;
}