#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

/*
    How-to: give the first 5 arguments to configure processes
    Example: gcc proc.c -lm
             ./a.out 10 0.01 400 10 256

*/
/*
*   Brief Manual - Operations of each function
*   ready queue is managed as an array of ptrs to Process structs
*   1. append_ready_queue()
*       traverse the array of processes
*       if the process->arrival_t is equal to current time
*           add a ptr of the process to ready queue[]
*           change the process->stat to 2(ready)
*           
*        (TODO: "ties" are sorted) !!!but never checked the correctness
*   
*       
*   
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
    int tau;            // estimated burst time
    int sample_t;        // actual burst time
} Proc;


double get_rand(int ub, double lambda)
{
    double r = drand48();
    double x = -log(r) / lambda;

    while (x > ub)
    {
        return get_rand(ub, lambda);
    }

    return x;
}

void gen_procs(Proc *procs, int seed, int procs_num, int ub, double lambda)
{
    srand48(seed);

    // Generate processes
    for (int i = 0; i < procs_num; i++)
    {
        procs[i].id = i + 'A';
        procs[i].stat = 0;
        
        // initial process arrival time
        procs[i].arrival_t = trunc(get_rand(ub, lambda));
        procs[i].arrival_t_static = procs[i].arrival_t;

        procs[i].cpu_b = trunc(drand48() * 100) + 1; // CPU burst range: [1, 100]
        procs[i].cpu_b_static = procs[i].cpu_b;

        //  identify the actual CPU burst time and the I/O burst time
        //  don't generate I/O burst for the last CPU burst
        for (int j = 0; j < procs[i].cpu_b; j++)
        {
            procs[i].cpu_t[j] = ceil(get_rand(ub, lambda));
            if (j == procs[i].cpu_b - 1)
                procs[i].io_t[j] = 0;
            else
                procs[i].io_t[j] = ceil(get_rand(ub, lambda));
        }
        procs[i].tau = 1 / lambda;
        procs[i].sample_t = 0;
    }
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
            queue[ctr++] = ready[i]->id;
            queue[ctr++] = ' ';
        }
    }
    if (ctr == 0)
        queue[ctr] = '\0';
    else
        queue[--ctr] = '\0';
}

void append_io_to_ready_queue (Proc * ready_procs[], Proc * procs, int procs_num, int * ctr_ready, int t)
{
    Proc ready[26];
    int ctr = 0;
    
    for (int i = 0; i < procs_num; i++)
    {
        if (procs[i].arrival_t == t && procs[i].stat == 3)
        {
            ready[ctr] = procs[i];
            ctr++;
        }
    }
    if (ctr > 1)
    {
        qsort(ready, ctr, sizeof(Proc), sort);
    }
    for (int i = 0; i < ctr; i++)
    {
        ready_procs[*ctr_ready] = &procs[(int)ready[i].id - 65];
        ready_procs[*ctr_ready]->stat = 2;
        *ctr_ready += 1;
        char q[60];
        get_Q(ready_procs, procs_num, q);
        printf("time %dms: Process %c completed I/O; added to ready queue [Q %s]\n", t, ready[i].id, q);
    }
}

void append_new_to_ready_queue (Proc * ready_procs[], Proc * procs, int procs_num, int * ctr_ready, int t)
{
    Proc ready[26];
    int ctr = 0;
    
    for (int i = 0; i < procs_num; i++)
    {
        if (procs[i].arrival_t == t && procs[i].stat == 0)
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
        ready_procs[*ctr_ready]->stat = 2;
        *ctr_ready += 1;
        char q[60];
        get_Q(ready_procs, procs_num, q);
        printf("time %dms: Process %c arrived; added to ready queue [Q %s]\n", t, ready[i].id, q);
    }
}

void SJF_sort (Proc * ready[], int ctr_ready)
{
    int i = 0;
    if (ready[0] == NULL)
        return;
    if (ready[0]->stat != 2)
        i = 1;
    Proc * p;
    for (; i < ctr_ready - 1; i++)
    {
        for (int j = 0; j < ctr_ready-i-1; j++)
        {
            if (ready[j]->tau > ready[j+1]->tau)
            {
                // swap
                p = ready[j];
                ready[j] = ready[j+1];
                ready[j+1] = p;
            }
        }
    }
}

void burst_begin (Proc * proc, int t)
{
    if (proc->stat != 5 || proc->cpu_b == 0)
        perror("ERROR: <Invalid Ready Queue.>");
    proc->stat = 4;
    proc->cpu_b -= 1;
    proc->arrival_t = t + proc->cpu_t[0];
    for (int i = 0; i < proc->cpu_b; i++)
    {
        proc->cpu_t[i] = proc->cpu_t[i+1];
    }
}

void rm_running_proc (Proc * ready[], int procs_num, int * ctr_ready)
{
    for (int i = 0; i < procs_num-1; i++)
    {
        if (ready[i] == NULL)
            break;
        ready[i] = ready[i + 1];
    }
    ready[procs_num-1] = NULL;
    *ctr_ready -= 1;
}

int check_proc_completion (Proc * ready[], int procs_num, int * ctr_ready, int t)
{
    int rc = 0;
    if (ready[0]->stat == 6 && ready[0]->arrival_t == t)
    {
        if (ready[0]->cpu_b == 0)
        {
            ready[0]->stat = 1;
            rc = 1;
        }
        else
        {
            rc = 2;
            ready[0]->stat = 3;
            ready[0]->arrival_t = t + ready[0]->io_t[0];
            for (int i = 0; i < ready[0]->cpu_b; i++)
            {
                ready[0]->io_t[i] = ready[0]->io_t[i+1];
            }
        }
    }
    return rc;
}

void cxt_s_in (Proc * proc[], int cs_t, int t)
{
    proc[0]->arrival_t = t + cs_t/2;
    proc[0]->stat = 5;
}

void cxt_s_out (Proc * proc, int cs_t, int t)
{
    proc->arrival_t = t + cs_t/2;
    proc->stat = 6;
}

int check_burst (Proc * proc, int cs_t, int t)
{
    if (proc->stat == 4 && proc->arrival_t == t)
    {   
        cxt_s_out(proc, cs_t, t);
        return 1;
    }
    return 0;
}


// ----------------------------------------------------------------
int main (int argc, char * argv[])
{
    /* Command line arugments:
    * argv[1]: seed for random number generator;
    * argv[2]: lambda, for exponential distribution
    * argv[3]: upper bound for valid pseudo-random numbers
    * argv[4]: n, number processes to simulate
    * argv[5]: t_cs, context switch time
    * argv[6]: alpha, used to estimate CPU burst times for SJF and SRT
    * argv[7]: t_slice, time slice for RR algorithm
    * argv[8]: rr_add, define whether processes are added to the end or beginning of the ready queue
    */

    int seed = strtol(argv[1], NULL, 10);      // (int) argv[1]
    double lambda = strtod(argv[2], NULL);     // (double) argv[2]
    int ub = strtol(argv[3], NULL, 10);        // argv[3]
    int procs_num = strtol(argv[4], NULL, 10); // argv[4]
    int cs_t = strtol(argv[5], NULL, 10);      // argv[5]
    // float alpha = strtof(argv[6], NULL);       // argv[6]


    char *scheduling_algos[4] = {"FCFS", "SJF", "SRT", "RR"};

    for (int k = 0; k < 4; k++)
    {
        Proc procs[procs_num];
        gen_procs(procs, seed, procs_num, ub, lambda);

        // Create instances
        int t = 0;
        int temp;
        Proc * ready[procs_num];
        for (int i = 0; i < procs_num; i++)
            ready[i] = NULL;
        int ctr_ready = 0;                      // number of procs in ready[] array
#if 0
        if (strcmp(scheduling_algos[k], "SJF") == 0)
        {
            for (int procs_ctr = 0; procs_ctr < procs_num; procs_ctr++)
            {
                if (procs[procs_ctr].cpu_b > 1)
                    printf("Process %c [NEW] (arrival time %d ms) %d CPU bursts\n", procs[procs_ctr].id, 
                    procs[procs_ctr].arrival_t, procs[procs_ctr].cpu_b);
                else
                {
                    printf("Process %c [NEW] (arrival time %d ms) %d CPU burst\n", procs[procs_ctr].id, 
                    procs[procs_ctr].arrival_t, procs[procs_ctr].cpu_b);
                }
                
            }
            printf("time %dms: Simulator started for FCFS [Q <empty>]\n", t);

            // Time starts
            while (1)
            {
                // Step 1: Check if all processes complete
                temp = 0;
                for (int i = 0; i < procs_num; i++)
                {
                    if (procs[i].stat == 1)
                        temp++;
                }
                if (temp == procs_num)
                    break;

                // Step 1.5: Check the ready queue (and begin to burst) before appending new ready procs

            }
            
        }
#endif
        if(strcmp(scheduling_algos[k], "FCFS") == 0)
        {
            for (int procs_ctr = 0; procs_ctr < procs_num; procs_ctr++)
            {
                if (procs[procs_ctr].cpu_b > 1)
                    printf("Process %c [NEW] (arrival time %d ms) %d CPU bursts\n", procs[procs_ctr].id, 
                    procs[procs_ctr].arrival_t, procs[procs_ctr].cpu_b);
                else
                {
                    printf("Process %c [NEW] (arrival time %d ms) %d CPU burst\n", procs[procs_ctr].id, 
                    procs[procs_ctr].arrival_t, procs[procs_ctr].cpu_b);
                }
                
            }
            printf("time %dms: Simulator started for FCFS [Q <empty>]\n", t);
            
            // Time starts
            while (1)
            {
                // Step 1: Check if all processes complete
                temp = 0;
                for (int i = 0; i < procs_num; i++)
                {
                    if (procs[i].stat == 1)
                        temp++;
                }
                if (temp == procs_num)
                    break;
                
                // Step 1.5: Check the ready queue (and begin to burst) before appending new ready procs
                if (ready[0] != NULL)
                {
                    if (ready[0]->stat == 2)
                    {
                        cxt_s_in(ready, cs_t, t);
                    }
                    if (ready[0]->stat == 5 && ready[0]->arrival_t == t)
                    {
                        burst_begin(ready[0], t);
                        char q[60];
                        get_Q(ready, procs_num, q);
                        if (strlen(q) == 0)
                            printf("time %dms: Process %c started using the CPU for %dms burst [Q <empty>]\n", t, ready[0]->id, ready[0]->arrival_t - t);
                        else
                            printf("time %dms: Process %c started using the CPU for %dms burst [Q %s]\n", t, ready[0]->id, ready[0]->arrival_t - t, q);
                    }
                }

                // Step 2: Check if CPU burst/context switch completes
                if (ready[0] != NULL)
                {
                    if (check_burst(ready[0], cs_t, t) == 1)
                    {
                        char q[60];
                        get_Q(ready, procs_num, q);
                        if (strlen(q) == 0)
                        {
                            if (ready[0]->cpu_b == 1)
                            {
                                printf("time %dms: Process %c completed a CPU burst; %d burst to go [Q <empty>]\n", t, ready[0]->id, ready[0]->cpu_b);
                                printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q <empty>]\n", t, ready[0]->id, ready[0]->arrival_t + ready[0]->io_t[0]);
                            }                            
                            else if (ready[0]->cpu_b > 1)
                            {
                                printf("time %dms: Process %c completed a CPU burst; %d bursts to go [Q <empty>]\n", t, ready[0]->id, ready[0]->cpu_b);
                                printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q <empty>]\n", t, ready[0]->id, ready[0]->arrival_t + ready[0]->io_t[0]);
                            }
                            else 
                                printf("time %dms: Process %c terminated [Q <empty>]\n", t, ready[0]->id);
                        }
                        else
                        {
                            if (ready[0]->cpu_b == 1)
                            {
                                printf("time %dms: Process %c completed a CPU burst; %d burst to go [Q %s]\n", t, ready[0]->id, ready[0]->cpu_b, q);
                                printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q %s]\n", t, ready[0]->id, ready[0]->arrival_t + ready[0]->io_t[0], q);
                            }
                            else if (ready[0]->cpu_b > 1)
                            {
                                printf("time %dms: Process %c completed a CPU burst; %d bursts to go [Q %s]\n", t, ready[0]->id, ready[0]->cpu_b, q);
                                printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q %s]\n", t, ready[0]->id, ready[0]->arrival_t + ready[0]->io_t[0], q);
                            }
                            else
                                printf("time %dms: Process %c terminated [Q %s]\n", t, ready[0]->id, q);
                        }
                    }
                    int rc = check_proc_completion(ready, procs_num, &ctr_ready, t);
                    if (rc == 1 || rc == 2)
                    {   
                        rm_running_proc(ready, procs_num, &ctr_ready);
                    }
                }
                
                
                // Step 2: Fill in the ready queue
                append_io_to_ready_queue(ready, procs, procs_num, &ctr_ready, t);
                append_new_to_ready_queue(ready, procs, procs_num, &ctr_ready, t);

                // Step 4: Begin to burst/context switch on to CPU
                if (ready[0] != NULL)
                {
                    if (ready[0]->stat == 2)
                    {
                        cxt_s_in(ready, cs_t, t);
                    }
                    if (ready[0]->stat == 5 && ready[0]->arrival_t == t)
                    {
                        burst_begin(ready[0], t);
                        char q[60];
                        get_Q(ready, procs_num, q);
                        if (strlen(q) == 0)
                            printf("time %dms: Process %c started using the CPU for %dms burst [Q <empty>]\n", t, ready[0]->id, ready[0]->arrival_t - t);
                        else
                            printf("time %dms: Process %c started using the CPU for %dms burst [Q %s]\n", t, ready[0]->id, ready[0]->arrival_t - t, q);
                    }
                }
                t++;
            }
            printf("time %dms: Simulator ended for FCFS [Q <empty>]\n", --t);
        }
    }
}
