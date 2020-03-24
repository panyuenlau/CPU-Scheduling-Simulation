#include "proc.h"

/*This file contains the common functions for all scheduling algorithms*/

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

int append_io_to_ready_queue (Proc * ready_procs[], Proc * procs, int procs_num, int * ctr_ready, int t)
{
    Proc ready[26];
    int ctr = 0;
    int idx = *ctr_ready;
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
    }
    return idx;
}

int append_new_to_ready_queue (Proc * ready_procs[], Proc * procs, int procs_num, int * ctr_ready, int t)
{
    Proc ready[26];
    int ctr = 0;
    int idx = *ctr_ready;
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
    }
    return idx;
}


void burst_begin (Proc * proc, int t)
{
    if (proc->stat != 5 || proc->cpu_b == 0)
        perror("ERROR: <Invalid Ready Queue.>");
    proc->stat = 4;
    proc->cpu_b -= 1;
    proc->arrival_t = t + proc->cpu_t[0];
    proc->sample_t = proc->cpu_t[0];
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

int check_proc_completion (Proc * ready[], int procs_num, int t)
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
        int last_tau = proc->tau;
        proc->tau = ceil(alpha * proc->sample_t + (1 - alpha) * proc->tau);
        return last_tau;
    }
    return 0;
}

int check_all_procs(Proc *procs, int procs_num)
{
    /*A helper function to check if all processes are completed
    returns 1 if all processes completed, returns 0 otherwise
    */
    int temp = 0;
    for (int i = 0; i < procs_num; i++)
    {
        if (procs[i].stat == 1)
            temp++;
    }
    
    if (temp == procs_num)
        return 1;
    
    return 0;
}

void check_rdy_que(Proc **ready, int cs_t, int procs_num, int t)
{
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
                printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst [Q <empty>]\n", t, ready[0]->id, ready[0]->tau, ready[0]->arrival_t - t);
            else
                printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst [Q %s]\n", t, ready[0]->id, ready[0]->tau, ready[0]->arrival_t - t, q);
        }
    }
}

void check_cpub_context(Proc **ready, int cs_t, int procs_num, int t, int *ctr_ready)
{
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
                    printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d burst to go [Q <empty>]\n", t, ready[0]->id, last_tau, ready[0]->cpu_b);
                    printf("time %dms: Recalculated tau = %dms for process %c [Q <empty>]\n", t, ready[0]->tau, ready[0]->id);
                    printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q <empty>]\n", t, ready[0]->id, ready[0]->arrival_t + ready[0]->io_t[0]);
                }                            
                else if (ready[0]->cpu_b > 1)
                {
                    printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d bursts to go [Q <empty>]\n", t, ready[0]->id, last_tau, ready[0]->cpu_b);
                    printf("time %dms: Recalculated tau = %dms for process %c [Q <empty>]\n", t, ready[0]->tau, ready[0]->id);
                    printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q <empty>]\n", t, ready[0]->id, ready[0]->arrival_t + ready[0]->io_t[0]);
                }
                else 
                    printf("time %dms: Process %c terminated [Q <empty>]\n", t, ready[0]->id);
            }
            else
            {
                if (ready[0]->cpu_b == 1)
                {
                    printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d burst to go [Q %s]\n", t, ready[0]->id, last_tau, ready[0]->cpu_b, q);
                    printf("time %dms: Recalculated tau = %dms for process %c [Q %s]\n", t, ready[0]->tau, ready[0]->id, q);
                    printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q %s]\n", t, ready[0]->id, ready[0]->arrival_t + ready[0]->io_t[0], q);
                }
                else if (ready[0]->cpu_b > 1)
                {
                    printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d bursts to go [Q %s]\n", t, ready[0]->id, last_tau, ready[0]->cpu_b, q);
                    printf("time %dms: Recalculated tau = %dms for process %c [Q %s]\n", t, ready[0]->tau, ready[0]->id, q);
                    printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q %s]\n", t, ready[0]->id, ready[0]->arrival_t + ready[0]->io_t[0], q);
                }
                else
                    printf("time %dms: Process %c terminated [Q %s]\n", t, ready[0]->id, q);
            }
        }
        int rc = check_proc_completion(ready, procs_num, t);
        if (rc == 1 || rc == 2)
        {   
            rm_running_proc(ready, procs_num, ctr_ready);
        }
    }
}


void burst_context(Proc **ready, int cs_t, int procs_num, int t)
{
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
                printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst [Q <empty>]\n", t, ready[0]->id, ready[0]->tau, ready[0]->arrival_t - t);
            else
                printf("time %dms: Process %c (tau %dms)started using the CPU for %dms burst [Q %s]\n", t, ready[0]->id, ready[0]->tau, ready[0]->arrival_t - t, q);
        }
    }
}