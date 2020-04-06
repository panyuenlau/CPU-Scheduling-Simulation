#include "proc.h"

void RR(Proc *procs, Proc **ready, int procs_num, int t, int cs_t, int ctr_ready, int slice, char *rr_add, int fd, int cpu_burst)
{
    // Optional setting rr_add: append return process to the BEGINNING or END
    bool add = true;
    if (strcmp(rr_add, "BEGINNING") == 0)
        add = false;
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
    printf("time %dms: Simulator started for RR [Q <empty>]\n", t);

    // Time starts
    while (1)
    {
        // Step 1: Check if all processes complete
        if (check_all_procs(procs, procs_num) == 1) 
            break;

        // Step 1.5: Check the ready queue (and begin to burst) before appending new ready procs
        // check_rdy_que(procs,ready, cs_t, procs_num, t, false, ctr_ready, false);
        RR_check_rdy_que(procs, ready, cs_t, procs_num, t, ctr_ready, slice);

        // Step 2: Check if CPU burst/context switch completes
        if (ready[0] != NULL)
        {
            int rc = RR_check_burst(ready, cs_t, t, slice, procs_num, ctr_ready);
            if (rc == 1)
            {
                char q[60];
                get_Q(ready, procs_num, q);
                if (t <= 999)
                {
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
                    }
                }
                
                if (ready[0]->cpu_b == 0)
                {
                    if (strlen(q) == 0)
                        printf("time %dms: Process %c terminated [Q <empty>]\n", t, ready[0]->id);
                    else
                        printf("time %dms: Process %c terminated [Q %s]\n", t, ready[0]->id, q);
                }
            }
            rc = RR_check_proc_completion(ready, procs_num, t, ctr_ready);
            if (rc == 1 || rc == 2)
            {   
                rm_running_proc(ready, procs_num, &ctr_ready);
            }
            else if (rc == 3)
            {
                Proc *tmp = ready[0];
                rm_running_proc(ready, procs_num, &ctr_ready);
                ready[ctr_ready] = tmp;
                ctr_ready ++;
            }
        }
        
        // Step 3: Fill in the ready queue
        int start = append_io_to_ready_queue(ready, procs, procs_num, &ctr_ready, t, add);
        if (add)
        {
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
                char q[60];
                get_Q(temp_ready[j - start], procs_num, q);
                if (t <= 999)
                    printf("time %dms: Process %c completed I/O; added to ready queue [Q %s]\n", t, ready[j]->id, q);
            }
        }
        
        start = append_new_to_ready_queue(ready, procs, procs_num, &ctr_ready, t, add);
        if (add)
        {
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
                char q[60];
                get_Q(temp_ready[j - start], procs_num, q);
                if (t <= 999)
                    printf("time %dms: Process %c arrived; added to ready queue [Q %s]\n", t, ready[j]->id, q);
            }
        }

        // Step 4: Begin to burst/context switch on to CPU
        // check_rdy_que(procs, ready, cs_t, procs_num, t, false, ctr_ready, false);
        RR_check_rdy_que(procs, ready, cs_t, procs_num, t, ctr_ready, slice);

        // printf("time = %d\n", t);
        t++;
    }
    printf("time %dms: Simulator ended for RR [Q <empty>]\n", --t);

    print_stat(procs, procs_num, fd, cs_t, cpu_burst);
}