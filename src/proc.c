#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

/*
    How-to: give the first 4 arguments to configure processes
    Example: gcc proc.c -lm
             ./a.out 10 0.01 400 10

    Construct an array of processes using the structure below
        - First 4 arguments are needed to create processes
        - Random arrival and burst times based on exponential distribution
        TODO: how seed needed to be manipulated
*/
typedef struct
{
    char id;        // name of the proc
    int stat;       // status {0: new, 1: ready, 2: running, 3: blocking, 4:                            completed}
    int cpu_b;      // number of cpu bursts it has
    int arrival_t;  // arrival time
    int cpu_t[100]; // length of each burst
    int io_t[100];  // length of each io
} Proc;

void gen_procs(Proc *procs, char *argv[]);
double *gen_rands(int *seed, int iterations, int ub, double lambda, double *times);

int main(int argc, char *argv[])
{
    /*
    * argv[1]: seed for random number generator;
    * argv[2]: lambda, for exponential distribution
    * argv[3]: upper bound for valid pseudo-random numbers
    * argv[4]: process ids, A--Z
    * argv[5]: t_cs, context switch time
    * argv[5]: n, number processes to simulate
    * argv[6]: alpha, used to estimate CPU burst times for SJF and SRT
    * argv[7]: t_slice, time slice for RR algorithm
    * argv[8]: rr_add, define whether processes are added to the end or begining of the ready queue
    * */
    int procs_num = strtol(argv[4], NULL, 10);
    Proc procs[procs_num];
    gen_procs(procs, argv);
    return EXIT_SUCCESS;
}

void gen_procs(Proc *procs, char *argv[])
{
    int seed = strtol(argv[1], NULL, 10);      // (int) argv[1]
    double lambda = strtod(argv[2], NULL);     //(double) argv[2]
    int ub = strtol(argv[3], NULL, 10);        // argv[3]
    int procs_num = strtol(argv[4], NULL, 10); // argv[4]
    printf("seed = %u\n", seed);
    int iterations = 100000;

    // Generate random variables
    double times[iterations];
    int t_ctr = 0;
    gen_rands(&seed, iterations, ub, lambda, times);

    // Generate processes
    for (int i = 0; i < procs_num; i++)
    {
        procs[i].id = i + 'A';
        srand48(seed);
        procs[i].cpu_b = trunc(drand48() * 100);
        seed += 1;
        procs[i].arrival_t = trunc(times[t_ctr++]);
        for (int j = 0; j < procs[i].cpu_b; j++)
        {
            if (ceil(times[t_ctr]) <= ub)
                procs[i].cpu_t[j] = ceil(times[t_ctr++]);
            else
            {
                procs[i].cpu_t[j] = ub;
                t_ctr++;
            }
            if (ceil(times[t_ctr]) <= ub)
                procs[i].io_t[j] = ceil(times[t_ctr++]);
            else
            {
                procs[i].io_t[j] = ub;
                t_ctr++;
            }
#if 0
    printf("cpu_t = %d\n", procs[i].cpu_t[j]);
    printf("io_t  = %d\n", procs[i].io_t[j]);
#endif
        }
#if 1
        printf("proc[%c] cpu bursts = %d\n", procs[i].id, procs[i].cpu_b);
#endif
    }
}

double *gen_rands(int *seed, int iterations, int ub, double lambda, double *times)
{
    for (int i = 0; i < iterations; i++)
    {
        srand48(*seed);
        double r = drand48();
        double x = -log(r) / lambda;
        if (x > ub)
        {
            x = ub;
        }
        times[i] = x;
        *seed += 1;
    }
    return times;
}