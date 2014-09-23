#include "experimento1.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

int expow(int number,int exponential){
    int result = 0;
    int i;
    if(exponential == 0)
        return 1;
    result = number;
    for (i = 1; i < exponential; i++){
        result *= number; 
    }
    return result;
}

int **allocateMatrix(int row, int col){
    int **matrix;
    int i;
    matrix = (int **)malloc(sizeof(int *) * row);
    if (matrix == NULL) {
        perror("FAILURE in the allocation of the rows.");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < row; ++i) {
        matrix[i] = (int *)malloc(sizeof(int) * col);
        if (matrix[i] == NULL) {
            perror("FAILURE in the allocation of the columns.");
            exit(EXIT_FAILURE);
        }
    }
    return matrix;
}

float **allocateMatrixErr(int row, int col){
    float **matrix;
    int i;
    matrix = (float **)malloc(sizeof(float *) * row);
    if (matrix == NULL) {
        perror("FAILURE in the allocation of the rows.");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < row; ++i) {
        matrix[i] = (float *)malloc(sizeof(float) * col);
        if (matrix[i] == NULL) {
            perror("FAILURE in the allocation of the columns.");
            exit(EXIT_FAILURE);
        }
    }
    return matrix;
}

void freeMatrix(int **matrix, int row){
    int i;
    for (i = 0; i < row; ++i)
        free(matrix[i]);
    free(matrix);
}

char *prepareString(char *s){
    int i=0;
    char *numbers = malloc(1);

    while(s[i]!='\0'){
        size_t length = strlen(numbers);
        if( isdigit(s[i])){
            numbers = (char *) realloc(numbers,length + 1);
            numbers[length] = s[i];
            numbers[length + 1] = '\0';
        }
        else if (s[i] == '_'){ 
            numbers = (char *) realloc(numbers,length + 1);
            numbers[length] = ' ';
            numbers[length + 1] = '\0';
        }     
        i++;
    }
    return numbers;
}

PGM* readPGM(char *filename, PGM *data){
    percentage = 0;
    if (debug){
        sleep(1);
    }
    execution = 0; 
    FILE *pgmImage;
    char *imageLength;
    int i, j, garbage,width,heigth,low;
    float divisor,w,error;

    pgmImage = fopen(filename, "rb");
    imageLength = prepareString(filename);
    int boolean = sscanf(imageLength,"%d %d %d",&garbage,&width,&heigth);
    
    if (boolean){
        data->max_gray = 1;
        data->col = width;
        data->row = heigth;
    }
    else
        fprintf(stderr, "Cannot get image width and heigth.\n");

    if (pgmImage == NULL) {
        fprintf(stderr,"The IMAGE %s cannot be opened.\n",filename);
        exit(-1);
    }

    data->matrix = allocateMatrix(data->row, data->col);
    data->matrixErr = allocateMatrixErr(data->row, data->col);

    for (i = 0; i < data->row; ++i){
        for (j = 0; j < data->col; ++j) {
            low = fgetc(pgmImage);
            divisor = (float)low/255;
            w = divisor - data->matrixErr[i][j];
            if (w >= THRESHOLD){
                data->matrix[i][j] = 1;
            }
            else{
                data->matrix[i][j] = 0;
            }
            error = data->matrix[i][j] - w;
            propagateError(data, i, j, error);
        }
    }
    fclose(pgmImage);
    percentage += 20;
    return data;
}

void writePGM(const char *filename, const PGM *data, int flag){
    FILE *pgmImage;
    int i, j;
    int low;
 
    if (debug){
        sleep(1);
    }
    execution = 1;
    pgmImage = fopen(filename, "wb");
    if (pgmImage == NULL) {
        perror("Cannot OPEN the file to write");
        exit(-1);
    }
    if (flag){
        fprintf(pgmImage, "P5 ");
        fprintf(pgmImage, "%d %d ", data->col, data->row);
        fprintf(pgmImage, "%d ", data->max_gray);
    }
    for (i = 0; i < data->row; ++i){
            for (j = 0; j < data->col; ++j) {
                low = LO(data->matrix[i][j]);
                fputc(low, pgmImage);
            }
    }
    percentage += 10;
    fclose(pgmImage);
}

