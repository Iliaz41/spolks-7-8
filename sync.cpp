#include <iostream>
#include <mpi.h>
#include "include/matrix.hpp"
#include "include/utils.hpp"
#include <algorithm>
#include <cstring>
#include <math.h>
#include <unistd.h>

int COLOR;
int COLOR_DEV;
int DEV;

inline int send_and_fin(MPI::Intracomm, uint8_t *, uint64_t);
inline void master_start_copy_proc(MPI::Intracomm, uint8_t *, uint64_t, uint64_t, MPI::Request *);
int master_wait_and_fin(MPI::Intracomm, uint8_t *, uint64_t , uint64_t , Matrix &, const char *);
inline void group_handler(MPI::Intracomm, const char *, const char *, const char *);

int main(int argc, char **argv)
{
    MPI::Init(argc, argv);
    int group_size = MPI::COMM_WORLD.Get_size(),
        group_rank = MPI::COMM_WORLD.Get_rank();
    DEV = group_rank;
    COLOR_DEV = DEV;

    try
    {
        if (argc != 4 || argv[1] == nullptr || argv[2] == nullptr || argv[3] == nullptr)
        {
            throw std::invalid_argument("invalid argument were specified.");
        }
        group_handler(MPI::COMM_WORLD, argv[1], argv[2], argv[3]);
    }
    catch (BusinessException ex)
    {
        std::cout << " expt:"
                  << "dev" << DEV << " (" << COLOR << ":" << COLOR_DEV << ")"
                  << ": " << ex.what() << std::endl;
    }
    catch (std::invalid_argument ex)
    {
        std::cout << " expt: "
                  << "dev" << DEV << " (" << COLOR << ":" << COLOR_DEV << ")"
                  << ": " << ex.what() << std::endl;
    }

    MPI::Finalize();
    return 0;
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

int send_and_fin(MPI::Intracomm comm, uint8_t *data, uint64_t length)
{
    char str[MPI::MAX_ERROR_STRING];
    int strLenght = 0;
    MPI::Status status;
    // MPI_Recv(data, length, MPI_BYTE, 0, 0, comm, &status.mpi_status);
    comm.Send((void*)data, length, MPI::BYTE, 0, 0);
    // MPI::Get_error_string(status.Get_error(), str, strLenght);
    // std::cout << "dev" << DEV << " (" << COLOR << ":" << COLOR_DEV << ")"
    //           << ": send status " << status.Get_error() << "\t\t : " << str << std::endl;
    return status.Get_error();
}

int master_wait_and_fin(MPI::Intracomm comm, uint8_t *dest, uint64_t group_size, uint64_t recv_size, Matrix &matrix, const char *filename)
{

    char str[MPI::MAX_ERROR_STRING];
    int strLenght = 0;
    MPI::Status status;
    std::cout << "dev" << DEV << " (" << COLOR << ":" << COLOR_DEV << ")"
              << "waiting for other dev..." << std::endl;
    for (uint64_t counter = 0; counter < group_size - 1; counter++)
    {
        comm.Recv(dest + recv_size * (counter+1), recv_size, MPI::BYTE, counter+1, MPI::ANY_TAG, status);
        MPI::Get_error_string(status.Get_error(), str, strLenght);
        std::cout << "dev" << DEV << " (" << COLOR << ":" << COLOR_DEV << ")" << status.Get_source() << ": status " << status.Get_error() << "\t\t : " << str << std::endl;
    }
    MatrixUtils::save(matrix, filename);
    return 0;
}

void group_handler(MPI::Intracomm comm, const char *filename_0, const char *filename_1, const char *filename_result)
{
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
            MatrixUtils::multipliation(buffer, data_0, data_1, result_matrix_width, result_matrix_depth, matrix_0.get_height());
            retVal = master_wait_and_fin(MPI::COMM_WORLD, buffer, group_size, buffer_size, matrix_result, filename_result);
            end_time = MPI::Wtime();
            std::cout << "dev" << DEV << " (" << COLOR << ":" << COLOR_DEV << ")"
                      << "ellapsed time " << end_time - start_time << std::endl;
        }
        else
        {
            buffer = new uint8_t[buffer_size]{0};
            MatrixUtils::multipliation(buffer, data_0, data_1, result_matrix_width, result_matrix_depth, matrix_0.get_height());
            std::cout << "dev" << DEV << ": mult complete" << std::endl;
            retVal = send_and_fin(comm, buffer, buffer_size);
        }
        delete[] buffer, matrix_0.get_data(), matrix_1.get_data();
    }
    catch (BusinessException ex)
    {
        std::cout << " expt: "
                  << "dev" << DEV << " (" << COLOR << ":" << COLOR_DEV << ")"
                  << ": " << ex.what() << std::endl;
    }
    catch (std::exception ex)
    {
        std::cout << " expt: "
                  << "dev" << DEV << " (" << COLOR << ":" << COLOR_DEV << ")"
                  << ": " << ex.what() << std::endl;
    }
    catch(...){
        std::cout << "dev" << DEV << ": unknown fault occured." << std::endl;
    }
}