
# MPI Exercises

## Overview
This repository, "mpi_exercises",  is part of a university project for the course "Introduction to Parallel Computing". It contains a series of exercises showcasing the use of the Message Passing Interface (MPI) for parallel computing. The project is affiliated with the University of West Attica (Uniwa), under the undergraduate master's program for Computer Engineering.

## Course Details
- **Course Name:** Introduction to Parallel Computing
- **Institution:** University of West Attica (Uniwa)
- **Program:** Undergraduate Master in Computer Engineering

## Contents
The repository consists of exercises that gradually introduce different elements of MPI programming, including:
- Basic setup of the MPI environment
- Point-to-point and collective communication
- Working with MPI derived data types
- Implementation of parallel algorithms

Each directory or file is dedicated to a specific MPI programming exercise or concept.

## Getting Started

### Prerequisites
- MPI implementation (e.g., MPICH or Open MPI)
- C/C++ compiler
- Fundamental knowledge of parallel computing and C/C++ programming

### Installation
1. Clone the repository:
   ```
   git clone https://github.com/stafidopsomo/mpi_exercises.git
   ```
2. Navigate to the exercise directory of choice.
3. Compile using an MPI compiler (e.g., `mpicc`):
   ```
   mpicc -o exercise_name exercise_name.c
   ```

### Running the Exercises
Execute an MPI program using:
```
mpirun -np <number_of_processes> ./exercise_name
```
Replace `<number_of_processes>` with the desired count of parallel processes.

## Contributing
Contributions to "mpi_exercises" are encouraged and appreciated. This can include adding new exercises, bug fixes, or documentation enhancements. Please see `CONTRIBUTING.md` for contribution guidelines.

## License
This project is under the MIT License - refer to the `LICENSE` file for more details.

## Acknowledgments
- Appreciation to those whose code was utilized
- Special thanks to the "Introduction to Parallel Computing" course instructors
- Additional acknowledgments to peers and collaborators
