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
    char id;            // name of the proc
    int stat;           // status {0: new, 1: completed, 2: ready, 3: blocking, 4: running}
    int cpu_b;          // number of cpu bursts it has
    int arrival_t;      // arrival time
    int cpu_t[100];     // length of each burst
    int io_t[100];      // length of each io
} Proc;

double * gen_rands (int * seed, int iterations, int ub, double lambda, double * times);

int gen_procs (Proc * procs, char * argv[])
{
    int seed = strtol(argv[1], NULL, 10); // (int) argv[1]
    double lambda = strtod(argv[2], NULL); //(double) argv[2]
    int ub = strtol(argv[3], NULL, 10); // argv[3]
    int procs_num = strtol(argv[4], NULL, 10);  // argv[4]  
#if 0
    printf("seed = %u\n", seed);
#endif
    int iterations = 10000;
    
    // Generate random variables
    double times[iterations];
    int t_ctr = 0;
    gen_rands(&seed, iterations, ub, lambda, times);

    // Generate processes
    for (int i = 0; i < procs_num; i++)
    {
        procs[i].id = i + 'A';
        procs[i].stat = 0;
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
#if 1
    printf("cpu_t = %d\n", procs[i].cpu_t[j]);
    printf("io_t  = %d\n", procs[i].io_t[j]);
#endif
        }
#if 1
    printf("proc[%c] cpu bursts = %d\n", procs[i].id, procs[i].cpu_b);
    printf("proc[%c] arrival time = %d\n", procs[i].id, procs[i].arrival_t);

#endif
    }
    return 0;
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

// Policy for "Ties"
int sort (const void * a, const void * b)
{
    Proc * p1 = (Proc *)a;
    Proc * p2 = (Proc *)b;
    if (p1->stat > p2->stat)
        return -1;
    else if (p1->stat < p2->stat)
        return 1;
    else
        return (p1->id > p2->id) - (p1->id < p2->id);
}

void append_ready_queue (Proc * ready_procs[], Proc * procs, int procs_num, int * ctr_ready, int t)
{
    Proc ready[26];
    int ctr = 0;
    
    for (int i = 0; i < procs_num; i++)
    {
        if (procs[i].arrival_t == t && procs[i].stat != 4)
        {
            ready[ctr] = procs[i];
            ctr++;
        }
    }
    if (ctr > 1)
        qsort(ready, ctr, sizeof(Proc), sort);
    for (int i = 0; i < ctr; i++)
    {
#if 1
    printf("At time %d, append ready queue[%d] = %c\n", t, *ctr_ready, ready[i].id);
#endif
        ready_procs[*ctr_ready] = &procs[(int)ready[i].id - 65];
        ready_procs[*ctr_ready]->stat = 2;
        *ctr_ready += 1;
    }
}

void FCFS_SJF_burst_begin (Proc * proc, int t)
{
    if (proc->stat != 2 || proc->cpu_b == 0)
        perror("ERROR: <Invalid Ready Queue.>");
#if 1
    printf("At time %d, Process[%c] begin to burst\n", t, proc->id);
#endif
    proc->stat = 4;
    proc->cpu_b -= 1;
    proc->arrival_t = t + proc->cpu_t[0];
    for (int i = 0; i < proc->cpu_b - 1; i++)
    {
        proc->cpu_t[i] = proc->cpu_t[i+1];
    }
}

void check_proc_completion (Proc * ready[], int procs_num, int * ctr_ready, int t)
{
    if (ready[0]->stat == 4 && ready[0]->arrival_t == t)
    {
        if (ready[0]->cpu_b == 0)
        {
            ready[0]->stat = 1;
#if 1
    printf("At time %d, Process[%c] completes all bursts\n", t, ready[0]->id);
#endif
        }
        else
        {
            ready[0]->stat = 3;
            ready[0]->arrival_t = t + ready[0]->io_t[0];
            for (int i = 0; i < ready[0]->cpu_b - 1; i++)
            {
                ready[0]->io_t[i] = ready[0]->io_t[i+1];
            }
#if 1
    printf("At time %d, Process[%c] finishes a burst\n", t, ready[0]->id);
#endif
        }
        
        for (int i = 0; i < procs_num-1; i++)
        {
            if (ready[i] == NULL)
                break;
            ready[i] = ready[i + 1];
        }
        ready[procs_num-1] = NULL;
        *ctr_ready -= 1;
    }
}


int main (int argc, char * argv[])
{
    int procs_num = strtol(argv[4], NULL, 10);
    Proc procs[procs_num];
    gen_procs(procs, argv);
    // Create instances
    int t = 0;
    int temp;
    Proc * ready[procs_num];
    for (int i = 0; i < procs_num; i++)
        ready[i] = NULL;
    int ctr_ready = 0;

#if 1
    printf("Time starts\n");
#endif

    // Time starts
    while (1)    
    {
        // Step 1: Check if all processes complete
        // TODO: Check synchronously as each process done
        temp = 0;
        for (int i = 0; i < procs_num; i++)
        {
            if (procs[i].stat == 1)
                temp++;
        }
        if (temp == procs_num)
            break;
        
        // Step 2: Fill in the ready queue
        append_ready_queue(ready, procs, procs_num, &ctr_ready, t);
        
        // Step 3: Check if burst completes
        if (ready[0] != NULL)
            check_proc_completion (ready, procs_num, &ctr_ready, t);

        // Step 4: Begin to burst
        if (ready[0] != NULL && ready[0]->stat != 4)
            FCFS_SJF_burst_begin(ready[0], t);

        t++;
    }

}

