#include <iostream>
#include <mpi.h>
#include "include/matrix.hpp"
#include "include/utils.hpp"
#include "include/exception.hpp"
#include "include/matrix_metadata.hpp"
#include <algorithm>
#include <cstring>
#include <math.h>
#include <random>

inline int send_and_fin(MPI::Intracomm, uint8_t *, uint64_t);
inline void master_start_copy_proc(MPI::Intracomm, uint8_t *, uint64_t, uint64_t, MPI::Request *);
inline int master_wait_and_fin(uint64_t, MPI::Request *, Matrix &, const char *);
inline void group_handler(MPI::Intracomm, const char*, const char*, const char*);


int COLOR;
int COLOR_DEV;
int DEV;
int main(int argc, char **argv)
{
    MPI::Init(argc, argv);
    int group_size = MPI::COMM_WORLD.Get_size(),
        group_rank = MPI::COMM_WORLD.Get_rank();
    DEV = group_rank;

    try
    {
        if (argc != 5 || argv[1] == nullptr || argv[2] == nullptr || argv[3] == nullptr || argv[4] == nullptr)
        {
            throw std::invalid_argument("invalid argument were specified.");
        }
        int group_num = std::stoi(argv[4]);

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(0, group_num-1); // distribution in range [1, 6]

        if(group_num > group_size){
            group_num = group_size;
        }

        if(group_rank == 0){
            std::cout << "group count: " << group_num << std::endl;
        }
        COLOR = dist6(rng);
        MPI::Intracomm comm = MPI::COMM_WORLD.Split(COLOR, group_rank);
        group_size = comm.Get_size();
        group_rank = comm.Get_rank();
        COLOR_DEV = group_rank;
        std::cout << "dev" << DEV << " (" << COLOR<< ":"<< COLOR_DEV << ")" <<  ": color " << COLOR << std::endl;
        std::string filename_result(argv[3]);
        filename_result += COLOR + '0';
        group_handler(comm, argv[1], argv[2], filename_result.c_str());
    }
    catch (BusinessException ex)
    {
        std::cout << " expt:"  << "dev" << DEV << " (" << COLOR<< ":"<< COLOR_DEV << ")" << ": " << ex.what() << std::endl;
    }
    catch (std::invalid_argument ex)
    {
        std::cout << " expt: "<< "dev" << DEV << " (" << COLOR<< ":"<< COLOR_DEV << ")" << ": " << ex.what() << std::endl;
    }

    MPI::Finalize();
    return 0;
}

int send_and_fin(MPI::Intracomm comm, uint8_t *data, uint64_t length)
{
    MPI::Request request;
    MPI::Status status;
    request = comm.Isend(data, length, MPI::BYTE, 0, 0);
    request.Wait(status);
    return status.Get_error();
}

void master_start_copy_proc(MPI::Intracomm comm, uint8_t *dest, uint64_t group_size, uint64_t recv_size, MPI::Request *requests)
{
    MPI::Status *status = new MPI::Status[group_size - 1];

    for (uint64_t counter = 1; counter < group_size; counter++)
    {
        requests[counter - 1] = comm.Irecv(dest + recv_size * counter, recv_size, MPI::BYTE, counter, MPI::ANY_TAG);
        // std::cout << "dev" << DEV << " (" << COLOR<< ":"<< COLOR_DEV << ")" << ": cpoy proc started" << std::endl;
    }
}

int master_wait_and_fin(uint64_t group_size, MPI::Request *requests, Matrix &matrix, const char *filename)
{

    char str[MPI::MAX_ERROR_STRING];
    int strLenght = 0;
    MPI::Status statuses[group_size - 1];
    std::cout << "dev" << DEV << " (" << COLOR<< ":"<< COLOR_DEV << ")"<< "waiting for other dev..." << std::endl;
    MPI::Request::Waitall(group_size - 1, requests, statuses);
    for (uint64_t counter = 0; counter < group_size - 1; counter++)
    {
        MPI::Get_error_string(statuses[counter].Get_error(), str, strLenght);
        std::cout << "dev" << DEV << " (" << COLOR<< ":"<< COLOR_DEV << ")" << statuses[counter].Get_source() << ": status " << statuses[counter].Get_error() << "\t\t : " << str << std::endl;
    }
    MatrixUtils::save(matrix, filename);
    return 0;
}

