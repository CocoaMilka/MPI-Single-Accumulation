#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void reduce_sequential(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root_process, MPI_Comm comm);

int main(int argc, char* argv[])
{
	// MPI Init
	int rank, p, source, dest, tag=0;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	MPI_Status status;

	// Generate random arrays
	srand(time(NULL));
	int data_size = 10;
	int* data = malloc(data_size*sizeof(int));
	
	for (int i = 0; i < data_size; i++)
		data[i] = rand() % 10;

	int* out = NULL;
	int* out_mpi = NULL;	

	// Reduce SUM sums element-wise, p0_data[i] + p1_data[i] + ...
	// Returns an array of size 'count'
	if (rank == 0)
	{
		out = malloc(data_size * sizeof(int));
		out_mpi = malloc(data_size * sizeof(int));
	}

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
	MPI_Reduce(
		data,
		out_mpi,
		data_size,
		MPI_INT,
		MPI_SUM,
		0,
		MPI_COMM_WORLD
	);
	
	if (rank == 0)
	{
		for (int i = 0; i < data_size; i++)
			if (out[i] != out_mpi[i])
				printf("Missmatch at index: %d, got %d expected %d", i, out[i], out_mpi[i]);
	
		printf("reduce_sequential: \t");
		for (int i = 0; i < data_size; i++)
			printf(" %d ", out[i]);
	
		printf("\nMPI_Reduce: \t");
		for (int i = 0; i < data_size; i++)
			printf(" %d ", out_mpi[i]);

		free(out);
		free(out_mpi);
	}

	free(data);

	MPI_Finalize();
	return 0;
}


void reduce_sequential(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root_process, MPI_Comm comm)
{
	// Only support MPI_INT and MPI_SUM !!!
	if (datatype != MPI_INT || op != MPI_SUM)
	{	
		printf("ERROR: only MPI_INT and MPI_SUM implemented!!");
		return;
	}

	// Redefine b/c new scope
	int rank, p;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &p);

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
	{
		int* out = (int*)recvbuf;

		for (int i = 0; i < count; i++)
			out[i] = 0;

		for (int i = 0; i < p; i++)
			for (int j = 0; j < count; j++)
				out[j] += gather[i * count + j]; // recvbuf[j] = Σ sendbuf_rank[j]
		
		free(gather);
	}
}


void reduce_tree()
{

}
