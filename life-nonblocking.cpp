#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "mpi.h"
#include <vector>
using namespace std;

/*
 * Reads the input file line by line and stores it in a 2D matrix.
 */
void read_input_file(int **life, string const &input_file_name) {
    
    // Open the input file for reading.
    ifstream input_file;
    input_file.open(input_file_name);
    if (!input_file.is_open())
        perror("Input file cannot be opened");

    string line, val;
    int x, y;
    while (getline(input_file, line)) {
        stringstream ss(line);
        
        // Read x coordinate.
        getline(ss, val, ',');
        x = stoi(val);
        
        // Read y coordinate.
        getline(ss, val);
        y = stoi(val);

        // Populate the life matrix.
        life[x][y] = 1;
    }
    input_file.close();
}

/* 
 * Writes out the final state of the 2D matrix to a csv file. 
 */
void write_output(int **result_matrix, int X_limit, int Y_limit,
                  string const &input_name, int num_of_generations) {
    
    // Open the output file for writing.
    ofstream output_file;
    string input_file_name = input_name.substr(0, input_name.length() - 5);
    output_file.open(input_file_name + "." + to_string(num_of_generations) +
                    "_parallel.csv");
    if (!output_file.is_open())
        perror("Output file cannot be opened");
    
    // Output each live cell on a new line. 
    for (int i = 0; i < X_limit; i++) {
        for (int j = 0; j < Y_limit; j++) {
            if (result_matrix[i][j] == 1) {
                output_file << i << "," << j << "\n";
            }
        }
    }
    output_file.close();
}

/*
 * Processes the life array for one generation.
 */
void compute(int **life, int **previous_life, int local_X_limit, int Y_limit) {
    int neighbors = 0;

    // Update the previous_life matrix with the current life matrix state.
    for (int i = 0; i < local_X_limit + 2; i++) {
        for (int j = 0; j < Y_limit; j++) {
            previous_life[i][j + 1] = life[i][j];
        }
        // Pad the first and last columns with zeros
        previous_life[i][0] = 0;
        previous_life[i][Y_limit + 1] = 0;
    }

    // For simulating each generation, calculate the number of live
    // neighbors for each cell and then determine the state of the cell in
    // the next iteration.
    for (int i = 1; i <= local_X_limit; i++) { // i from 1 to local_X_limit inclusive
        for (int j = 1; j <= Y_limit; j++) {   // j from 1 to Y_limit inclusive
            neighbors = previous_life[i - 1][j - 1] + previous_life[i - 1][j] +
                        previous_life[i - 1][j + 1] + previous_life[i][j - 1] +
                        previous_life[i][j + 1] + previous_life[i + 1][j - 1] +
                        previous_life[i + 1][j] + previous_life[i + 1][j + 1];

            if (previous_life[i][j] == 0) {
                // A cell is born only when an unoccupied cell has 3 neighbors.
                if (neighbors == 3)
                    life[i][j - 1] = 1;
                else
                    life[i][j - 1] = 0;
            } else {
                // An occupied cell survives only if it has either 2 or 3 neighbors.
                if (neighbors == 2 || neighbors == 3)
                    life[i][j - 1] = 1;
                else
                    life[i][j - 1] = 0;
            }
        }
    }
}

/**
  * The main function to execute "Game of Life" simulations on a 2D board.
  */
