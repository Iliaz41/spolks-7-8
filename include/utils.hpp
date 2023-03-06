#pragma once
#include <stdint.h>
#include <fstream>
#include "matrix.hpp"
#include "exception.hpp"
#define MATRIX_FILE_HEADER_SIZE 12
class MatrixUtils
{
private:
    /* data */
    MatrixUtils(/* args */) = default;
    ~MatrixUtils() = default;

public:
    static uint8_t *generate(uint64_t, uint64_t);
    static Matrix generate_M(uint64_t, uint64_t);
    static bool compare(const Matrix &, const Matrix &);
    static uint64_t diff(const Matrix &, const Matrix &);
    static void load(Matrix &, const char *);
    static Matrix load(const char *);
    static void save(const Matrix &, const char *);
    static void save(const Matrix &, const std::string &);
    static void print(const Matrix &);
    static void print(const Metadata &, const char *);
    static void print(const Matrix &, const char *);
    static void multipliation(uint8_t *, uint8_t *, uint8_t *, uint64_t, uint64_t, uint64_t, uint64_t);
    static void multipliation(uint8_t *, uint8_t *, uint8_t *, uint64_t, uint64_t, uint64_t);
    static Matrix *split(Matrix &, uint64_t);
    static Matrix readPortion(const char *, uint64_t, uint64_t);
    static const Metadata readMetadata(const char *);
};

Matrix MatrixUtils::generate_M(uint64_t height, uint64_t width)
{
    Matrix matrix(height, width);
    matrix.set_data(generate(height, width));
    return matrix;
}

uint8_t *MatrixUtils::generate(uint64_t height, uint64_t width)
{
    uint8_t *data = new uint8_t[width * height];
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            data[i * width + j] = rand();
        }
    }
    return data;
}

bool MatrixUtils::compare(const Matrix &matrix1, const Matrix &matrix2)
{
    bool retval = true;
    if (matrix1.get_width() != matrix2.get_width() ||
        matrix1.get_height() != matrix2.get_height())
    {
        retval = false;
    }
    if (retval)
    {
        const uint8_t *data_1 = matrix1.get_data();
        const uint8_t *data_2 = matrix2.get_data();

        for (uint64_t iter = 0; iter < matrix1.get_height() * matrix1.get_width(); iter++)
        {
            if (data_1[iter] != data_2[iter])
            {
                std::cout << "iter " << iter << std::endl;
                retval = false;
                break;
            }
        }
    }
    return retval;
}

uint64_t MatrixUtils::diff(const Matrix &matrix1, const Matrix &matrix2)
{
    uint64_t retval = 0;
    if (matrix1.get_width() != matrix2.get_width() ||
        matrix1.get_height() != matrix2.get_height())
    {
        throw BusinessException("matrix have diffrent headers");
    }
    const uint8_t *data_1 = matrix1.get_data();
    const uint8_t *data_2 = matrix2.get_data();

    uint64_t last_holder = 0;
    uint64_t first_holder = 0;
    bool is_diff = false;
    for (uint64_t iter = 0; iter < matrix1.get_height() * matrix1.get_width(); iter++)
    {
        if (data_1[iter] != data_2[iter])
        {
            if(!is_diff){
                first_holder = iter;
                is_diff = true;
            }
            last_holder = iter;
            ++retval;
        }
    }
    std::cout << "first dif index: " << first_holder << std::endl;
    std::cout << "last dif index: " << last_holder << std::endl;
    return retval;
}

void MatrixUtils::load(Matrix &matrix, const char *filename)
{
    std::ifstream stream(filename);
    stream >> matrix;
    stream.close();
}

Matrix MatrixUtils::load(const char *filename)
{
    Matrix matrix;
    std::ifstream stream(filename);
    stream >> matrix;
    stream.close();
    return matrix;
}

void MatrixUtils::save(const Matrix &matrix, const char *filename)
{
    std::ofstream stream(filename);
    stream << matrix;
    stream.close();
}

void MatrixUtils::save(const Matrix &matrix, const std::string &filename)
{
    std::ofstream stream(filename, std::ios::binary);
    stream << matrix;
    stream.close();
}

void MatrixUtils::print(const Matrix &matrix)
{
    std::cout << "result matrix:" << std::endl
              << "height: " << matrix.get_height() << std::endl
              << "width: " << matrix.get_width() << std::endl;
}

