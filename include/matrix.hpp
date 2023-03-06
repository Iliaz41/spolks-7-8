#pragma once
#include <stdint.h>
#include <stddef.h>
#include <iostream>
#include "matrix_metadata.hpp"

class Matrix
{
private:
    Metadata metadata;
    uint8_t *data = nullptr;

public:
    Matrix();
    Matrix(uint64_t, uint64_t);
    Matrix(Matrix&);
    const uint8_t *get_data() const;
    uint8_t *get_data();
    void set_data(uint8_t *);
    const uint8_t get(const uint64_t, const uint64_t);

    const uint64_t get_height() const;
    const uint64_t get_width() const;
    friend std::ostream &operator<<(std::ostream &out, const Matrix &c);
    friend std::istream &operator>>(std::istream &in, Matrix &c);

    ~Matrix();
};

Matrix::Matrix(){
    this->metadata = {0, 0, 1};
    this->data = nullptr;
}

Matrix::Matrix(Matrix& matrix){
    this->metadata = matrix.metadata;
    this->data = matrix.data;
}

Matrix::Matrix(uint64_t height, uint64_t width)
{
    this->metadata = {height, width, 1};
    this->data = nullptr;
}

Matrix::~Matrix()
{
}

const uint64_t Matrix::get_height() const
{
    return this->metadata.height;
}

const uint64_t Matrix::get_width() const
{
    return this->metadata.width;
}

const uint8_t *Matrix::get_data() const
{
    return this->data;
}

uint8_t *Matrix::get_data()
{
    return this->data;
}

void Matrix::set_data(uint8_t *data)
{
    this->data = data;
}

std::ostream &operator<<(std::ostream &out, const Matrix &c)
{

    uint64_t height = c.get_height(),
             width = c.get_width();
    uint8_t *data = (uint8_t *)c.get_data();
    out.write((char*)(&height), sizeof(uint64_t));
    out.write((char*)(&width), sizeof(uint64_t));
    out.write((char*)data, height*width);
    return out;
}

std::istream &operator>>(std::istream &in, Matrix &c)
{
    try
    {
        if (c.data != nullptr)
        {
            std::cout << "matrix data delete before load from stream" << std::endl;
            delete[] c.data;
        }
    }
    catch (std::exception exception)
    {
        std::cout << exception.what();
    }

    in.read((char*)(&c.metadata.height), sizeof(uint64_t));
    in.read((char*)(&c.metadata.width), sizeof(uint64_t));
    uint8_t *data = new uint8_t[c.metadata.width * c.metadata.height];
    c.data = data;
    in.read((char*)data, c.metadata.height * c.metadata.width);
    
    return in;
}

const uint8_t Matrix::get(const uint64_t x, const uint64_t y)
{
    return data[y * metadata.width + x];
}
