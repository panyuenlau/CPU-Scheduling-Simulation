#include "proc.h"

void FCFS(Proc *procs, Proc **ready, int procs_num, int t, int cs_t, int ctr_ready)
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
        if (check_all_procs(procs, procs_num) == 1) 
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
            int last_tau = check_burst(ready[0], cs_t, t);
            if (last_tau)
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
            int rc = check_proc_completion(ready, procs_num, t);
            if (rc == 1 || rc == 2)
            {   
                rm_running_proc(ready, procs_num, &ctr_ready);
            }
        }
        
        
        // Step 3: Fill in the ready queue
        int start = append_io_to_ready_queue(ready, procs, procs_num, &ctr_ready, t);
        for (int i = start; i < procs_num; i++)
        {
            if (ready[i] == NULL)
                break;
            char q[60];
            get_Q(ready, procs_num, q);
            printf("time %dms: Process %c completed I/O; added to ready queue [Q %s]\n", t, ready[i]->id, q);
        }
        start = append_new_to_ready_queue(ready, procs, procs_num, &ctr_ready, t);
        for (int i = start; i < procs_num; i++)
        {
            if (ready[i] == NULL)
                break;
            char q[60];
            get_Q(ready, procs_num, q);
            printf("time %dms: Process %c arrived; added to ready queue [Q %s]\n", t, ready[i]->id, q);
        }

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
    printf("time %dms: Simulator ended for FCFS [Q <empty>]\n\n", --t);
}