void MatrixUtils::print(const Metadata &metadata, const char *name)
{
    std::cout << "matrix: " << name << std::endl
              << "height: " << metadata.height << std::endl
              << "width: " << metadata.width << std::endl
              << "element size: " << metadata.element_size << std::endl;
}

void MatrixUtils::print(const Matrix &matrix, const char *name)
{
    std::cout << "matrix: " << name << std::endl
              << "height: " << matrix.get_height() << std::endl
              << "width: " << matrix.get_width() << std::endl;
}

void MatrixUtils::multipliation(uint8_t *dest, uint8_t *source1, uint8_t *source2, uint64_t width, uint64_t depth, uint64_t lower_height, uint64_t upper_heigth)
{
    uint8_t *data_buffer_placeholder,
        *data_1_plceholder;

    for (uint64_t y = lower_height; y < upper_heigth; y++)
    {
        data_buffer_placeholder = dest + (y - lower_height) * width;
        uint8_t *data_0_placeholder = source1 + y * depth;
        for (uint64_t n = 0; n < depth; n++)
        {
            uint8_t multiplicant = *(data_0_placeholder + n);
            data_1_plceholder = source2 + n * width;
            for (uint64_t x = 0; x < width; x++)
            {
                data_buffer_placeholder[x] += multiplicant * data_1_plceholder[x];
            }
        }
    }
}

void MatrixUtils::multipliation(uint8_t *dest, uint8_t *source1, uint8_t *source2, uint64_t width, uint64_t depth, uint64_t heigth)
{
    extern int DEV;
    uint8_t *data_buffer_placeholder,
        *data_1_plceholder;

    for (uint64_t y = 0; y < heigth; y++)
    {
        data_buffer_placeholder = dest + y * width;
        uint8_t *data_0_placeholder = source1 + y * depth;
        for (uint64_t n = 0; n < depth; n++)
        {
            uint8_t multiplicant = *(data_0_placeholder + n);
            data_1_plceholder = source2 + n * width;
            for (uint64_t x = 0; x < width; x++)
            {
                // if(DEV != 0) std::cout << "num1 = " << (int)(char)multiplicant << "\nnum2 = " << (int)(char)(data_1_plceholder[x]) << std::endl;
                data_buffer_placeholder[x] += multiplicant * data_1_plceholder[x];
            }
        }
    }
}

Matrix *MatrixUtils::split(Matrix &matrix, uint64_t partitions)
{
    Matrix *matrexes = nullptr;
    if (partitions > 0)
    {
        matrexes = new Matrix[partitions];

        uint64_t height = matrix.get_height() / partitions;
        uint64_t last_height = matrix.get_height() % partitions;
        last_height = last_height != 0 ? last_height : height;
        uint64_t width = matrix.get_width();
        uint8_t *data = matrix.get_data();

        for (uint64_t counter = 0; counter < partitions - 1; counter++)
        {
            matrexes[counter] = Matrix(height, width);
            matrexes[counter].set_data(data + width * counter * height);
        }
        matrexes[partitions - 1] = Matrix(last_height, width);
        matrexes[partitions - 1].set_data(data + width * (partitions - 1) * height);
    }
    return matrexes;
}

Matrix MatrixUtils::readPortion(const char *filename, uint64_t lRow, uint64_t rows)
{
    std::ifstream in(filename);
    uint64_t height, width, part_height;
    in.read((char *)(&height), sizeof(uint64_t));
    in.read((char *)(&width), sizeof(uint64_t));
    if (lRow > height)
    {
        throw BusinessException("incompatible portion params.");
    }
    part_height = (height < lRow + rows) ? (height - lRow) : rows;

    in.seekg(lRow * width + 2 * sizeof(uint64_t), std::ios::beg);
    uint8_t *data = new uint8_t[part_height * width];
    in.read((char *)data, part_height * width);
    in.close();
    Matrix matrix(part_height, width);
    matrix.set_data(data);
    return matrix;
}

const Metadata MatrixUtils::readMetadata(const char *filename)
{
    Metadata metadata;
    std::ifstream in(filename, std::ios::binary);
    metadata.element_size = 1;
    in.read((char *)(&metadata.height), sizeof(uint64_t));
    in.read((char *)(&metadata.width), sizeof(uint64_t));
    in.close();
    return metadata;
}