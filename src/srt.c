#include "proc.h"


void SRT(Proc *procs, Proc **ready, int procs_num, int t, int cs_t, int ctr_ready)
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
    printf("time %dms: Simulator started for SRT [Q <empty>]\n", t);
    while(1)
    {
        bool prem = false;
        // Step 1: Check if all processes complete
        if (check_all_procs(procs, procs_num) == 1)
            break;

        /*Update remaining tau and CPU burst time*/
        update_remain_t(procs, procs_num);
        
        // Step 1.5: Check the ready queue (and begin to burst) before appending new ready procs
        // check_rdy_que(procs, ready, cs_t, procs_num, t, true, ctr_ready, prem);
        check_rdy_que(procs, ready, cs_t, procs_num, t, true, ctr_ready);

        // Step 2: Check if CPU burst/context switch completes; update estimated burst time
        check_cpub_context(ready, cs_t, procs_num, t, &ctr_ready);

        // Step 3: Fill in the ready queue       
        int start = append_io_to_ready_queue(ready, procs, procs_num, &ctr_ready, t, false);

        Proc * temp_ready[26][26];
        int temp = 0;
        while(start + temp < ctr_ready)
        {
            int i;
            for (i = 0; i <= start + temp; i++)
            {
                temp_ready[temp][i] = ready[i];
            }
            temp_ready[temp][i] = NULL;
            temp ++;
        }

        for (int j = start; j < ctr_ready; j++)
        {
            printf("time %dms: Process %c (tau %dms) completed I/O; ", t, ready[j]->id, ready[j]->tau);
            
            prem = check_preem_from_io(procs, procs_num, ready, j, t, ctr_ready, cs_t);
            sort_queue (temp_ready[j - start], j + 1, true);
            char q[60];
            get_Q(temp_ready[j-start], procs_num, q);
            if(prem)
                printf("preempting %c [Q %s]\n", ready[0]->id, q);
            else
                printf("added to ready queue [Q %s]\n", q);
        }
        sort_queue(ready, ctr_ready, true);

        prem = false;
        start = append_new_to_ready_queue(ready, procs, procs_num, &ctr_ready, t, false);
        temp = 0;

        while(start + temp < ctr_ready)
        {
            int i;
            for ( i = 0; i <= start + temp; i++)
            {
                temp_ready[temp][i] = ready[i];
            }
            temp_ready[temp][i] = NULL;
            temp ++;
        }
        for (int j = start; j < ctr_ready; j++)
        {
            printf("time %dms: Process %c (tau %dms) arrived; ", t, ready[j]->id, ready[j]->tau);
            
            prem = check_preem_from_io(procs, procs_num, ready, j, t, ctr_ready, cs_t);

            sort_queue (temp_ready[j - start], j + 1, true);
            char q[60];
            get_Q(temp_ready[j-start], procs_num, q);            
            if(prem)
                printf("preempting %c [Q %s]\n", ready[0]->id, q);
            else
                printf("added to ready queue [Q %s]\n", q);
        }
        sort_queue (ready, ctr_ready, true);
        
        // Step 4: Begin to burst/context switch on to CPU
        check_rdy_que(procs, ready, cs_t, procs_num, t, true, ctr_ready);

        t++;
    }
    printf("time %dms: Simulator ended for SRT [Q <empty>]\n", --t);
}