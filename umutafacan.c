#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>



int nodes[100][3];
int maze[100][100]

void allocate_mem(int*** arr, int n)
{
  *arr = (int**)malloc(n*sizeof(int*));
  for(int i=0; i<n; i++)
    (*arr)[i] = (int*)malloc(n*sizeof(int));
} 


int main (int argc, char **argv) {
	int num_procs, my_id;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	double * data = 0;
	int count = 0;

	int column_count =0;
	int row_count = 0;

	if(my_id == 0) {
		// MASTER - NODE

		// Input Reading
		// Only for Master-Node
		FILE *input = fopen (argv[1], "r");


		char line [256]; // temp variable for per line
		char *column; // temp variable for per column eg accX, accY

		int is_header = 1;
		int matrice_lenght;


		while (fgets(line, sizeof line, input) != NULL) {

			// pass header of text file, starting line 1
			if(is_header == 1) {
				column = strtok(line,"\t");
				column = strtok(NULL,"\t");

				matrice_lenght = atoi(column);
				is_header = 0;
				continue;
			}

			//allocates maze
			allocate_mem(maze,matrice_lenght);
			
			// splits line by tab charecter
			column = strtok(line, "\t");

			
			int column_count = 0;
			while (column != NULL) {
				column = strtok(NULL, "\t");
				// writes int vals to apporiate arrays
				maze[row_count][column_count] = atoi(column);
				// increases column
				column_count++;
			}

			// increases number of rows
			row_count++;
		}
		fclose (input);



	} else {
		
		


	}


	MPI_Finalize();

	return 0;
}
