# **Parallel Game of Life**

This project represents my implementation of a parallelized version of Conway's Game of Life using **MPI** (Message Passing Interface). The program efficiently simulates the evolution of cellular automata on large grids by distributing the workload across multiple processes in a cluster. I utilized non-blocking communication (`MPI_Isend`/`MPI_Irecv`) and a 1D row decomposition strategy to ensure scalability and efficient load balancing.

## **Project Overview**

Working on this project, I explored how to scale computational tasks using parallel programming techniques. Each process is responsible for a subset of rows on the grid, and neighboring processes exchange boundary information to ensure accuracy during simulation. By leveraging MPI and efficient communication techniques, I achieved significant speedup over the serial implementation.

This project was a part of my coursework in parallel computing and gave me hands-on experience in solving a real-world computational problem at scale.

## **Project Structure**

Here's an outline of the files included in this repository:

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

## **Key Features**

This implementation has several notable features:

- **Parallelization Strategy**: The grid is divided into rows, and each MPI process computes the next generation for its subset of rows. Ghost rows are added to enable efficient boundary communication.

- **Efficient Communication**: Non-blocking `MPI_Isend` and `MPI_Irecv` calls allow processes to exchange boundary data asynchronously, reducing idle time and improving performance.

- **Scalability**: The program handles grids of varying sizes (256x256, 512x512, 1024x1024) and adapts dynamically to the number of processes specified during runtime.

- **Performance**: Achieves significant speedup:
  - A **95% reduction in runtime** with 4 processes compared to the serial implementation.
  - A **130x speedup with 128 processes** on a 512x512 grid.

## **How to Build**

To compile the project, ensure that you have MPI installed (e.g., OpenMPI) and run:

```bash
make
```

This will produce two executables:

- `life-nonblocking` (parallel implementation)
- `serial` (for comparison with the parallel version)

## **How to Run**

### **Using SLURM**

To execute the program on a cluster using SLURM, you can use the provided `submit.sh` script:

```bash
sbatch submit.sh <num_processes> <dataset> <generations> <rows> <cols>
```

For example:

```bash
sbatch submit.sh 16 life.1.256x256.data 100 256 256
```

### **Manual Execution**

You can also run the program directly from the command line:

#### **Parallel (MPI Implementation)**:

```bash
mpirun -np <num_processes> ./life-nonblocking <dataset> <generations> <rows> <cols>
```

#### **Serial Implementation**:

```bash
./serial <dataset> <generations> <rows> <cols>
```

## **Input Format**

The input file specifies the coordinates of live cells in the initial grid. Each line in the file is formatted as:

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

The program generates a CSV file listing the coordinates of live cells at the final generation. The output file is named as:

```plaintext
<dataset_name>.<num_generations>_parallel.csv
```

For example:

```plaintext
life.1.256x256.100_parallel.csv
```

## **Performance Analysis**

Through systematic testing and profiling, I observed the following trends:

1. **Speedup**:

   - The program achieves a **95% reduction in runtime** with 4 processes compared to the serial implementation.
   - A **130x speedup with 128 processes** was achieved for a 512x512 grid.

2. **Scalability**:
   - While parallelization shows significant performance gains, diminishing returns appear as the process count increases beyond 64, primarily due to communication overhead. This is consistent with expectations for MPI-based applications.

## **Skills and Knowledge Gained**

This project enhanced my expertise in parallel and distributed computing, particularly with MPI. I gained hands-on experience in domain decomposition, non-blocking communication, and synchronization, which are essential for solving large-scale computational problems. By optimizing workload distribution and communication patterns, I achieved significant performance gains and developed a deeper understanding of scalability and trade-offs in high-performance systems.

Implementing this simulation in C++ reinforced my skills in code optimization and resource management, including efficient memory handling and balancing computation with communication. Profiling and analyzing performance metrics sharpened my ability to identify bottlenecks and improve efficiency in distributed environments.

Overall, this experience strengthened my technical foundation and prepared me for future challenges in computational science and large-scale system design.