void propagateError(PGM *data, int row, int col, float error){

    int i,j; 
    for (i = row; i <= (row + 2) ; i++){
        for (j = (col - 2); j <= (col + 2); j++){
            if ((i < 0) || (i >= data->row) || (j < 0) || (j >= data->col)) continue;
            if ((i == row) && (j == col)) continue;
            if (i == row){
                if (j > col){
                    if(j == (col + 1))
                        data->matrixErr[i][j] += (error * 4/21);
                    else
                        data->matrixErr[i][j] += (error * 2/21);
                }
            }
            else{
                if (i == (row + 1)){
                    if (j == (col-2))
                        data->matrixErr[i][j] += (error * 1/21);
                    else if (j == (col-1))
                        data->matrixErr[i][j] += (error * 2/21);
                    else if (j == col)
                        data->matrixErr[i][j] += (error * 4/21);
                    else if (j == (col+1))
                        data->matrixErr[i][j] += (error * 2/21);
                    else if (j == (col+2))
                        data->matrixErr[i][j] += (error * 1/21);
                }
                else{
                    if (j == (col-2))
                        data->matrixErr[i][j] += (error * 1/42);
                    else if (j == (col-1))
                        data->matrixErr[i][j] += (error * 1/21);
                    else if (j == col)
                        data->matrixErr[i][j] += (error * 2/21);
                    else if (j == (col+1))
                        data->matrixErr[i][j] += (error * 1/21);
                    else if (j == (col+2))
                        data->matrixErr[i][j] += (error * 1/42);
                }
            }
        }
    }
}

void append(char *string, char c){
        int len = strlen(string);
        string[len] = c;
        string[len+1] = '\0';
}

int binaryToDecimal(char *string){
    int i,number,total=0,exponential=0;
    for (i = (strlen(string) - 1); i > 0; i--)
    {
        number = string[i] - '0';
        if (number)
            total += expow(2,exponential);
        exponential++;
    }
    return total;
}

void *matrixHistogram(void *parameters){
    int i,j,index,steps,j_begin,j_end;
    char binary[50] = "B";
    char c;
    Params* params = (Params*) parameters;
    j_begin = 0;
    j_end = 4;
    for (steps = 0; steps < ((params->max_col+1)/5); steps++){
        for (i = params->top; i <= params->low;i++){
            for (j = j_begin; j <= j_end; j++){
                if ( (i == (params->top+2)) && (j == (j_begin+2))){
                    continue; 
                }
                c = (char)( ((int)'0') + params->matrix[i][j] );
                append(binary ,c);
            }
        }
        index = binaryToDecimal(binary);
        j_begin+=5;
        j_end+=5;
        params->vector[ params->thread_number * ((params->max_col+1)/5) + steps] = index;
        strcpy(binary, "B");
    }
    freeMatrix(params->matrix, 5);
    return NULL;
}

void matrixSetZero(PGM *data, int top, int low, int max_col){
    int i,j,steps,j_begin,j_end;
    j_begin = 0;
    j_end = 4;
    for (steps = 0; steps < ((max_col+1)/5); steps++){
        for (i = top; i <= low;i++){
            for (j = j_begin; j <= j_end; j++){
                if ( (i == (top+2)) && (j == (j_begin+2))){
                    data->matrix[i][j] = 0;
                    continue; 
                }
            }
        }
        j_begin+=5;
        j_end+=5;
    }
}