int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    // get the rank of this process, and total number of processes
    int rank, num_processes;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

    // if we have a bad number of args
    if (argc != 5) {
        // if this is the 0th process, error with a message
        if (rank == 0) {
            perror("Expected arguments: ./life <input_file> <num_of_generations> <X_limit> <Y_limit>");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    string input_file_name = argv[1];
    int num_of_generations = stoi(argv[2]);
    int X_limit = stoi(argv[3]);
    int Y_limit = stoi(argv[4]);

    // each process will have a portion of the grid
    int local_X_limit = X_limit / num_processes;

    // Each process allocates life and previous_life arrays
    int **life = new int *[local_X_limit + 2];
    int **previous_life = new int *[local_X_limit + 2];
    for (int i = 0; i < local_X_limit + 2; i++) {
        life[i] = new int[Y_limit];
        previous_life[i] = new int[Y_limit + 2];
        for (int j = 0; j < Y_limit; j++) {
            life[i][j] = 0;
        }
        for (int j = 0; j < Y_limit + 2; j++) {
            previous_life[i][j] = 0;
        }
    }

    // Read input and distribute data
    if (rank == 0) {
        // Read the entire grid
        int **full_life = new int *[X_limit];
        for (int i = 0; i < X_limit; i++) {
            full_life[i] = new int[Y_limit];
            for (int j = 0; j < Y_limit; j++) {
                full_life[i][j] = 0;
            }
        }
        read_input_file(full_life, input_file_name);

        // Non-blocking send data to other processes
        MPI_Request *send_requests = new MPI_Request[(num_processes - 1) * local_X_limit];
        int req_count = 0;
        for (int p = 1; p < num_processes; p++) {
            int start_row = p * local_X_limit;
            for (int i = 0; i < local_X_limit; i++) {
                MPI_Isend(full_life[start_row + i], Y_limit, MPI_INT, p, 0, MPI_COMM_WORLD, &send_requests[req_count++]);
            }
        }

        // Copy own data
        for (int i = 0; i < local_X_limit; i++) {
            for (int j = 0; j < Y_limit; j++) {
                life[i + 1][j] = full_life[i][j]; // +1 offset for halo
            }
        }

        // Wait for all sends to complete
        MPI_Waitall(req_count, send_requests, MPI_STATUSES_IGNORE);

        // Clean up send requests and full_life
        delete[] send_requests;
        for (int i = 0; i < X_limit; i++) {
            delete[] full_life[i];
        }
        delete[] full_life;
    } else {
        // Non-blocking receive data from rank 0
        MPI_Request *recv_requests = new MPI_Request[local_X_limit];
        for (int i = 0; i < local_X_limit; i++) {
            MPI_Irecv(life[i + 1], Y_limit, MPI_INT, 0, 0, MPI_COMM_WORLD, &recv_requests[i]); // +1 offset for halo
        }

        // Wait for all receives to complete
        MPI_Waitall(local_X_limit, recv_requests, MPI_STATUSES_IGNORE);

        // Clean up receive requests
        delete[] recv_requests;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    clock_t start = clock();

    // Main simulation loop
    for (int numg = 0; numg < num_of_generations; numg++) {
        // Exchange halo rows with neighboring processes using non-blocking communication
        MPI_Request requests[4];
        int req_count = 0;

        // Send top row to rank-1, receive from rank-1
        if (rank > 0) {
            MPI_Isend(life[1], Y_limit, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
            MPI_Irecv(life[0], Y_limit, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
        } else {
            // For rank 0, set top halo row to zeros (edge of grid)
            for (int j = 0; j < Y_limit; j++) {
                life[0][j] = 0;
            }
        }

        // Send bottom row to rank+1, receive from rank+1
        if (rank < num_processes - 1) {
            MPI_Isend(life[local_X_limit], Y_limit, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
            MPI_Irecv(life[local_X_limit + 1], Y_limit, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
        } else {
            // For last rank, set bottom halo row to zeros (edge of grid)
            for (int j = 0; j < Y_limit; j++) {
                life[local_X_limit + 1][j] = 0;
            }
        }

        // Wait for all non-blocking communications to complete
        MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);

        // Perform computation on local grid
        compute(life, previous_life, local_X_limit, Y_limit);
    }

    MPI_Barrier(MPI_COMM_WORLD);  // Synchronize after computation

    clock_t end = clock();
    float local_time = float(end - start) / CLOCKS_PER_SEC;

    // Collect timing information
    float min_time, max_time, avg_time;
    MPI_Reduce(&local_time, &min_time, 1, MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time, &max_time, 1, MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_time, &avg_time, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    avg_time /= num_processes;

    if (rank == 0) {
        cout << "TIME: Min: " <<  min_time << " s Avg: " << avg_time << " s Max: " << max_time << " s\n";
    }

    // Gather final data to rank 0
    if (rank == 0) {
        // Allocate full result matrix
        int **result_matrix = new int *[X_limit];
        for (int i = 0; i < X_limit; i++) {
            result_matrix[i] = new int[Y_limit];
        }

        // Copy own data
        for (int i = 0; i < local_X_limit; i++) {
            for (int j = 0; j < Y_limit; j++) {
                result_matrix[i][j] = life[i + 1][j]; // +1 offset for halo
            }
        }

        // Non-blocking receive data from other processes
        MPI_Request *recv_requests = new MPI_Request[(num_processes - 1) * local_X_limit];
        int req_count = 0;
        for (int p = 1; p < num_processes; p++) {
            int start_row = p * local_X_limit;
            for (int i = 0; i < local_X_limit; i++) {
                MPI_Irecv(result_matrix[start_row + i], Y_limit, MPI_INT, p, 0, MPI_COMM_WORLD, &recv_requests[req_count++]);
            }
        }

        // Wait for all receives to complete
        MPI_Waitall(req_count, recv_requests, MPI_STATUSES_IGNORE);

        // Write output
        write_output(result_matrix, X_limit, Y_limit, input_file_name, num_of_generations);

        // Clean up result_matrix and receive requests
        for (int i = 0; i < X_limit; i++) {
            delete[] result_matrix[i];
        }
        delete[] result_matrix;
        delete[] recv_requests;
    } else {
        // Non-blocking send own data to rank 0
        MPI_Request *send_requests = new MPI_Request[local_X_limit];
        for (int i = 0; i < local_X_limit; i++) {
            MPI_Isend(life[i + 1], Y_limit, MPI_INT, 0, 0, MPI_COMM_WORLD, &send_requests[i]); // +1 offset for halo
        }

        // Wait for all sends to complete
        MPI_Waitall(local_X_limit, send_requests, MPI_STATUSES_IGNORE);

        // Clean up send requests
        delete[] send_requests;
    }

    // Clean up local arrays
    for (int i = 0; i < local_X_limit + 2; i++) {
        delete[] life[i];
        delete[] previous_life[i];
    }
    delete[] life;
    delete[] previous_life;

    MPI_Finalize();  // Finalize MPI
    return 0;
}