void group_handler(MPI::Intracomm comm, const char* filename_0, const char* filename_1, const char* filename_result){
    int group_size = comm.Get_size(),
        group_rank = comm.Get_rank();
    try
    {
        Metadata meta_0 = MatrixUtils::readMetadata(filename_0),
                 meta_1 = MatrixUtils::readMetadata(filename_1);

        if (meta_0.width != meta_1.height)
        {
            throw BusinessException("Matrixes are incompatible.");
        }

        uint64_t matrix_0_width = meta_0.width,
                 matrix_0_height = meta_0.height,
                 matrix_0_length = matrix_0_height * matrix_0_width,
                 matrix_1_width = meta_1.width,
                 matrix_1_height = meta_1.height,
                 matrix_1_length = matrix_1_height * matrix_1_width;

        uint64_t result_matrix_height = matrix_0_height,
                 result_matrix_width = matrix_1_width,
                 result_matrix_depth = meta_0.width;
        uint64_t full_lines_portion = (result_matrix_height + group_size - 1) / group_size,
                 y_begin = full_lines_portion * group_rank,
                 buffer_size = result_matrix_width * full_lines_portion;

        const Matrix &matrix_1 = MatrixUtils::load(filename_1),
                     &matrix_0 = MatrixUtils::readPortion(filename_0, y_begin, full_lines_portion);

        uint8_t *data_0 = (uint8_t *)matrix_0.get_data(),
                *data_1 = (uint8_t *)matrix_1.get_data();

        if (result_matrix_height >= UINT64_MAX / result_matrix_width ||
            matrix_0_height >= UINT64_MAX / matrix_0_width ||
            matrix_1_height >= UINT64_MAX / matrix_1_width)
        {
            throw BusinessException("Matrixes are too big to be multiplied.");
        }

        uint8_t *buffer;
        int retVal = 0;
        if (group_rank == 0)
        {
            double start_time, end_time;
            MPI::Request requests[group_size - 1];
            buffer = new uint8_t[group_size * full_lines_portion * result_matrix_width]{0};
            Matrix matrix_result(result_matrix_height, result_matrix_width);
            matrix_result.set_data(buffer);
            MatrixUtils::print(matrix_result, "result matrix");
            start_time = MPI::Wtime();
            master_start_copy_proc(comm, buffer, group_size, buffer_size, requests);
            MatrixUtils::multipliation(buffer, data_0, data_1, result_matrix_width, result_matrix_depth, matrix_0.get_height());
            retVal = master_wait_and_fin(group_size, requests, matrix_result, filename_result);
            end_time = MPI::Wtime();
            std::cout << "dev" << DEV << " (" << COLOR<< ":"<< COLOR_DEV << ")" <<  "ellapsed time " << end_time - start_time << std::endl;
        }
        else
        {
            buffer = new uint8_t[buffer_size]{0};
            MatrixUtils::multipliation(buffer, data_0, data_1, result_matrix_width, result_matrix_depth, matrix_0.get_height());
            retVal = send_and_fin(comm, buffer, buffer_size);
        }
        delete[] buffer, matrix_0.get_data(), matrix_1.get_data();
    }
    catch (BusinessException ex)
    {
        std::cout << " expt: "<< "dev" << DEV << " (" << COLOR<< ":"<< COLOR_DEV << ")" << ": " << ex.what() << std::endl;
    }
    catch (std::exception ex)
    {
        std::cout << " expt: "<< "dev" << DEV << " (" << COLOR<< ":"<< COLOR_DEV << ")" << ": " << ex.what() << std::endl;
    }
}