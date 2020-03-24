#include "proc.h"

/*TODO: need to modify the soring rule for SRT algorithm*/
void SRT_sort (Proc * ready[], int ctr_ready)
{
    if (ready[0] == NULL || ready[1] == NULL)
        return;
    

    int k = 1;
    int m = 0;
    if (ready[0]->stat != 2)
    {
        m = 1;
        k = 2;
    }
    Proc * p;
    // Bubble sort
    for (int i = 0; i < ctr_ready - k; i++)
    {
        for (int j = m; j < ctr_ready-i-1; j++)
        {
            if (ready[j]->tau > ready[j+1]->tau)
            {
                // swap
                p = ready[j];
                ready[j] = ready[j+1];
                ready[j+1] = p;
            }
            else if (ready[j]->tau == ready[j+1]->tau && ready[j]->id > ready[j+1]->id)
            {
                // swap
                p = ready[j];
                ready[j] = ready[j+1];
                ready[j+1] = p;
            }
        }
    }
}

void SRT(Proc *procs, Proc **ready, int procs_num, int t, int cs_t, int ctr_ready)
{
    for (int i = 0; i < procs_num; i++)
    {
        printf("Current process is: %c\n", procs[i].id);
        for(int j = 0; j < procs[i].cpu_b; j++)
        {
            printf("%d ", procs[i].cpu_t[j]);
        }
    }
    printf("\n");

    for (int procs_ctr = 0; procs_ctr < procs_num; procs_ctr++)
    {
        if (procs[procs_ctr].cpu_b > 1)
            printf("Process %c [NEW] (arrival time %d ms) %d CPU bursts (tau %dms)\n", procs[procs_ctr].id, 
            procs[procs_ctr].arrival_t, procs[procs_ctr].cpu_b, procs[procs_ctr].tau);
        else
        {
            printf("Process %c [NEW] (arrival time %d ms) %d CPU burst (tau %dms)\n", procs[procs_ctr].id, 
            procs[procs_ctr].arrival_t, procs[procs_ctr].cpu_b, procs[procs_ctr].tau);
        }
    }
    printf("time %dms: Simulator started for SRT [Q <empty>]\n", t);

    while(1)
    {
        // Step 1: Check if all processes complete
        if (check_all_procs(procs, procs_num) == 1)
            break;
        
        // Step 1.5: Check the ready queue (and begin to burst) before appending new ready procs
        check_rdy_que(ready, cs_t, procs_num, t);
    
        // Step 2: Check if CPU burst/context switch completes
        check_cpub_context(ready, cs_t, procs_num, t, &ctr_ready);

        // Step 3: Fill in the ready queue       
        int start = append_io_to_ready_queue(ready, procs, procs_num, &ctr_ready, t);
        char id_l[26];
        int temp = 0;
        while(start + temp < ctr_ready)
        {
            id_l[temp] = ready[start + temp]->id;
            temp ++;
        }
        for (int i = 0; i < temp; i++)
        {
            for (int j = 0; j < ctr_ready; j++)
            {
                if (id_l[i] == ready[j]->id)
                {
                    printf("time %dms: Process %c (tau %dms) completed I/O; ", t, ready[j]->id, ready[j]->tau);
                    SRT_sort(ready, ctr_ready);
                    char q[60];
                    get_Q(ready, procs_num, q);
                    printf("added to ready queue [Q %s]\n", q);
                    break;
                }
            }
        }
    
        start = append_new_to_ready_queue(ready, procs, procs_num, &ctr_ready, t);
        temp = 0;
        while(start + temp < ctr_ready)
        {
            id_l[temp] = ready[start + temp]->id;
            temp ++;
        }
        for (int i = 0; i < temp; i++)
        {
            for (int j = 0; j < ctr_ready; j++)
            {
                if (id_l[i] == ready[j]->id)
                {
                    printf("time %dms: Process %c (tau %dms) arrived; ", t, ready[j]->id, ready[j]->tau);
                    SRT_sort (ready, ctr_ready);
                    char q[60];
                    get_Q(ready, procs_num, q);
                    printf("added to ready queue [Q %s]\n", q);
                    break;
                }
            }
        }
    
        // Step 4: Begin to burst/context switch on to CPU
        burst_context(ready, cs_t, procs_num, t);

        t++;
    }
    printf("time %dms: Simulator ended for SRT [Q <empty>]\n\n", --t);
}