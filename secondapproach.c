/*
Student Name: UMUT AFACAN
Student Number: 2010400141
Compile Status: Compiling
Program Status: Working
Notes: Matrice size should be divisible by square_root(proccessor count-1).
the 1 for master. This code for second approach
*/

#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>


int** maze;
int matrice_size, row_size, row_num_fproc;
int** array_signal;

#define A_DIRECTION 30
#define B_DIRECTION 31
#define C_DIRECTION 32
#define D_DIRECTION 33

#define NEXT_ITERATION 15
#define DEADEND_SIGNAL 14



int checkCells(int i,int j,int proc_id,int **array, int row, int col,int *upperLine, int *lowerLine,int *rightLine,int *leftLine,int p)
{
	if(array[i][j] == 0){
		return 0;// cell is already wall or marked
  }else{

    	
    	int count=0;
    	//check above
    	if(i > 0)
    	{
    		if(array[i-1][j] == 0){
    			count++;
        }
    	}
    	else if((proc_id-1)/p > 0 )
    	{
    	 	
        if(upperLine[j]== 0){

    	 		count++;
        }
    	}
    	//check right
    	if(j < col-1)
    	{
    		if(array[i][j+1] == 0){
    			count++;
        }
    	}
      else if( (proc_id-1)%p < p-1)
      {
        if(rightLine[i] == 0){
          count++;
        }
      }

    	//check below
    	if(i < row-1)
    	{
    		if(array[i+1][j] == 0)
    			count++;
    	}else if( (proc_id-1)/p < p-1 )
    	{
    		if(lowerLine[j] == 0)
    			count++;
    	}

    		// check left
    	if(j > 0)
    	{
    		if(array[i][j-1] == 0){
    			count++;
        }
    	}else if( (proc_id-1) % p > 0)
      {
        if(leftLine[i] == 0){
          count++;
        }
      }


    	if (count > 2 )
    	{
    		return 2; //cell is surrounded by 3 walls
    	}else
      {
    		return 1; // cell is not surrounded by 3 walls
      }
	}
} 

