This project requires MPI to run

## Compiling (for Mac)
`mpicc main.c`

## Executing
`mpirun -np {number of processes} a.out`

:P

## About
Created as part of an exercise for my parallel und verteilte systeme course! This shows 2 different implementations of `MPI_Reduce()` the first using `MPI_Gather()` and the second using `MPI_Send()` and `MPI_Recv()`. The first approach `reduce_sequential()` simply uses `MPI_Gather` to append each process data togther into a single array and calculates the element-wise sum. The second approach `reduce_tree()` uses a tree structure based on the rank of each process to calculate the sum. Each child sends its data to its parent and the parent calculates the element-wise sum of itself and its children before sending it further up the tree. At the end the root process will contain the reduced sum of all processes.
