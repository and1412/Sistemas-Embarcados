#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#define LO(num) ((num) & 0x000000FF)


typedef struct _Params {
	int **matrix;
	int top;
	int low;
	int max_col;
	int thread_number;
	int *vector;
} Params;

typedef struct _PGM {
    int row;
    int col;
    int max_gray;
    int **matrix;
} PGM;

typedef struct _intmap {
	int index;
	int frequence;
} intmap;

int expow(int number,int exponential);
int **allocateMatrix(int row, int col);
void freeMatrix(int **matrix, int row);
PGM* readPGM(char *filename, PGM *data);
int prepareImage(PGM *data);
void *matrixHistogram(void *parameters);
void radixsort(int vector[], int length);
void radixSortIntmap(intmap vector[], int length);
int binaryToDecimal(char *string);
void append(char *string, char c);
int count(int arr[], int x, int n);
int first(int arr[], int low, int high, int x, int n);
int last(int arr[], int low, int high, int x, int n);
void writePGM(const char *filename, const PGM *data, int flag);
void matrixSetZero(PGM *data, int top, int low, int max_col);
void htoi(const char *ptr, char *binAddr);

int main(int argc, char *argv[]){

	char *filename;
	int result;
	PGM *pgm = malloc(1 * sizeof(PGM));

	if(argc != 2){
		fprintf(stderr, "\033[22;31mOnly one argument is allowed: %s <FILENAME>\n\033[0m", argv[0]);
		exit(-1);
	}
	
	filename = argv[1];
	pgm = readPGM(filename, pgm);

	result = prepareImage(pgm);
 	
 	if (result){
 		printf("[SUCCESS] Image wasn't modified!\n");
 	}
 	else
 		fprintf(stderr,"ERROR - Image was modified.");


	return 0;
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

void freeMatrix(int **matrix, int row){
	int i;
    for (i = 0; i < row; ++i)
        free(matrix[i]);
    free(matrix);
}

PGM* readPGM(char *filename, PGM *data){
	FILE *pgmImage;
	char *imageLength;
	char version[3];
    int i, j, low;

    pgmImage = fopen(filename, "rb");
    fgets(version, sizeof(version), pgmImage);
    fscanf(pgmImage, "%d", &data->col);
    fscanf(pgmImage, "%d", &data->row);
    fscanf(pgmImage, "%d", &data->max_gray);
    fgetc(pgmImage);	
    data->matrix = allocateMatrix(data->row, data->col);

    for (i = 0; i < data->row; ++i){
        for (j = 0; j < data->col; ++j) {
            low = fgetc(pgmImage);
            data->matrix[i][j] = low;
        }
 	}

    fclose(pgmImage);
    return data;
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

int prepareImage(PGM *data){
    
    int i,top = 0,low = 4,max_col,r,c,i2,j2,top2=0,low2=4;
    int thread_number = (data->row/5);
    int *vector;

    pthread_t *thread;
    Params *params;
    thread = malloc(thread_number * sizeof(pthread_t));
    params = malloc(thread_number * sizeof(Params));

    max_col = data->col - (data->col % 5);
    vector = (int *) malloc((thread_number * (max_col/5)) * sizeof(int));
    for (i = 0; i < thread_number;i++){
        params[i].matrix = allocateMatrix(5, data->col);
        params[i].max_col = max_col;
        params[i].vector = vector;
        params[i].top = top;
        params[i].low = low;
        params[i].thread_number = i;
        for (r = top2,i2=0; r <= low2;r++,i2++){
            for (c = 0,j2=0; c < max_col; c++,j2++){
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
    
    int step = 0, length_histogram=0,j=0;
    int length = thread_number * (max_col/5);
    radixsort(vector, length);
    intmap *histogram = malloc(length * sizeof(intmap)); 
    for (i=0,j=0; i < length ;i = i + step,j++){
        histogram[j].index = vector[i];
        histogram[j].frequence = count(vector,vector[i],length);
        step = histogram[j].frequence;
        length_histogram++;
    }
 
    radixSortIntmap(histogram,length_histogram);

    //printf("Padrão %d com frequência %d \n",histogram[length_histogram-1].index,histogram[length_histogram-1].frequence);
    char bits[129] = "";
    int lh = length_histogram-1;
    int pattern = histogram[lh].index;
    int pattern_frequence = histogram[lh].frequence;
    top = 0; low = 4;
    int insertion = 0;


    while ((insertion < 128 )&&(low <= (data->row - (data->row % 5))) ){
        
        int i_row,j_col,index,steps,j_begin,j_end,center_i,center_j;
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
            	//printf("[%d][%d] = %d\n",center_i,center_j,data->matrix[center_i][center_j]);
                bits[insertion] = (char)( ((int)'0') + data->matrix[center_i][center_j]);
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

    //------------------------

    top = 0; low = 4;
    for (i = 0; i < thread_number;i++){
        matrixSetZero(data,top,low,max_col);
        top +=5;
        low +=5;
    }

    writePGM("check_binary_center_zero2.pgm",data,0);
    FILE* fp;
    char* command;
    command = malloc(200 * sizeof(char));
    strcpy(command,"md5sum check_binary_center_zero2.pgm");
    char hex[33];
    char bits2[129];
    fp = popen(command,"r");
    fgets(hex, sizeof(hex)+1, fp);
    //printf("binary image %s \n",hex);
    htoi(hex, bits2);

    printf("%s\n",bits);
    printf("%s\n",bits2 );

    if (!strcmp( bits, bits2))
    	return 1;
    else
    	return 0;
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

int count(int arr[], int x, int n){
    int i,j;     

    i = first(arr, 0, n-1, x, n); 
    if(i == -1)
        return i;    
    j = last(arr, i, n-1, x, n);    
    return j-i+1;
}

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

void writePGM(const char *filename, const PGM *data, int flag){
	FILE *pgmImage;
    int i, j;
    int low;
 
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
    fclose(pgmImage);
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
                    break; 
                }
            }
        }
        j_begin+=5;
        j_end+=5;
    }
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
        //printf("%s\n", value);
    }
    sscanf(value, "%s", binAddr);
    //*binAddr = *value;
}