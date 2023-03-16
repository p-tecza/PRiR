#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h> 

double **A;
double **B;
double **C;
pthread_mutex_t lock;
float global_sum=0.0;
float frobenius_norm=0.0;

struct helpfulInfo{
    int m;
    int n;
    int start_index;
    int end_index;
    int iterator;
}

print_matrix(double**A, int m, int n)
{
    int i, j;
    printf("[");
    for(i =0; i< m; i++)
    {
        for(j=0; j<n; j++)
        {
            printf("%f ", A[i][j]);
        }
        printf("\n");
    }
    printf("]\n");
}

void *test_func(void* hi){

    int m = ((struct helpfulInfo *)hi)->m;
    int n = ((struct helpfulInfo *)hi)->n;
    int my_start_index = ((struct helpfulInfo *)hi)->start_index;
    int my_end_index = ((struct helpfulInfo *)hi)->end_index;
    int it = ((struct helpfulInfo *)hi)->iterator;
    float local_sum=0;
    for(int i = my_start_index; i<my_end_index;i++){
        int m_result_index = i / n;
        int n_result_index = i % n;
        
        for(int x = 0; x<it; x++){
            local_sum = local_sum + A[m_result_index][x]*B[x][n_result_index];
        }
        pthread_mutex_lock(&lock);
        global_sum = global_sum + local_sum;
        frobenius_norm = frobenius_norm + local_sum*local_sum;
        pthread_mutex_unlock(&lock);
    
    C[m_result_index][n_result_index]=local_sum;
    printf("Suma w komorce: [%d][%d] -> %f\n",m_result_index,n_result_index,local_sum);
    local_sum=0.0;

    }
}

int main(int argc, char* argv[])
{

    printf("Wywolanie:\n./program [ile_watkow] [plik z mat A] [plik z mat B]\n");
    printf("Domyslnie pliki: A.txt B.txt\n");

    FILE *fpa;
    FILE *fpb;
    int ma, mb, na, nb;
    int i, j;
    double x;

    int N = 2;
    char *fileA = "A.txt";
    char *fileB = "B.txt";

    if(argc>3){
        N=atoi(argv[1]);
        fileA = argv[2];
        fileB = argv[3];
    }
    else if(argc>1){
        N = atoi(argv[1]);
        return -1;
    }
    else{
        printf("Nie podano argumentow wywolania programu. Przyjmuje wartosci domyslne.\n");
    }

    printf("-> Ilosc uzytych watkow: [%d]\n-> Plik mat A: [%s]\n-> Plik mat B: [%s]\n",N,fileA, fileB);

    fpa = fopen(fileA, "r");
    fpb = fopen(fileB, "r");
    if( fpa == NULL || fpb == NULL )
    {
        perror("błąd otwarcia pliku");
        exit(-10);
    }

    fscanf (fpa, "%d", &ma);
    fscanf (fpa, "%d", &na);


    fscanf (fpb, "%d", &mb);
    fscanf (fpb, "%d", &nb);


    //LICZENIE INDEXOW ORAZ SPRAWDZENIE OPTYMALNEJ ILOSCI WATKOW
    int trivial_mul_amount = ma*nb;

    if(trivial_mul_amount < N){
        printf("Zbyt duzo watkow do rownoleglych obliczen!\n");
        printf("Ustawiam na rownowartosc ilosci sum czastkowych: [%d]\n",trivial_mul_amount);;
        N=trivial_mul_amount;
    }

    int values_per_thread = trivial_mul_amount / N;
    int remainder = trivial_mul_amount % N;

    int *indexes = malloc(sizeof(int)*(N+1));
    indexes[0]=0;

    for(int i=1;i<=N;i++){
        indexes[i]=values_per_thread*i;
    }

    for (int i=N;remainder>0;i--){
        for(int j=i; j<=N;j++){
            indexes[j]++;
        }
        remainder--;
    }

    printf("pierwsza macierz ma wymiar %d x %d, a druga %d x %d\n", ma, na, mb, nb);

    if(na != mb)
    {
        printf("Złe wymiary macierzy!\n");
        return EXIT_FAILURE;
    }
    
    /*Alokacja pamięci*/
    A = malloc(ma*sizeof(double));
    for(i=0; i< ma; i++)
    {
        A[i] = malloc(na*sizeof(double));
    }

    B = malloc(mb*sizeof(double));
    for(i=0; i< mb; i++)
    {
        B[i] = malloc(nb*sizeof(double));
    }

    /*Macierz na wynik*/
    C = malloc(ma*sizeof(double));
    for(i=0; i< ma; i++)
    {
        C[i] = malloc(nb*sizeof(double));
    }

    printf("Rozmiar C: %dx%d\n", ma, nb);
    for(i =0; i< ma; i++)
    {
        for(j = 0; j<na; j++)
        {
            fscanf( fpa, "%lf", &x );
            A[i][j] = x;
        }
    }

    printf("A:\n");
    print_matrix(A, ma, mb);

    for(i =0; i< mb; i++)
    {
        for(j = 0; j<nb; j++)
        {
            fscanf( fpb, "%lf", &x );
            B[i][j] = x;
        }
    }

    printf("B:\n");
    print_matrix(B, mb, nb);

    // TWORZENIE WĄTKÓW
    pthread_t *threads = malloc(sizeof(pthread_t)*N);
    struct helpfulInfo hi[N];

    for(int i=0;i<N;i++){
        hi[i].m=ma;
        hi[i].n=nb;
        hi[i].iterator=na;
        hi[i].start_index=indexes[i];
        hi[i].end_index=indexes[i+1];
        pthread_create(&(threads[i]),NULL,test_func,(void*)&(hi[i]));
    }

    for(int i=0;i<N;i++){
        pthread_join(threads[i],NULL);
    }

    printf("C:\n");
    print_matrix(C,ma,nb);

    printf("Suma całkowita elementow macierzy C (policzona przez wątki): %f\n",global_sum);
    printf("Norma Frobeniusa macierzy C: %f\n",sqrt(frobenius_norm));

    for(i=0; i<na; i++)
    {
        free(A[i]);
    }
    free(A);

    for(i=0; i<nb; i++)
    {
        free(B[i]);
    }
    free(B);

    for(i=0; i<nb; i++)
    {
        free(C[i]);
    }
    free(C);

   
    fclose(fpa);
    fclose(fpb);


    return 0;
}