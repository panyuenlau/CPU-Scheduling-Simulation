#include <unistd.h>
#include "proc.h"

/*
    How-to: give the first 5 arguments to configure processes
    Example: gcc -Wall -Werror -o a.out *.c -lm 
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
    int slice = strtol(argv[7], NULL, 10); // argv[7]
    char rr_add[10];
    strcpy(rr_add, "END");
    if (argc > 8)
        strcpy(rr_add, argv[8]);
    
    // Creating variables for simout
    int fd = open ("simout.txt", O_CREAT | O_WRONLY | O_TRUNC, 0600);
            if (fd < 0)
                perror("open failed");
    char buffer[100];
    float avg_cpu_burst;



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
            if (write(fd, "Algorithm FCFS\n", strlen("Algorithm FCFS\n")) < 0)
                perror("write failed");
            int cpu_burst = 0;
            int cpu_burst_ctr = 0;
            for (int i = 0; i < procs_num; i++)
            {
                for (int j = 0; j < procs[i].cpu_b; j++)
                {
                    cpu_burst += procs[i].cpu_t[j];
                    cpu_burst_ctr++;
                }
            }
            avg_cpu_burst = (float)cpu_burst / cpu_burst_ctr;
            int n = snprintf(buffer, 100,
                            "-- average CPU burst time: %.3f ms\n", avg_cpu_burst);
            buffer[n] = '\0';
            if (write(fd, buffer, strlen(buffer)) < 0)
                perror("write failed");
            FCFS(procs, ready, procs_num, t, cs_t, ctr_ready, fd);
        }
#endif

#if 1
        if (strcmp(scheduling_algos[k], "SJF") == 0)
        {
            if (write(fd, "Algorithm SJF\n", strlen("Algorithm SJF\n")) < 0)
                perror("write failed");
            if (write(fd, buffer, strlen(buffer)) < 0)
                perror("write failed");
            SJF(procs, ready, procs_num, t, cs_t, ctr_ready, fd);
        }
#endif

#if 1
        if (strcmp(scheduling_algos[k], "SRT") == 0)
        {
            if (write(fd, "Algorithm SRT\n", strlen("Algorithm SRT\n")) < 0)
                perror("write failed");
            if (write(fd, buffer, strlen(buffer)) < 0)
                perror("write failed");
            SRT(procs, ready, procs_num, t, cs_t, ctr_ready, fd);
        }
#endif
#if 1
        if (strcmp(scheduling_algos[k], "RR") == 0)
        {
            if (write(fd, "Algorithm RR\n", strlen("Algorithm RR\n")) < 0)
                perror("write failed");
            if (write(fd, buffer, strlen(buffer)) < 0)
                perror("write failed");
            RR(procs, ready, procs_num, t, cs_t, ctr_ready, slice, rr_add, fd);
        }
#endif
    }
    close(fd);
    return EXIT_SUCCESS;
}
