#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

int** maze;
int matrice_size, row_size, row_num_fproc;

int **alloc_2d_int(int rows, int cols) {
    int *data = (int *)malloc(rows*cols*sizeof(int));
    int **array= (int **)malloc(rows*sizeof(int*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}


int main (int argc, char **argv) {
	int num_procs, my_id;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

  if(my_id == 0) {
		// MASTER - NODE

    FILE *file = fopen(argv[1], "r");
    fscanf(file, "%d", &matrice_size);
    maze = alloc_2d_int(matrice_size, matrice_size);

    for (int i = 0; i < matrice_size; i++)
        for (int j = 0; j < matrice_size; j++)
            if (!fscanf(file, "%d", &maze[i][j]))
                break;

    fclose(file);

    row_num_fproc = matrice_size / (num_procs - 1);
    printf("num_procs %d\nrow_num_fproc %d\nmatrice_size %d\n ",num_procs, row_num_fproc, matrice_size);

    for(int i = 1; i < num_procs; i++) {
      int** node_maze = alloc_2d_int(row_num_fproc, matrice_size);

      for(int k = 0; k < row_num_fproc; k++) {
        for(int l = 0; l < matrice_size; l++) {
          node_maze[k][l] = maze[k + ((i - 1) * row_num_fproc)][l];
        }
      }


      //MPI_Send(&i, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(&row_num_fproc, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(&matrice_size, 1 , MPI_INT, i, 1, MPI_COMM_WORLD);
      MPI_Send(&(node_maze[0][0]), row_num_fproc * matrice_size, MPI_INT, i, 2, MPI_COMM_WORLD);
      //break;
    }

  } else {
    int row_num_fproc, matrice_size;
    MPI_Recv(&row_num_fproc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&matrice_size, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int** node_maze = alloc_2d_int(row_num_fproc, matrice_size);
    MPI_Recv(&(node_maze[0][0]), row_num_fproc * matrice_size, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


	  //MPI_Recv(&node_maze, 200, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    for(int k = 0; k < 10; k++) {
      for(int l = 0; l < 20; l++) {
        //node_maze[k][l] = maze[k + ((i - 1) * row_num_fproc)][l];
        printf("%d ", node_maze[k][l]);
      }
      printf("\n");
    }

  }


  MPI_Finalize();

  return 0;
}
