#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <string>


using namespace std;

void read_data(int** A, int size, string filename)
{
    ifstream data;
    data.open(filename);

    if (data.is_open()) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                data >> A[i][j];
            }
        }
    }
    data.close();
}

void write_result_matrix(string filename, int** C, int size)
{
    ofstream data(filename);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            data << C[i][j] << "\n";
        }
    }

    data.close();
}

int** resize_matrix(int size)
{
    int** matrix = new int* [size];
    for (int i = 0; i < size; ++i)
        matrix[i] = new int[size];
    return matrix;
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");
    srand(time(NULL));

    int sizes[9]{ 10, 50, 100, 500, 600, 700, 800, 900, 1000 };

    int** A, ** B, ** C;
    string res;
    vector<vector<string>> rez_txt;
    string res_filename, filename1, filename2;

    int threads = 1;
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    for (int t = 0; t < 5; t++)
    {
        cout << "Кол-во потоков: " << size << endl;

        for (int current_size : sizes)
        {
            if (rank == 0)
                cout << "Размер матриц: " << current_size << endl;

            res.append("Количество потоков: ").append(to_string(size)).append("\n");
            res.append("Размер матрицы: ").append(to_string(current_size)).append("x").append(to_string(current_size)).append("\n");
            res.append("Время перемножения: ");

            if (rank == 0) {
                A = resize_matrix(current_size);
                B = resize_matrix(current_size);
                C = resize_matrix(current_size);
                filename1 = filename[0];
                filename2 = filename[1];
                read_data(A, current_size, filename1.append(to_string(current_size)).append(".txt"));
                read_data(B, current_size, filename2.append(to_string(current_size)).append(".txt"));
            }

            MPI_Barrier(MPI_COMM_WORLD);
            double wtime = MPI_Wtime();

            for (int m = 0; m < 10; m++)
            {
                MPI_Bcast(&(B[0][0]), current_size * current_size, MPI_INT, 0, MPI_COMM_WORLD);

                int rows_per_process = current_size / size;
                int rows_remaining = current_size % size;
                int start_row = rank * rows_per_process + min(rank, rows_remaining);
                int end_row = start_row + rows_per_process + (rank < rows_remaining ? 1 : 0);

                for (int i = start_row; i < end_row; i++)
                {
                    for (int j = 0; j < current_size; j++)
                    {
                        C[i][j] = 0;
                        for (int k = 0; k < current_size; k++)
                            C[i][j] += B[i][k] * A[k][j];
                    }
                }

                MPI_Barrier(MPI_COMM_WORLD);
            }

            MPI_Barrier(MPI_COMM_WORLD);
            wtime = MPI_Wtime() - wtime;

            if (rank == 0) {
                res_filename = filename[2];
                write_result_matrix(res_filename.append(to_string(current_size)).append("_").append(to_string(t)).append(".txt"), C, current_size);
                cout << "Время: " << wtime << " сек." << endl;
                res.append(to_string(wtime)).append("; ");
            }

            res.append("\n--------------------------------------\n");
        }
        threads *= 2;
    }

    MPI_Finalize();
    ofstream data("rez_mul.txt");
    data << res;
    data.close();

    return 0;
}