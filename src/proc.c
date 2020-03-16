#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>

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
    int stat;           // status {0: new, 1: completed, 2: ready, 3: blocking, 4: running, 5: context switch on to CPU, 6: context switch out of CPU}
    int cpu_b;          // number of cpu bursts it has
    int arrival_t;      // arrival time
    int cpu_t[100];     // length of each burst
    int io_t[100];      // length of each io
    int cpu_b_static;   // reserved for final calculation
    int arrival_t_static; // reserved for final calculation
    int burst_t;        // keep track of burst time
    int wait_t;         // keep track of wait time
} Proc;

double * gen_rands (int * seed, int iterations, int ub, double lambda, double * times);
void cxt_s_in (Proc * proc[], int cs_t, int procs_num, int t);
void cxt_s_out (Proc * proc, int cs_t);


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
        procs[i].cpu_b_static = procs[i].cpu_b;
        seed += 1;
        procs[i].arrival_t = trunc(times[t_ctr++]);
        procs[i].arrival_t_static = procs[i].arrival_t;
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

void get_Q (Proc * ready[], int procs_num, char * queue)
{
    int ctr = 0;
    for (int i = 0; i < procs_num; i++)
    {
        if (ready[i] == NULL)
            break;
        if (ready[i]->stat == 2)
        {
            queue[ctr] = ready[i]->id;
            ctr++;
        }
    }
    queue[ctr] = '\0';
}

void append_ready_queue (Proc * ready_procs[], Proc * procs, int procs_num, int * ctr_ready, int t)
{
    Proc ready[26];
    int ctr = 0;
    
    for (int i = 0; i < procs_num; i++)
    {
        if (procs[i].arrival_t == t && procs[i].stat < 4)
        {
            ready[ctr] = procs[i];
            ctr++;
        }
    }
    if (ctr > 1)
        qsort(ready, ctr, sizeof(Proc), sort);
    for (int i = 0; i < ctr; i++)
    {
        ready_procs[*ctr_ready] = &procs[(int)ready[i].id - 65];
        bool io = false;
        if (ready_procs[*ctr_ready]->stat == 3)
            io = true;
        ready_procs[*ctr_ready]->stat = 2;

#if 1
    char q[procs_num+1];
    get_Q(ready_procs, procs_num, q);
    printf("time <%d>ms: <Process[%c]: arrival> [Q <%s>]\n", t, ready[i].id, q);
    if (io == true)
        printf("time <%d>ms: <Process[%c]: finishes performing I/O> [Q <%s>]\n", t, ready[i].id, q);
#endif
        *ctr_ready += 1;
    }
}

void FCFS_SJF_burst_begin (Proc * proc, int t)
{
    if (proc->stat != 5 || proc->cpu_b == 0)
        perror("ERROR: <Invalid Ready Queue.>");
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
    if (ready[0]->stat == 6 && ready[0]->arrival_t == t)
    {
        if (ready[0]->cpu_b == 0)
        {
            ready[0]->stat = 1;
#if 1
    char q[procs_num+1];
    get_Q(ready, procs_num, q);
    printf("time <%d>ms: <Process[%c]: terminates by finishing its last CPU busrt> [Q <%s>]\n", t, ready[0]->id, q);
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
    char q[procs_num+1];
    get_Q(ready, procs_num, q);
    printf("time <%d>ms: <Process[%c]: finishes using the CPU> [Q <%s>]\n", t, ready[0]->id, q);
    printf("time <%d>ms: <Process[%c]: starts performing I/O> [Q <%s>]\n", t, ready[0]->id, q);
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

void cxt_s_in (Proc * proc[], int cs_t, int procs_num, int t)
{
    proc[0]->arrival_t += cs_t/2;
    proc[0]->stat = 5;
#if 1
    char q[procs_num+1];
    get_Q(proc, procs_num, q);
    printf("time <%d>ms: <Process[%c]: starts using the CPU> [Q <%s>]\n", t, proc[0]->id, q);
#endif
}

void cxt_s_out (Proc * proc, int cs_t)
{
    proc->arrival_t += cs_t/2;
    proc->stat = 6;
}

void check_burst (Proc * proc, int cs_t, int t)
{
    if (proc->stat == 4 && proc->arrival_t == t)
    {
        cxt_s_out(proc, cs_t);
    }
}


// ----------------------------------------------------------------
int main (int argc, char * argv[])
{
    int procs_num = strtol(argv[4], NULL, 10);
    int cs_t = strtol(argv[5], NULL, 10);
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
    printf("Start of simulation\n");
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
       
        // Step 3: Check if CPU burst/context switch completes
        if (ready[0] != NULL)
        {
            check_burst(ready[0], cs_t, t);
            check_proc_completion(ready, procs_num, &ctr_ready, t);
        }

        // Step 4: Begin to burst/context switch on to CPU
        if (ready[0] != NULL)
        {
            if (ready[0]->stat == 2)
                cxt_s_in(ready, cs_t, procs_num, t);
            if (ready[0]->stat == 5)
                FCFS_SJF_burst_begin(ready[0], t);
        }

        t++;
    }
#if 1
    printf("End of simulation\n");
#endif
}

