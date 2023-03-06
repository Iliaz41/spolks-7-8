#include <iostream>
#include <fstream>
#include "include/utils.hpp"
#include "include/matrix.hpp"

using namespace std;

int main(int argc, char **argv)
{
    const char *filename1, *filename2,
            *message_equal = "equal",
            *message_differ = "differ";
    try
    {
        filename1 = argv[1];
        filename2 = argv[2];
    }
    catch (std::exception e)
    {
        std::cout << "incompateble arguments" << std::endl;
        return -1;
    }

    std::ifstream is1(filename1);
    std::ifstream is2(filename2);

    if (!is1.is_open() || !is2.is_open())
    {
        std::cout << "cannot open file" << std::endl;
        return -2;
    }

    Matrix matrix1, matrix2;
    is1 >> matrix1;
    is2 >> matrix2;
    is1.close();
    is2.close();
    bool are_equal = MatrixUtils::compare(matrix1, matrix2);

    if(are_equal){
        std::cout << message_equal<< std::endl;
    }else{
        std::cout << message_equal<< std::endl;
        std::cout << "number elements differ:"<<MatrixUtils::diff(matrix1, matrix2)<< std::endl;
    }
    
    return 0;
}