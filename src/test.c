#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

typedef struct
{
    int cpu_b;
    int arrival_t;
    int cpu_t[100];
    int io_t[100];
} Proc;

double * gen_rands (int * seed, int iterations, int ub, double lambda, double * times);

int main (int argc, char * argv[])
{
    int seed = strtol(argv[1], NULL, 10); // (int) argv[1]
    double lambda = 0.01; //(double) argv[2]
    int ub = 300; // (int) argv[3]
    int procs_num = 10; // (int) argv[4]    
    printf("seed = %u\n", seed);
    int iterations = 100000;
    
    // Generate random variables
    double times[iterations];
    int t_ctr = 0;
    gen_rands(&seed, iterations, ub, lambda, times);

    // Generate processes
    Proc procs[procs_num];
    for (int i = 0; i < procs_num; i++)
    {
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
        
        printf("proc[%d] cpu bursts = %d\n", i, procs[i].cpu_b);
    }
}

double * gen_rands (int * seed, int iterations, int ub, double lambda, double * times)
{
   for (int i = 0; i < iterations; i++)
   {
       srand48(*seed);
       double r = drand48();
       double x = - log(r) / lambda; 
       if (x > ub) { x = ub; }
       times[i] = x;
       *seed += 1;
   } 
   return times;
}

