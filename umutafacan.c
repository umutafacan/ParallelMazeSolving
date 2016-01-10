#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define WAIT_TAG 9
#define SIGNAL_TAG 10

int** maze;
int matrice_size, row_size, row_num_fproc;
int** maze_finish;
int** array_signal;

int monitorSlaves(int i, int j, int **array_signal, int size, int proc_num)
{
	int count = 0;
	for (int k = 1; k < proc_num; k++)
	{
		if(array_signal[i+k-1][j] == 1)
			count++;
	}
	
	if(count == proc_num-1)
		return 1;
	else
		return 0;
}
//needed to add indexing
void waitMaster()
{
	int wait = 0;
	while(wait == 0)
	{
		MPI_Recv(&waitMaster,1,MPI_INT,0,WAIT_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}

}

//also need indexes
void signalMaster()
{		
		int message =1; 
		MPI_Send(&message,1,MPI_INT,0,SIGNAL_TAG,MPI_COMM_WORLD);
}





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

    /*
    //monitor slaves
    for(int i = 0 ; i< row_num_fproc;i++)
    {
    	for(int j = 0 ; j < matrice_size; j++)
    	{
    		//sends signal to slaves for iteration
    		for (int k = 1; k < num_procs; k++)
    		{
    			int message=1;
    			MPI_bcast(&message,1,MPI_INT,0,MPI_COMM_WORLD);
    		}
    		int slave=0;
    		//waits until all slaves sent signal for completion
    		while ( 0 == slave )
    		{
    			for(int u = 1 ; u < num_procs; u++)
    			{
    				MPI_Recv(&(array_signal[i+u-1][j]),1,MPI_INT,u,SIGNAL_TAG,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    			}
    			monitorSlaves(i,j,array_signal,matrice_size,num_procs);
    		}

    	}
    }
*/
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

    int flag;
    MPI_Recv(&flag,1,MPI_INT,0,15,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    while(flag)
    {
    	int deadend=0;
	    for(int i = 0; i< row_num_fproc  ;i++)
	    {
	    	for (int j = 0; j < matrice_size; j++)
	    	{
		    	int res = checkCells(i,j,my_id,node_maze,row_num_fproc,matrice_size);
	    		if(2 == res )
	    		{
	    			node_maze[i][j]=0;
	    			deadend=1; // deadend exists
	    			printf("deadend exists\n");
	    		}
	    		
	    	}
	    }
	    int send_sig = 0;
	   
	    //if first slave
	    if(my_id == 1){

	    	 MPI_Send(&send_sig,1,MPI_INT,proc_id+1)

	    	while(1 == 1){
	    		int neighboor_signal_down; 
	    		MPI_Recv(&neighboor_signal_down,my_id+1,MPI_INT,2,30,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	    		if(neighboor_signal_down == 1)
	    		{
	    			int sig = 1;
	    			MPI_Send(&sig,1,MPI_INT,my_id+1,31,MPI_COMM_WORLD);
	    			int* array =malloc(2*sizeof(int)); 
	    			MPI_Recv(&array,2,MPI_INT,my_id+1,32,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	    			MPI_Send(&node_maze[array[0]][array[1]],1,MPI_INT,my_id+1,33,MPI_COMM_WORLD);
	    		}
	    		else{
	    			break
	    		}
	    	}	

		
		}
		//last slave
		else if(my_id==num_procs-1)
		{



		}//other slaves
		else
		{



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