int prepareImage(PGM *data){
    execution = 2;
    int i,top = 0,low = 4,max_col,r,c,i2,j2,j=0,j_begin,j_end,steps;
    int thread_number = (data->row/5);
    int *vector;
    if (debug){
        sleep(1);
    }
    pthread_t *thread;
    Params *params;
    thread = malloc(thread_number * sizeof(pthread_t));
    params = malloc(thread_number * sizeof(Params));

    max_col = data->col - (data->col % 5);
    vector = (int *) malloc((thread_number * (max_col/5)) * sizeof(int));
    top = 0; low = 4;
    
    int top2=0,low2=4;
    for (i = 0; i < thread_number;i++){
        params[i].matrix = allocateMatrix(5, data->col);
        params[i].max_col = max_col;
        params[i].vector = vector;
        params[i].top = top;
        params[i].low = low;
        params[i].thread_number = i;
        for (r = top2,i2 = 0; r <= low2; r++,i2++){
            for (c = 0, j2 = 0; c < max_col; c++, j2++){                
                params[i].matrix[i2][j2] = data->matrix[r][c];
            }
        }
        pthread_create (&thread[i], NULL, &matrixHistogram, &params[i]);
        top2 +=5;
        low2 +=5;
    }

    for (i=0;i < thread_number; i++){
        pthread_join(thread[i], NULL);
    }

    percentage += 20;
    
    int step = 0, length_histogram=0;
    int length = thread_number * (max_col/5);
    radixsort(vector, length);
    intmap *histogram = malloc(length * sizeof(intmap)); 
    for (i=0,j=0; i < length ;i = i + step,j++){
        histogram[j].index = vector[i];
        histogram[j].frequence = count(vector,vector[i],length);
        step = histogram[j].frequence;
        length_histogram++;
    }
 
    if (debug){
        sleep(2);
    }

    execution = 3;
    radixSortIntmap(histogram,length_histogram);
    percentage += 20;


    if (debug){
        sleep(2);
    }

    execution = 4;
    top = 0; low = 4;
    for (i = 0; i < thread_number;i++){
        matrixSetZero(data,top,low,max_col);
        top +=5;
        low +=5;
    }

    writePGM("check_binary_center_zero.pgm",data,0);
    FILE* fp;
    char* command;
    command = malloc(200 * sizeof(char));
    strcpy(command,"md5sum check_binary_center_zero.pgm");

    char hex[32];
    char bits[128];
    fp = popen(command,"r");

    fgets(hex, sizeof(hex)+1, fp);
    htoi(hex, bits);

    int lh = length_histogram-1;
    int pattern = histogram[lh].index;
    int pattern_frequence = histogram[lh].frequence;
    top = 0; low = 4;
    int insertion = 0;

    while ((insertion < 128 )&&(low <= (data->row - (data->row % 5))) ){
        
        int i_row,j_col,index,center_i,center_j;
        char binary[50] = "B";
        char c;
        j_begin = 0;
        j_end = 4;
        
        for (steps = 0; steps < ((max_col+1)/5); steps++){
            for (i_row = top; i_row <= low; i_row++){
                for (j_col = j_begin; j_col <= j_end; j_col++){
                    if ( (i_row == (top+2)) && (j_col == (j_begin+2))){
                        center_i = i_row;
                        center_j = j_col;
                        continue; 
                    }
                    c = (char)( ((int)'0') + data->matrix[i_row][j_col]);
                    append(binary ,c);
                }
            }
            index = binaryToDecimal(binary);
            j_begin+=5;
            j_end+=5;
            if (index == pattern){
                data->matrix[center_i][center_j] = bits[insertion] - '0';
                insertion++;
                if (insertion == 128)
                    break;
                pattern_frequence--;
                if (pattern_frequence == 0){
                    lh--;
                    pattern = histogram[lh].index;
                    pattern_frequence = histogram[lh].frequence;
                } 
            }
            strcpy(binary, "B");
        }
        top +=5;
        low +=5;
        if (low >= (data->row - (data->row % 5))){
            top = 0;
            low = 4;
        }
    }
    writePGM("check_signed.pgm",data,1);
    percentage += 10;
    if (debug){
        sleep(1);
    }
    return 1;
}

void binary_hex(int n, char hex[]){
    int i=0,decimal=0, rem;
    while (n!=0)
    {
        decimal += (n%10)*expow(2,i);
        n/=10;
        ++i;
    } 

    i=0;
    while (decimal!=0)
    {
        rem=decimal%16;
        switch(rem)
        {
            case 10:
              hex[i]='A';
              break;
            case 11:
              hex[i]='B';
              break;
            case 12:
              hex[i]='C';
              break;
            case 13:
              hex[i]='D';
              break;
            case 14:
              hex[i]='E';
              break;
            case 15:
              hex[i]='F';
              break;
            default:
              hex[i]=rem+'0';
              break;
        }
        ++i;
        decimal/=16;
    }
    hex[i]='\0';
    strrev(hex);
}   

