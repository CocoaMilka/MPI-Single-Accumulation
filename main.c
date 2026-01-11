#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char* argv[])
{
	// MPI Init
	int rank, p, source, dest, tag=0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	MPI_Status status;

	// Generate random arrays
	int data_size = 10;
	int* data = malloc(data_size*sizeof(int));
	
	for (int i = 0; i < data_size; i++)
		data[i] = rand() % 10;

	int* out = NULL;
	// allocate tbd
	if (rank == 0)
		// either return single sum of all arrays, or array of sum of each array

	// Send to root then sum
	reduce_sequential(
		data,			// personal array to send
		out,			// where we sending to
		data_size,		// size of array to send
		MPI_INT,		// type of sent data
		MPI_SUM,		// operation to reduce data
		0,				// root proccess
		MPI_COMM_WORLD
	); 

	// Compare with MPI_Reduce


	MPI_Finalize();
	return 0;
}


void reduce_sequential(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root_process, MPI_Comm comm)
{
	// Only support MPI_INT and MPI_SUM !!!
	if (datatype != MPI_INT || op != MPI_SUM)
		return MPI_ERR_OP;

	// Redefine b/c new scope
	int rank, p;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	int* gather = NULL;
	if (rank == root_process)
		gather = malloc(count*p*sizeof(int)); // number of elements in array * number of arrays * size of int

	MPI_Gather(
		sendbuf,		// address of data to send
		count,			// number of elements to send
		datatype,		// type of sending
		gather,			// where all the data goes to
		count,			// number of elements to receive
		datatype,		// type of received
		root_process,	// who to send to
		comm
	);

	if (rank == root_process)
		for (int i = 0; i < count * p; i++)
			sum += recvbuf[i];	

	free(gather);
}


void reduce_tree()
{

}
