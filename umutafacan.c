#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>


int** maze;
int matrice_size, row_size, row_num_fproc;
int** maze_finish;
int** array_signal;




int checkCells(int i,int j,int proc_id,int **array, int row, int col,int *upperLine, int *lowerLine)
{
	if(array[i][j] == 0)
		return 0;

	int proc_count = col/row;
	int count=0;
	//check above
	if(i>0)
	{
		if(array[i-1][j] == 0)
			count++;
	}
	else if(proc_id>1)
	{
	 	if(upperLine[j]== 0)
	 		count++;
	}
	//check right
	if(j<col-1)
	{
		if(array[i][j+1] == 0)
			count++;
	}
	//check below
	if(i<row-1)
	{
		if(array[i+1][j] == 0)
			count++;
	}else if(proc_id < proc_count)
	{
		if(lowerLine[j] == 0)
			count++;
	}

		// check left
	if(j>0)
	{
		if(array[i][j-1] == 0)
			count++;
	}
	if (count == 3)
	{
		return 2;
	}else
		return 1;
	
} 



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

    array_signal = alloc_2d_int(matrice_size,matrice_size);

   
    int flag=1;

   	for (int i = 1; i < num_procs ; ++i)
   	{
   	 	MPI_Send(&flag,1,MPI_INT,i,15,MPI_COMM_WORLD);
    }

    int iteration = 0;
    while(flag == 1)
    {
    	printf("iteration count : %d ----\n",iteration++);
    	//signals iteration	
    	flag=0;
       	for (int i = 1; i < num_procs ; ++i)
    	{
    		int deadend=0;
    		MPI_Recv(&deadend,1,MPI_INT,i,14,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    		if (deadend == 1)
    		{
    			//if any deadend exist signals next iteration
    			flag=1;
    		}
    	}

    	for (int i = 1; i < num_procs ; ++i)
    	{
    		
    		MPI_Send(&flag,1,MPI_INT,i,15,MPI_COMM_WORLD);
    	}


    }


    int** maze_finish = alloc_2d_int(matrice_size,matrice_size);

    for(int i = 1; i < num_procs; i++) {
      int** node_maze = alloc_2d_int(row_num_fproc, matrice_size);

      MPI_Recv(&(node_maze[0][0]), row_num_fproc*matrice_size, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      for(int k = 0; k < row_num_fproc; k++) {
        for(int l = 0; l < matrice_size; l++) {
          maze_finish[k + ((i - 1) * row_num_fproc)][l] = node_maze[k][l];
        }
      }

    }






    printf("*****************\n");
    for (int i = 0; i < matrice_size; ++i)
    {
     for (int j = 0; j < matrice_size; ++j)
     {
       printf("%d ", maze_finish[i][j]);
     }
     printf("\n");
    }



  } else {
    int row_num_fproc, matrice_size;
    MPI_Recv(&row_num_fproc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&matrice_size, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int** node_maze = alloc_2d_int(row_num_fproc, matrice_size);
    MPI_Recv(&(node_maze[0][0]), row_num_fproc * matrice_size, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);


    //largest iteration parsing all data controlled by master
    int flag;
    MPI_Recv(&flag,1,MPI_INT,0,15,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    while(flag)
    {

    	int lowerLine[matrice_size];
    	int upperLine[matrice_size];
    	//if first slave
	    if(my_id == 1){

	    	 //get lowerline from below neighboor
	    	MPI_Recv(&lowerLine,matrice_size,MPI_INT,my_id+1,30,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

	    	//send to lower neighboor
	    	MPI_Send(&node_maze[row_num_fproc-1][0],matrice_size,MPI_INT,my_id+1,31,MPI_COMM_WORLD);

		
		}
		//other slaves
		else if(my_id<num_procs-1)
		{
			//send upper neighboor
			MPI_Send(&node_maze[0][0],matrice_size,MPI_INT,my_id-1,30,MPI_COMM_WORLD);
			//get from lower neighboor
			MPI_Recv(&lowerLine,matrice_size,MPI_INT,my_id+1,30,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

			//get from upper 
			MPI_Recv(&upperLine,matrice_size,MPI_INT,my_id-1,31,MPI_COMM_WORLD,MPI_STATUS_IGNORE);			
			//send to lower;
			MPI_Send(&node_maze[row_num_fproc-1][0],matrice_size,MPI_INT,my_id+1,31,MPI_COMM_WORLD);
			
			

		}//last slave
		else
		{
			//send to upper
			MPI_Send(&node_maze[0][0],matrice_size,MPI_INT,my_id-1,30,MPI_COMM_WORLD);

			//get from upper
			MPI_Recv(&upperLine,matrice_size,MPI_INT,my_id-1,31,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

		}



    	int deadend=0;
	    for(int i = 0; i< row_num_fproc  ;i++)
	    {
	    	for (int j = 0; j < matrice_size; j++)
	    	{
		    	
		    	int res = checkCells(i,j,my_id,node_maze,row_num_fproc,matrice_size,upperLine,lowerLine);

	    		if(2 == res )
	    		{
	    			node_maze[i][j]=0;
	    			deadend=1; // deadend exists
	    			printf("deadend exists\n");
	    		}
	    		
	    	}
	    }
	   
	    

	    //sends deadend data to master
	    MPI_Send(&deadend,1,MPI_INT,0,14,MPI_COMM_WORLD);

	    //receives next iteration signal
	    MPI_Recv(&flag,1,MPI_INT,0,15,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	  
	    
	}
	  //MPI_Recv(&node_maze, 200, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
/*
    for(int k = 0; k < row_num_fproc ; k++) {
      for(int l = 0; l < matrice_size; l++) {
        //node_maze[k][l] = maze[k + ((i - 1) * row_num_fproc)][l];
        printf("%d ", node_maze[k][l]);
      }
      printf("\n");
    }
      printf("------------------\n");
*/
      MPI_Send(&(node_maze[0][0]),row_num_fproc * matrice_size, MPI_INT, 0, 3, MPI_COMM_WORLD);
  }


  MPI_Finalize();

  return 0;
}
