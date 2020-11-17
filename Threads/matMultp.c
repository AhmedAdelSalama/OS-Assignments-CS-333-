#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#define N 1000

/*declaring i/p and o/p files*/
FILE *input1;
FILE *input2;
FILE *output;
/*defining structures*/
/*a struct to hold the dimensions of each element*/
struct dimensions{
    int col2,row1;
};
/*a struct to hold the resulting row*/
struct rowStruct{
    double *rowTemp;
};
/*a struct to hold the resulting element*/
struct element{
    double val;
};
/*declaring global variables*/
double *A[N];
double *B[N];
int row1 =0, col1 = 0, row2 =0, col2 = 0;
/*Functions prototypes*/
/*
Method 1: compute each row in the output matrix.
args: rowIndex, indicating the index of the row to be computed
return: a pointer to a structure(rowStruct) containing a pointer to the computed row.
*/
void *method_1(void *rowIndex);
/*
Method 2: compute each element in the output matrix.
args: id, a structure(dimensions) indicating the index of the element to be computed.
return: a pointer to a structure(rowStruct) containing a pointer to the computed row.
*/
void *method_2(void *di);
/*reads #cols and #rows for a matrix*/
void readDimensions(FILE *input, int *row, int *col);
/*reads matrix elements*/
void readMatrix(FILE *input, int row, int col, double *mat[]);






