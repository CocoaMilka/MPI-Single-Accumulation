#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

void reduce_sequential(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root_process, MPI_Comm comm);
void reduce_tree(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root_process, MPI_Comm comm);

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
	int* out_tree = NULL;
	int* out_mpi = NULL;	

	// Reduce SUM sums element-wise, p0_data[i] + p1_data[i] + ...
	// Returns an array of size 'count'
	if (rank == 0)
	{
		out = malloc(data_size * sizeof(int));
		out_tree = malloc(data_size * sizeof(int));
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

	// Send to root then sum
	reduce_tree(
		data,			// personal array to send
		out_tree,		// where we sending to
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

		printf("\nreduce_tree: \t");
			for (int i = 0; i < data_size; i++)
				printf(" %d ", out_tree[i]);

		printf("\nMPI_Reduce: \t");
		for (int i = 0; i < data_size; i++)
			printf(" %d ", out_mpi[i]);

		free(out);
		free(out_tree);
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


void reduce_tree(const void* sendbuf, void* recvbuf, int count, MPI_Datatype datatype, MPI_Op op, int root_process, MPI_Comm comm)
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

	int* local_sum = malloc(count*sizeof(int)); // number of elements in array * number of arrays * size of int
	memcpy(local_sum, sendbuf, count * sizeof(int)); // fill array with initial data of each node

	int* tmp = malloc(count*sizeof(int)); // use as temp buffer to store children data

	// Tree reduction
	int parent = (rank - 1) / 2;	// parent of rank = (r - 1) / 2
	int child_l = 2 * rank + 1;		// left child
	int child_r = 2 * rank + 2;		// right child
	
	if (child_l > p && child_r > p) // if node is leaf, send to parent	
		MPI_Send(sendbuf, count, datatype, parent, 0, comm);
	
	if (child_l < p) // receive and sum left child data to own data
	{	
		MPI_Recv(tmp, count, datatype, child_l, 0, comm, MPI_STATUS_IGNORE);
		
		for (int i = 0; i < count; i++)
			local_sum[i] += tmp[i];
	}

	if (child_r < p) // receive and sum right child data to own data
	{
		MPI_Recv(tmp, count, datatype, child_r, 0, comm, MPI_STATUS_IGNORE);
		
		for (int i = 0; i < count; i++)
			local_sum[i] += tmp[i];
	}

	if (rank != root_process) // node has received data from children and summed, now send up
		MPI_Send(local_sum, count, datatype, parent, 0, comm);

	if (rank == root_process)
	{	
		int* out = (int*)recvbuf;

		for (int i = 0; i < count; i++)
			out[i] = local_sum[i];
	}	

	// cleanup
	free(tmp);
	free(local_sum);
}
