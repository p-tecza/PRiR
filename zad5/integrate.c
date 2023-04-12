#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

double myfunction(double x)
{
    return x / 2 * (x + 5);
}

double integrate(double (*func)(double), double begin, double end, int num_points)
{

    double step = (end - begin) / (double)num_points;
    double sum = 0.0;
    double first_val = func(begin);
    double second_val;

    for (int i = 1; i <= num_points; i++)
    {
        second_val = func(begin + step * i);
        sum += (first_val + second_val) / 2 * step;
        first_val = second_val;
    }

    return sum;
}

int main(int argc, char **argv)
{
    MPI_Init(NULL, NULL);
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (argc != 4)
    {
        printf("Podano nieprawidłowe parametry. Poprawny format: [begin <double>] [end <double>] [n_points <int>]\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    const int N_POINTS = atoi(argv[3]);
    const double a = atof(argv[1]);
    const double b = atof(argv[2]);

    int calc_part_steps = N_POINTS / world_size;
    int remainder = N_POINTS - world_size * calc_part_steps;
    double *start_end_vector = malloc(sizeof(double) * 2);

    if (world_rank == 0)
    {

        double step_size = (b - a) / world_size;
        double offset = 0.0;

        for (int i = 1; i < world_size; i++)
        {

            if(remainder>0){
                start_end_vector[0] = offset + a + i * step_size;
                offset+=(b-a)/(double)N_POINTS;
                start_end_vector[1] = offset + a + (i + 1) * step_size;
                remainder--;
                int fixed_points_val = calc_part_steps + 1;
                MPI_Send(&fixed_points_val, 1, MPI_INT, i, i + 200, MPI_COMM_WORLD);
            }else{
                start_end_vector[0] = offset + a + i * step_size;
                start_end_vector[1] = a + (i + 1) * step_size;
                MPI_Send(&calc_part_steps, 1, MPI_INT, i, i + 200, MPI_COMM_WORLD);
            }
            MPI_Send(start_end_vector, 2, MPI_DOUBLE, i, i + 100, MPI_COMM_WORLD);
        }

        printf("[%d] Ile krokow: %d\n",world_rank, calc_part_steps);
        printf("[%d] Moj przedzial: %f - %f\n", world_rank, a, a + step_size);

        double main_process_integral_part = integrate(myfunction, a, a + step_size, calc_part_steps);

        for (int i = 1; i < world_size;)
        {
            double subprocess_sum;
            MPI_Recv(&subprocess_sum, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            main_process_integral_part += subprocess_sum;
            i++;
        }
        printf("-------------------\n");
        printf("Wynik całki: %f\n", main_process_integral_part);
        printf("-------------------\n");

        if(world_size*2>N_POINTS){
            printf("W zwiazku z małą ilością punktów próbkowania, niektóre procesy otrzymały mniej danych\n"
            "niż jest potrzebne do obliczenia całki metodą trapezów. Może to generować spory błąd. Proszę zmniejszyć liczbę procesów,\n"
            "bądź zwiększyć ilość punktów.\n");
        }
    }
    else
    {

        MPI_Recv(&calc_part_steps, 1, MPI_INT, 0, world_rank + 200, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);     
        
        MPI_Recv(start_end_vector, 2, MPI_DOUBLE, 0, world_rank + 100, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        printf("[%d] Ile krokow: %d\n",world_rank, calc_part_steps);
        printf("[%d] Moj przedzial: %f - %f\n", world_rank, start_end_vector[0], start_end_vector[1]);

        double part_integral = integrate(myfunction, start_end_vector[0], start_end_vector[1], calc_part_steps);

        MPI_Send(&part_integral, 1, MPI_DOUBLE, 0, 1337, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}