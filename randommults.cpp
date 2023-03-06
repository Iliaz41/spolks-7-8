#include <iostream>
#include <fstream>
#include "include/utils.hpp"
#include "include/matrix.hpp"

using namespace std;

int main(int argc, char **argv)
{
    uint64_t height, width;
    size_t element_size;
    try
    {
        height = stol(argv[2]);
        width = stol(argv[3]);
    }
    catch (std::exception e)
    {
        std::cout << "incompateble arguments" << std::endl;
        return -1;
    }

    std::string filename = "matrix";
    if (argc > 2)
    {
        filename = argv[1];
    }
    filename += ".mtrx";

    Matrix matrix = Matrix(height, width);
    uint8_t* data = MatrixUtils::generate(height, width);
    matrix.set_data(data);
    MatrixUtils::save(matrix, filename);
    std::cout << "complete" << std::endl;
    return 0;
}