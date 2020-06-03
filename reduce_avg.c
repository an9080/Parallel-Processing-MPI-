#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>

// Creates an array of random numbers. Each number has a value from 0 - 1
float *create_rand_nums(int num_elements) {
  float *rand_nums = (float *)malloc(sizeof(float) * num_elements);
  assert(rand_nums != NULL);
  int i;
  for (i = 0; i < num_elements; i++) {
    rand_nums[i] = (rand() / (float)RAND_MAX);
  }
  return rand_nums;
}


void my_reduce(float Ldata, float *Gdata,int count, MPI_Datatype datatype, int root,
              MPI_Comm communicator) {
  int world_rank;
  MPI_Comm_rank(communicator, &world_rank);
  int world_size;
  MPI_Comm_size(communicator, &world_size);
int i ;
float Gd = 0 ;
    for (i = 1; i < world_size; i++) {
      if (i != world_rank) {
       MPI_Send(&Ldata, count, datatype,root, 0, communicator);
      }
    }
  if (world_rank == root) {
    // If we are the root process, send our data to everyone
    Gd+=Ldata;
    int i;

    for (i = 0; i < world_size; i++) {
      if (i != world_rank) {
       MPI_Recv(&Ldata, count, datatype,i, 0, communicator, MPI_STATUS_IGNORE);
       Gd+=Ldata;
      }
      
    }

printf("sum = %f \n", Gd);
*Gdata = Gd;
 } else {
    // If we are a receiver process, receive the data from the root
    
   
    MPI_Send(&Gdata, count, datatype, root, 0, communicator);
    
  }
}


int main(int argc, char** argv) {


  if (argc != 2) {
    fprintf(stderr, "Usage: avg num_elements_per_proc\n");
    exit(1);
  }

  int num_elements_per_proc = atoi(argv[1]);
  MPI_Init(NULL, NULL);

  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  double total_my_reduce_time = 0.0;
  double total_mpi_reduce_time = 0.0;

  // Create a random array of elements on all processes.
  srand(time(NULL)*world_rank);   // Seed the random number generator to get different results each time for each processor
  float *rand_nums = NULL;
  rand_nums = create_rand_nums(num_elements_per_proc);

  // Sum the numbers locally
  float local_sum = 0;
  int i;
  for (i = 0; i < num_elements_per_proc; i++) {
    local_sum += rand_nums[i];
  }

  // Print the random numbers on each process
  printf("Local sum for process %d - %f, avg = %f\n",
         world_rank, local_sum, local_sum / num_elements_per_proc);


  // Reduce all of the local sums into the global sum
  float global_sum = 0;

   // Time my_bcast
    // Synchronize before starting timing
  MPI_Barrier(MPI_COMM_WORLD);
  total_my_reduce_time -= MPI_Wtime();
   my_reduce(local_sum,&global_sum, 1, MPI_FLOAT, 0,MPI_COMM_WORLD);
 // Synchronize again before obtaining final time
    MPI_Barrier(MPI_COMM_WORLD);
    total_my_reduce_time += MPI_Wtime();

    // Time MPI_Bcast
    MPI_Barrier(MPI_COMM_WORLD);
    total_mpi_reduce_time -= MPI_Wtime();
    MPI_Reduce(&local_sum , &global_sum , 1 , MPI_FLOAT,MPI_SUM ,0,MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    total_mpi_reduce_time += MPI_Wtime();

  // Print the result
  if (world_rank == 0) {

    printf("Total sum = %f, avg = %f\n", global_sum, global_sum / (world_size * num_elements_per_proc));
    printf("Data size = %d\n", num_elements_per_proc * (int)sizeof(int));
    printf("my_reduce time = %lf\n", total_my_reduce_time );
    printf("MPI_Reduce time = %lf\n", total_mpi_reduce_time);
  
  }

  // Clean up
  free(rand_nums);

  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
//gettimeofday(&t1 , NULL);
//printf("process in %.2g seconds\n", t1.tv_sec - t0.tv_sec + 1E-6 * (t1.tv_usec - t0.tv_usec));
}