int main(int argc, char *argv[])
{
    /*if the argv contains i/p and o/p files, open those files.*/
    if(argc == 4){
        input1 = fopen(argv[1],"r");
        input2 = fopen(argv[2],"r");
        output = fopen(argv[3],"w");
    }
    /*if not, open testing files*/
    else{
        input1 = fopen("./res/a.txt","r");
        input2 = fopen("./res/b.txt","r");
        output = fopen("c.out","w");
    }

    /*Reading Matrices from input files*/
    readDimensions(input1, &row1, &col1);
    readDimensions(input2, &row2, &col2);
    readMatrix(input1, row1, col1, A);
    readMatrix(input2, row2, col2, B);
    /*-----------------------------------------------------------------------*/
    /*Method 1: */
    /*-----------------------------------------------------------------------*/
    fputs("Matrix Multiplication Method 1:\n", output);
    /*measure the execution time*/
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    /*create threads*/
    pthread_t threads_1[row1];
    for(long i =0; i < row1; i++){
        if(pthread_create(&threads_1[i], NULL, method_1, (void*)i)){
            printf("Cannot create thread\n");
            exit(1);
        }
    }
    /*join threads and get the value out of each thread*/
    double *res1[row1];
    for(int i = 0; i < row1; ++i){
        struct rowStruct *temp ;
        res1[i] = malloc(col2*sizeof(double));
        pthread_join(threads_1[i],(void **)&temp);
        res1[i] = temp->rowTemp;
    }
    /*Writing resulting matrix to o/p file*/
    printf("row= %d\tcol= %d\n",row1, col2);
    fprintf(output, "row= %d\tcol= %d\n",row1, col2);
    for(int i = 0; i < row1; ++i){
        for(int j = 0; j< col2; ++j){
            fprintf(output, "%lf\t",res1[i][j]);
            printf("%lf\t",res1[i][j]);
        }
        fprintf(output, "\n");
        printf("\n");
    }
    /*measure the execution time*/
    gettimeofday(&stop, NULL);
    fprintf(output, "Number of threads: %d\n", row1);
    fprintf(output, "Seconds taken: %lu\n", stop.tv_sec - start.tv_sec);
    fprintf(output, "Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    /*free memory allocation*/
    for(int i = 0; i < row1; ++i){
        free(res1[i]);
    }
    /*-----------------------------------------------------------------------*/
    /*Method 2: */
    /*-----------------------------------------------------------------------*/
    fputs("\nMatrix Multiplication Method 2:\n", output);
    /*measure the execution time*/
    struct timeval start_2, stop_2;
    gettimeofday(&start_2, NULL);

    /*create threads*/
    int count =0 , threadsNum = row1*col2;
    struct dimensions *d[threadsNum];
    int structCount =0;
    pthread_t threads_2[threadsNum];
    for(int i =0; i <row1 ; i++){
        for(int j = 0; j <col2; j++){
            /*dimensions struct to hold the index of the element to be computed*/
            d[structCount] = (struct dimensions *)malloc(sizeof(struct dimensions));
            d[structCount]->row1 = i; d[structCount]->col2 = j;
            if(pthread_create(&threads_2[count++], NULL, method_2, (void*)d[structCount++])){
                printf("Cannot create thread\n");
                exit(1);
            }
        }
    }
    /*join threads and get the value out of each thread*/
    double res2[row1][col2];
    count =0;
    structCount = 0;
    struct element *temp[threadsNum];
    for(int i = 0; i < row1; ++i){
        for(int j = 0; j <col2; j++){
        temp[structCount] = (struct element *)malloc(sizeof(struct element));
            pthread_join(threads_2[count++],(void **)&temp[structCount]);
            res2[i][j] = temp[structCount++]->val;
        }
    }
    /*Writing resulting matrix to o/p file*/
    printf("row= %d\tcol= %d\n",row1, col2);
    fprintf(output, "row= %d\tcol= %d\n",row1, col2);
    for(int i = 0; i < row1; ++i){
        for(int j = 0; j< col2; ++j){
            fprintf(output, "%lf\t",res2[i][j]);
            printf("%lf\t",res2[i][j]);
        }
        fprintf(output, "\n");
        printf("\n");
    }
    /*measure the execution time*/
    gettimeofday(&stop_2, NULL);
    fprintf(output, "Number of threads: %d\n" , threadsNum);
    fprintf(output, "Seconds taken: %lu\n", stop_2.tv_sec - start_2.tv_sec);
    fprintf(output, "Microseconds taken: %lu\n", stop_2.tv_usec - start_2.tv_usec);
    /*free memory allocation*/
    for(int i = 0; i < threadsNum; ++i){
        free(d[i]);
        free(temp[i]);
    }
    /*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
    /*free memory allocation*/
    for(int i = 0; i < row1; ++i){
        free(A[i]);
    }
    for(int i = 0; i < row2; ++i){
        free(B[i]);
    }
    fclose(input1);
    fclose(input2);
    fclose(output);
    return 0;
}





/*
Method 1: compute each row in the output matrix.
args: rowIndex, indicating the index of the row to be computed
return: a pointer to a structure(rowStruct) containing a pointer to the computed row.
*/
void *method_1(void *rowIndex){
    if(col1!=row2){
        printf("ERROR: Matrices cannot not be multiplied.");
        exit(1);
    }
    long n = (long) rowIndex;
    double *row = malloc(col2*sizeof(double));
    for(int j = 0; j<col2; ++j){
        double res =0;
        for(int k = 0; k<col1; ++k){
            res += (A[n][k]*B[k][j]);
        }
        row[j] = res;
    }
    struct rowStruct *temp =(struct rowStruct *)malloc(sizeof(struct rowStruct)); 
    temp->rowTemp = row;
    pthread_exit((void*)temp);
}
/*
Method 2: compute each element in the output matrix.
args: id, a structure(dimensions) indicating the index of the element to be computed.
return: a pointer to a structure(rowStruct) containing a pointer to the computed row.
*/
void *method_2(void *di){
    if(col1!=row2){
        printf("ERROR: Matrices cannot not be multiplied.");
        exit(1);
    }
    struct dimensions *d;
    d = (struct dimensions*)di;
    int n = d->row1, m = d->col2;
    /*double*/
    double res =0;
    for(int k = 0; k<col1; ++k){
        res += (A[n][k]*B[k][m]);
    }
    struct element *temp = (struct element *)malloc(sizeof(struct element));
    temp->val = res;
    pthread_exit((void*)temp);
}
/*reads #cols and #rows for a matrix*/
void readDimensions(FILE *input, int *row, int *col){
    char c = fgetc(input);
    int count=0;
    while(1){
        if(c == '='){
            if(count){
                fscanf(input, "%d", col);
                return;
            }else{
                fscanf(input, "%d", row);
                count++;
            }
        }
        c = fgetc(input);
    }
}
/*reads matrix elements*/
void readMatrix(FILE *input, int row, int col, double *mat[]){
    for(int i = 0; i < row; ++i){
        double *temp = malloc(col*sizeof(double));
        /*read one element in the matrix*/
        for(int j = 0; j< col; ++j){
            fscanf(input, "%lf", &temp[j]);
        }
        mat[i] = temp;
    }
}
