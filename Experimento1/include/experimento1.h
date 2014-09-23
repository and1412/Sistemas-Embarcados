#ifndef EXPERIMENTO1_H
#define EXPERIMENTO1_H
#define LO(num) ((num) & 0x000000FF)
#define THRESHOLD 0.5

int execution;
int percentage;
int debug;

typedef struct _PGM {
    int row;
    int col;
    int max_gray;
    int **matrix;
    float **matrixErr;
} PGM;

typedef struct _intmap {
	int index;
	int frequence;
} intmap;

typedef struct _Params {
	int **matrix;
	int top;
	int low;
	int max_col;
	int thread_number;
	int *vector;
} Params;


/*functions*/

int **allocateMatrix(int row, int col);
float **allocateMatrixErr(int row, int col);
void freeMatrix(int **matrix, int row);
PGM* readPGM(char *filename, PGM *data);
void writePGM(const char *filename, const PGM *data, int flag);
char *prepareString(char *s);
void propagateError(PGM *data, int row, int col, float error);
int prepareImage(PGM *data);
void *matrixHistogram(void *parameters);
int binaryToDecimal(char *string);
void append(char *string, char c);
void strrev(char *p);
void binary_hex(int n, char hex[]);
int count(int arr[], int x, int n);
int first(int arr[], int low, int high, int x, int n);
int last(int arr[], int low, int high, int x, int n);
void radixsort(int vector[], int length);
void radixSortIntmap(intmap vector[], int length);
void matrixSetZero(PGM *data, int top, int low, int max_col);
void htoi(const char *ptr, char *binAddr);
int expow(int number,int exponential);

#endif
