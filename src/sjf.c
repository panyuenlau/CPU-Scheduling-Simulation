#include "proc.h"

void SJF(Proc *procs, Proc **ready, int procs_num, int t, int cs_t, int ctr_ready)
{
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
    printf("time %dms: Simulator started for SJF [Q <empty>]\n", t);

    // Time starts
    while (1)
    {
        // Step 1: Check if all processes complete
        if (check_all_procs(procs, procs_num) == 1) 
            break;

        // Step 1.5: Check the ready queue (and begin to burst) before appending new ready procs
        check_rdy_que(procs,ready, cs_t, procs_num, t, false, ctr_ready);

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
                    sort_queue(ready, ctr_ready, false);
                    char q[60];
                    get_Q(ready, procs_num, q, false);
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
                    sort_queue (ready, ctr_ready, false);
                    char q[60];
                    get_Q(ready, procs_num, q, false);
                    printf("added to ready queue [Q %s]\n", q);
                    break;
                }
            }
        }
        
        // Step 4: Begin to burst/context switch on to CPU
        check_rdy_que(procs, ready, cs_t, procs_num, t, false, ctr_ready);
        t++;
    }
    printf("time %dms: Simulator ended for SJF [Q <empty>]\n", --t);
}