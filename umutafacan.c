#include <stdio.h>
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>




int*** maze;

//checks 4 neighboor if exists 
int checkNeighboors(int i,int j, int p, int** array, int row, int col)
{
	//wall count
	int count=0;
	//return if its wall already
	if (array[i][j] == 0 ){
			return 0;
	} 
	//check above
	if(i>0)
	{
		if(array[i-1][j] == 0)
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
	}
	// check left
	if(j>0)
	{
		if(array[i][j-1] == 0)
			count++;
	}


	return 0;
} 

//wait
void waitMaster(int i,int j){}
//signal
void signalMaster(int i, int j, int p)
{}
//monitor
void monitorSlaves(int i,int j){}



int **alloc_2d_int(int rows, int cols) {
    int *data = (int *)malloc(rows*cols*sizeof(int));
    int **array= (int **)malloc(rows*sizeof(int*));
    for (int i=0; i<rows; i++)
        array[i] = &(data[cols*i]);

    return array;
}

int ***alloc_3d_int(int l,int m,int n)
{
 	int ***arr3D;
	int i,j,k;

	arr3D = (int***)malloc(l * sizeof(int **));

	for(i=0;i<l;i++)
	{
		arr3D[i] = (int**)malloc(m * sizeof(int*));
		for(j=0;j<m;j++)
		{
			arr3D[i][j] = (int*)malloc(n*sizeof(int));
		}
	}

	return arr3D;
}



int main (int argc, char **argv) {
	int num_procs, my_id;

	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
	MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

	int count = 0;

	int column_count =0;
	int row_count = 0;

	if(my_id == 0) {
		// MASTER - NODE

		// Input Reading
		// Only for Master-Node
		 
  		int matrice_size;



 		FILE *file;
  		file=fopen(argv[1], "r");
  		fscanf(file,"%d",&matrice_size);

	/*matrix*/
	/*Use double , you have floating numbers not int*/

  	/*
	int** mat=malloc(matrice_size*sizeof(int*)); 
	for(i=0;i<matrice_size;++i)
		mat[i]=malloc(matrice_size*sizeof(int));
	*/
		int** mat = alloc_2d_int(matrice_size,matrice_size);

		int i;
  		int j;
 		for(i = 0; i < matrice_size; i++)
 	 	{
      		for(j = 0; j < matrice_size; j++) 
      		{
  		//Use lf format specifier, %c is for character
       			if (!fscanf(file, "%d", &mat[i][j])) 
           			break;
      // mat[i][j] -= '0'; 
      // printf("%d   ",mat[i][j]); //Use lf format specifier, \n is for new line
    		  }
      //printf("\n");
  		}
 	 	fclose(file);

 	 	int row_num_fproc = matrice_size/(num_procs-1);
 	 	printf("num_procs %d\n row_num_fproc %d\n matrice_size %d\n ",num_procs,row_num_fproc,matrice_size);
 	 	
 	 	//alloc global array
 	 	maze=alloc_3d_int(num_procs-1,row_num_fproc,matrice_size);
 	 	//divide matrice to slave processors
 	 	int*** array3d= alloc_3d_int(num_procs-1,row_num_fproc,matrice_size);
 	 	int k;

 	 	for (i=0;i<num_procs-1;i++)
 	 	{
 	 		for(j=0; j< row_num_fproc ; j++)
 	 		{
 	 		
 	 			array3d[i][j] = mat[i*2 +j];
 	 		}	
		 }

	 	int *sizeArray; 
	 	sizeArray =(int *)malloc(2*sizeof(int));
	 	sizeArray[0]=row_num_fproc;
	 	sizeArray[1]=matrice_size; 

	 	//send matrice parts' sizes
	 	for (int i = 1; i < num_procs; ++i)
	 	{
	 		MPI_Send(&sizeArray[0],2,MPI_INT,i,1,MPI_COMM_WORLD);	
	 	}

	 	//sends matrice parts	
	 	for(i=1 ; i < num_procs;i++)
	 	{
	 		MPI_Send(&(array3d[i-1]),row_num_fproc*matrice_size,MPI_INT,i,2,MPI_COMM_WORLD);
	 	}

	 	int ***testArray;
	 	testArray=alloc_3d_int(num_procs-1,row_num_fproc,matrice_size);

	 	
	 	for(i=0;i<row_num_fproc;i++){
	 		for(j=0;j<matrice_size;j++){
	 			monitorSlaves(i,j);
	 		}
	 	}

/*
	 	for(i=1;i<num_procs;i++)
	 	{
	 		
	 		MPI_Recv(&(maze[i-1][0][0]),row_num_fproc*matrice_size,MPI_INT,i,3,MPI_COMM_WORLD,MPI_STATUS_IGNORE);		

	 	}
		
*/
	 	//MPI_Gather(NULL,row_num_fproc*matrice_size,MPI_INT,&(testArray[0][0][0]),row_num_fproc*matrice_size,MPI_INT,0,MPI_COMM_WORLD);

	 	int **temp = alloc_2d_int(row_num_fproc,matrice_size);

	 	MPI_Recv(&(temp[0][0]),row_num_fproc*matrice_size,MPI_INT,1,3,MPI_COMM_WORLD,MPI_STATUS_IGNORE);	
 	
 	
 	 	for (i = 0; i <1;i++)
 	 	{
 			 
 	 		for(j=0;j<row_num_fproc;j++)
 	 		{
 	 			for (int k = 0; k < matrice_size; k++)
 	 			{
 	 				printf("%d\t " ,temp[j][k]); 	 			
 	 			
 	 			}
 	 			printf("\n");
 	 		}
 	 		printf("\n --------------- \n");
 	 	}
 		

	
	} 
	else  //slave part
	{
			
		int* sizeArray;
		sizeArray=(int *)malloc(2*sizeof(int));
		MPI_Recv(&sizeArray[0],2,MPI_INT,0,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		
		int** matrice;
		matrice = alloc_2d_int(sizeArray[0],sizeArray[1]);

		//receive data from master
		int size= sizeArray[0]*sizeArray[1];
		MPI_Recv(&(matrice[0][0]),size,MPI_INT,0,2,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

		int i,j;
		for( i= 0; i < sizeArray[0];i++){
			for(j=0; j < sizeArray[1];j++){
				waitMaster(i,j);
				//matrice[i][j] = checkNeighboors(i,j,my_id,&matrice,sizeArray[0],sizeArray[1]);
				//matrice[i][j] = 0;
				signalMaster(i,j,my_id);

			}
		}

		//MPI_Gather(&(matrice[0][0]),size,MPI_INT,&(maze[my_id-1][0][0]),size,MPI_INT,0,MPI_COMM_WORLD);
		MPI_Send(&(matrice[0][0]),size,MPI_INT,0,3,MPI_COMM_WORLD);

	} //end of slave


	MPI_Finalize();

	return 0;
}
