// #include <stdlib.h>
// #include <math.h>
#include <unistd.h>
#include <stdbool.h>
#include "proc.h"

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
    alpha = strtof(argv[6], NULL);       // argv[6]


    char *scheduling_algos[4] = {"FCFS", "SJF", "SRT", "RR"};

    for (int k = 0; k < 4; k++)
    {
        Proc procs[procs_num];
        gen_procs(procs, seed, procs_num, ub, lambda);

        // Create instances
        int t = 0;
        Proc * ready[procs_num];
        for (int i = 0; i < procs_num; i++)
            ready[i] = NULL;
        int ctr_ready = 0;  // number of procs in ready[] array
#if 1
        if(strcmp(scheduling_algos[k], "FCFS") == 0)
        {
            FCFS(procs, ready, procs_num, t, cs_t, ctr_ready);
        }
#endif

#if 1
        if (strcmp(scheduling_algos[k], "SJF") == 0)
        {
            SJF(procs, ready, procs_num, t, cs_t, ctr_ready);
        }
#endif

#if 1
        if (strcmp(scheduling_algos[k], "SRT") == 0)
            SRT(procs, ready, procs_num, t, cs_t, ctr_ready);
#endif
    }

    return EXIT_SUCCESS;
}
