# **Parallel Game of Life**

This project is a parallel implementation of Conway's Game of Life using **MPI** (Message Passing Interface). It efficiently simulates the evolution of cellular automata on large grids using multiple processes distributed across a cluster. The implementation leverages non-blocking communication (`MPI_Isend`/`MPI_Irecv`) and 1D row decomposition for load balancing and scalability.

## **Project Structure**

```plaintext
.
├── 416 p2 writeup.pdf       # Detailed report with performance analysis
├── life-nonblocking.cpp     # Parallel implementation using MPI
├── life.1.256x256.data      # Sample input file (256x256 grid, scenario 1)
├── life.2.256x256.data      # Sample input file (256x256 grid, scenario 2)
├── life.512x512.data        # Sample input file (512x512 grid)
├── life.1024x1024.data      # Sample input file (1024x1024 grid)
├── Makefile                 # Build configuration
├── submit.sh                # SLURM script for running the program
```

## **Features**

- **Parallelization Strategy**:

  - The grid is decomposed into rows, and each MPI process handles a subset of rows.
  - Ghost rows are used for boundary communication between neighboring processes.

- **Scalable Communication**:

  - Non-blocking communication with `MPI_Isend`/`MPI_Irecv` ensures efficient data exchange.

- **Performance**:

  - Demonstrates significant speedup compared to the serial implementation.
  - Achieves a **130x speedup with 128 processes** on a 512x512 grid.

- **Flexibility**:
  - Supports grids of various sizes (e.g., 256x256, 512x512, 1024x1024).
  - The number of processes can be specified dynamically.

## **How to Build**

To compile the project, ensure you have MPI installed (e.g., OpenMPI), then run:

```bash
make
```

This will generate two executables:

- `life-nonblocking`: The parallel implementation.
- `serial`: The serial implementation for comparison.

## **How to Run**

### **Using SLURM**

The `submit.sh` script automates the execution of the program on a cluster:

```bash
sbatch submit.sh <num_processes> <dataset> <generations> <rows> <cols>
```

For example:

```bash
sbatch submit.sh 16 life.1.256x256.data 100 256 256
```

### **Manual Execution**

You can also run the program manually:

#### **Parallel (MPI Implementation)**

```bash
mpirun -np <num_processes> ./life-nonblocking <dataset> <generations> <rows> <cols>
```

#### **Serial Implementation**

```bash
./serial <dataset> <generations> <rows> <cols>
```

## **Input Format**

The input file specifies the initial live cells on the grid. Each line represents the coordinates of a live cell in the format:

```plaintext
x,y
```

For example:

```plaintext
1,3
4,5
10,15
```

## **Output**

The output is a CSV file containing the coordinates of live cells at the final generation. The file is named as follows:

```plaintext
<dataset_name>.<num_generations>_parallel.csv
```

For example:

```plaintext
life.1.256x256.100_parallel.csv
```

## **Performance Analysis**

The performance analysis (detailed in `analysis.pdf`) shows:

1. **Speedup**:

   - A **95% reduction in runtime** with 4 processes compared to serial execution.
   - **130x speedup with 128 processes** on a 512x512 grid.

2. **Scalability**:
   - Demonstrates diminishing returns with higher process counts due to communication overhead.

## **Acknowledgements**

This project was developed as part of CMSC 416 coursework, focusing on parallel and distributed computing. Special thanks to the instructors for providing the problem and dataset templates.
