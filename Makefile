#Complier
CC = gcc
CFLAGS = -Wall -Wextra -std=c99

#MPI Complier
MPICC = mpicc

#Files
SERIAL_SRC = bitonic.c
OMP_SRC = bitonic_omp.c
MPI_SRC = bitonic_mpi.c

# Outputs
SERIAL_OUT = bitonic_serial
OMP_OUT = bitonic_omp
MPI_OUT = bitonic_mpi

#Default target
all: $(SERIAL_OUT) $(OMP_OUT) $(MPI_OUT)

# Serial implementation
$(SERIAL_OUT): $(SERIAL_SRC)
	$(CC) $(CFLAGS) -o $(SERIAL_OUT) $(SERIAL_SRC)

# OpenMP implementation
$(OMP_OUT): $(OMP_SRC)
	$(CC) $(CFLAGS) -fopenmp -o $(OMP_OUT) $(OMP_SRC)

# MPI implementation
$(MPI_OUT): $(MPI_SRC)
	$(MPICC) $(CFLAGS) -o $(MPI_OUT) $(MPI_SRC)

# Clean
clean:
	rm -f $(SERIAL_OUT) $(OMP_OUT) $(MPI_OUT) $(MPI2_OUT)

.PHONY: all clean