int hex_binary(char hex[])
{
    int i, length, decimal=0, binary=0;
    for(length=0; hex[length]!='\0'; ++length);
    for(i=0; hex[i]!='\0'; ++i, --length)
    {
        if(hex[i]>='0' && hex[i]<='9')
            decimal+=(hex[i]-'0')*expow(16,length-1);
        if(hex[i]>='A' && hex[i]<='F')
            decimal+=(hex[i]-55)*expow(16,length-1);
        if(hex[i]>='a' && hex[i]<='f')
            decimal+=(hex[i]-87)*expow(16,length-1);
    }

    i=1;
    while (decimal!=0)
    {
        binary+=(decimal%2)*i;
        decimal/=2;
        i*=10;
    }
    return binary;
}

void htoi(const char *ptr, char *binAddr) {
    char value[128] = "";
    char ch = *ptr;
    int i;
    const char* quads[] = {"0000", "0001", "0010", "0011", "0100", "0101",
                         "0110", "0111", "1000", "1001", "1010", "1011",
                         "1100", "1101", "1110", "1111"};

    while (ch == ' ' || ch == '\t')
        ch = *(++ptr);

    for (i = 0; i < 32; i++) {
        if (ch >= '0' && ch <= '9')
            strncat(value, quads[ch - '0'], 4);
        if (ch >= 'A' && ch <= 'F')
            strncat(value, quads[10 + ch - 'A'], 4);
        if (ch >= 'a' && ch <= 'f')
            strncat(value, quads[10 + ch - 'a'], 4);

        ch = *(++ptr);
    }
    sscanf(value, "%s", binAddr);
}

void strrev(char *p)
{
  char *q = p;
  while(q && *q) ++q;
  for(--q; p < q; ++p, --q)
    *p = *p ^ *q,
    *q = *p ^ *q,
    *p = *p ^ *q;
}

int count(int arr[], int x, int n){
    int i,j;     

    i = first(arr, 0, n-1, x, n); 
    if(i == -1)
        return i;    
    j = last(arr, i, n-1, x, n);    
    return j-i+1;
}
 
int first(int arr[], int low, int high, int x, int n){
  if(high >= low)
  {
    int mid = (low + high)/2;
    if( ( mid == 0 || x > arr[mid-1]) && arr[mid] == x)
      return mid;
    else if(x > arr[mid])
      return first(arr, (mid + 1), high, x, n);
    else
      return first(arr, low, (mid -1), x, n);
  }
  return -1;
}

int last(int arr[], int low, int high, int x, int n){
  if(high >= low)
  {
    int mid = (low + high)/2;
    if( ( mid == n-1 || x < arr[mid+1]) && arr[mid] == x )
      return mid;
    else if(x < arr[mid])
      return last(arr, low, (mid -1), x, n);
    else
      return last(arr, (mid + 1), high, x, n);     
  }
  return -1;
}

void radixsort(int vector[], int length) {
    int i;
    int b[length];
    int maior = vector[0];
    int exp = 1;
 
    for (i = 0; i < length; i++) {
        if (vector[i] > maior)
            maior = vector[i];
    }
 
    while (maior/exp > 0) {
        int bucket[10] = { 0 };
        for (i = 0; i < length; i++)
            bucket[(vector[i] / exp) % 10]++; 
        for (i = 1; i < 10; i++)
            bucket[i] += bucket[i - 1];
        for (i = length - 1; i >= 0; i--)
            b[--bucket[(vector[i] / exp) % 10]] = vector[i];
        for (i = 0; i < length; i++)
            vector[i] = b[i];
        exp *= 10;
    }
}

void radixSortIntmap(intmap vector[], int length){
    int i;
    intmap b[length];
    int maior = vector[0].frequence;
    int exp = 1;
 
    for (i = 0; i < length; i++) {
        if (vector[i].frequence > maior)
            maior = vector[i].frequence;
    }
 
    while (maior/exp > 0) {
        int bucket[10] = { 0 };
        for (i = 0; i < length; i++)
            bucket[(vector[i].frequence / exp) % 10]++; 
        for (i = 1; i < 10; i++)
            bucket[i] += bucket[i - 1];
        for (i = length - 1; i >= 0; i--)
            b[--bucket[(vector[i].frequence / exp) % 10]] = vector[i];
        for (i = 0; i < length; i++)
            vector[i] = b[i];
        exp *= 10;
    }
}