int *getLowestRow(int** array, int n)
{
  int* result = (int *)malloc(n*sizeof(int));
  for(int i = 0 ;i< n;i++)
    result[i] = array[n-1][i];

  return result;
}
int *getUppestRow(int** array, int n)
{
  int* result = (int *)malloc(n*sizeof(int));
  for(int i = 0 ;i< n;i++)
    result[i] = array[0][i];
  
  return result;
}
int *getLeftestColumn(int** array, int n)
{
 int* result = (int *)malloc(n*sizeof(int));
  for(int i = 0 ;i< n;i++)
    result[i] = array[i][0];
  
  return result;
}
int *getRightestColumn(int** array, int n)
{
  int* result = (int *)malloc(n*sizeof(int));
  for(int i = 0 ;i< n;i++)
    result[i] = array[i][n-1];
  
  return result;
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

    //manage output file
    //output file
    char output[80] = "";
    char *val = strtok(argv[2], ".");
    strcat(output, val);
    val = strtok(NULL, ":");
    strcat(output, ".txt");

    // redirecting stdout to file.
    stdout = freopen(output, "w", stdout);


    int p = sqrt(num_procs-1); 
    int sub_size = matrice_size/p;

    

   
    //split maze to smaller parts and distribute to slaves
   for(int i = 1; i <= p ; i++)
   {
      for(int j = 1 ; j <=p ; j++)
      {
        int** node_maze = alloc_2d_int(sub_size,sub_size);
        for(int k= 0; k< sub_size; k++)
        {
          for(int l= 0; l<sub_size ; l++)
          {
            node_maze[k][l] = maze[k + (i-1)*sub_size] [l + (j-1)*sub_size];

          }
        }

      MPI_Send(&sub_size, 1, MPI_INT, ((i-1)*p)+j, 0, MPI_COMM_WORLD);
      MPI_Send(&matrice_size, 1 , MPI_INT, ((i-1)*p)+j, 1, MPI_COMM_WORLD);
      MPI_Send(&(node_maze[0][0]), sub_size * sub_size, MPI_INT, ((i-1)*p)+j, 2, MPI_COMM_WORLD);


      }

   }
    

    
    //controls deadends and sends signals to slaves to make iteration
    int flag=1;

   	for (int i = 1; i < num_procs ; ++i)
   	{
   	 	MPI_Send(&flag,1,MPI_INT,i,15,MPI_COMM_WORLD);
    }

    int iteration = 0;
    while(flag == 1 )
    {

    	
    	//signals iteration	
    	flag=0;
       	for (int i = 1; i < num_procs ; i++)
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

    for(int i = 1; i <= p; i++) {
      for(int j = 1 ; j <= p; j++ ){
        int** node_maze = alloc_2d_int(sub_size, sub_size);

        MPI_Recv(&(node_maze[0][0]), sub_size*sub_size, MPI_INT, ((i-1)*p)+j, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int k = 0; k < sub_size; k++) {
          for(int l = 0; l < sub_size; l++) {
            maze_finish[k + ((i - 1) * sub_size)][l+ (j-1)*sub_size] = node_maze[k][l];
          }
        }
      }

    }


    //print out for the file
    for (int i = 0; i < matrice_size; ++i)
    {
     for (int j = 0; j < matrice_size; ++j)
     {
       printf("%d ", maze_finish[i][j]);
     }
     printf("\n");
    }



  } else {
    int sub_size, matrice_size;
    MPI_Recv(&sub_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&matrice_size, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int** node_maze = alloc_2d_int(sub_size, sub_size);
    MPI_Recv(&(node_maze[0][0]), sub_size * sub_size, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    //p squera root of slaves
    int p = sqrt(num_procs-1);

   // printf("my id %d  p %d  num_procs %d \n",my_id,p,num_procs );
    //largest iteration parsing all data controlled by master
    int flag;
    MPI_Recv(&flag,1,MPI_INT,0,15,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    while(flag)
    {

      	int *lowerLine = (int *)malloc(sub_size*sizeof(int));
      	int *upperLine= (int *)malloc(sub_size*sizeof(int));
        int *rightLine= (int *)malloc(sub_size*sizeof(int));
        int *leftLine= (int *)malloc(sub_size*sizeof(int));
        
      	//if first column
  	    if( (my_id-1)%p == 0){
  	    	 //get right from right neighboor
  	    	MPI_Recv(&(rightLine[0]),sub_size,MPI_INT,my_id+1,30,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

  	    	//send to left neighboor
          int* rightest= malloc(sub_size*sizeof(int));
          rightest=getRightestColumn(node_maze,sub_size);

  	    	MPI_Send(&(rightest[0]),sub_size,MPI_INT,my_id+1,31,MPI_COMM_WORLD);

  		
    		}
    		//other slaves
    		else if( (my_id-1)%p <p-1)
    		{
    			//send upper neighboor
          int* leftest=getLeftestColumn(node_maze,sub_size);
    			MPI_Send(&leftest[0],sub_size,MPI_INT,my_id-1,30,MPI_COMM_WORLD);
    			//get from right neighboor
    			MPI_Recv(&rightLine[0],sub_size,MPI_INT,my_id+1,30,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

    			//get from left 
    			MPI_Recv(&leftLine[0],sub_size,MPI_INT,my_id-1,31,MPI_COMM_WORLD,MPI_STATUS_IGNORE);			
    			//send to right;
          int* rightest=getRightestColumn(node_maze,sub_size);
    			MPI_Send(&rightest[0],sub_size,MPI_INT,my_id+1,31,MPI_COMM_WORLD);
    			
    			

    		}//last column
    		else
    		{
    			//send to upper
          int* leftest=getLeftestColumn(node_maze,sub_size);
    			MPI_Send(&leftest[0],sub_size,MPI_INT,my_id-1,30,MPI_COMM_WORLD);

    			//get from upper
    			MPI_Recv(&leftLine[0],sub_size,MPI_INT,my_id-1,31,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

    		}
        
          //if first row
        if( (my_id-1)/p == 0){
           //get lowerline from below neighboor
          MPI_Recv(&lowerLine[0],sub_size,MPI_INT,my_id+p,32,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

          //send to lower neighboor
          int* lowest = getLowestRow(node_maze,sub_size);
          MPI_Send(&lowest[0],sub_size,MPI_INT,my_id+p,33,MPI_COMM_WORLD);

      
        }
        //other slaves
        else if( (my_id-1)/p <p-1)
        {
          
          //send upper neighboor
          int* uppest=getUppestRow(node_maze,sub_size);
          MPI_Send(&uppest[0],sub_size,MPI_INT,my_id-p,32,MPI_COMM_WORLD);
          //get from lower neighboor
          MPI_Recv(&lowerLine[0],sub_size,MPI_INT,my_id+p,32,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

          //get from upper 
          MPI_Recv(&upperLine[0],sub_size,MPI_INT,my_id-p,33,MPI_COMM_WORLD,MPI_STATUS_IGNORE);      
          //send to lower;
          int* lowest=getLowestRow(node_maze,sub_size);
          MPI_Send(&lowest[0],sub_size,MPI_INT,my_id+p,33,MPI_COMM_WORLD);
          
          

        }//last row
        else
        {
          //send to upper
          int* uppest=getUppestRow(node_maze,sub_size);
          MPI_Send(&uppest[0],sub_size,MPI_INT,my_id-p,32,MPI_COMM_WORLD);

          //get from upper
          MPI_Recv(&upperLine[0],sub_size,MPI_INT,my_id-p,33,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        }



    	int deadend=0;
	    for(int i = 0; i< sub_size  ;i++)
	    {
	    	for (int j = 0; j < sub_size; j++)
	    	{
		    	//checkCells(int i,int j,int proc_id,int **array, int row, int col,int *upperLine, int *lowerLine,int *rightLine,int *leftLine,int p)
		    	int res = checkCells(i,j,my_id,node_maze,sub_size,sub_size,upperLine,lowerLine,rightLine,leftLine,p);
	    		if(2 == res )
	    		{
	    			node_maze[i][j] = 0;
	    			deadend=1; // deadend exists
	    			//printf("deadend exists\n");
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
    for(int k = 0; k < sub_size ; k++) {
      for(int l = 0; l < sub_size; l++) {
        //node_maze[k][l] = maze[k + ((i - 1) * row_num_fproc)][l];
        printf("%d ", node_maze[k][l]);
      }
      printf("\n");
    }
      printf("------------------\n");
*/

      MPI_Send(&(node_maze[0][0]),sub_size * sub_size, MPI_INT, 0, 3, MPI_COMM_WORLD);
  }


  MPI_Finalize();

  return 0;
}
