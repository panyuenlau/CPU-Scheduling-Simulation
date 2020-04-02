#include "proc.h"

/*This file contains the common functions for all scheduling algorithms*/

/*A helper function to generate the next random number*/
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

/*Generate all the processes that need to be scheduled to burst with given parameters*/
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
        procs[i].remain_tau = procs[i].tau;
        procs[i].sample_t = 0;

        procs[i].remain_sample_t = 0;
        procs[i].original_burst_t = -1; // if the original_burst_t stays at -1, meaning no preemption occured to the running process
        procs[i].preempt = false;
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

void sort_queue (Proc * ready[], int ctr_ready, bool srt_sort)
{
    if (ready[0] == NULL || ready[1] == NULL)
        return;
    else if (ready[0]->stat != 2 && ready[2] == NULL)
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
            int tau1, tau2;
            if (srt_sort)
            {
                tau1 = ready[j]->remain_tau;
                tau2 = ready[j+1]->remain_tau;
            }
            else
            {
                tau1 = ready[j]->tau;
                tau2 = ready[j+1]->tau;
            }
            
            if (tau1 > tau2)
            {
                // swap
                p = ready[j];
                ready[j] = ready[j+1];
                ready[j+1] = p;
            }
            else if (tau1 == tau2 && ready[j]->id > ready[j+1]->id)
            {
                // swap
                p = ready[j];
                ready[j] = ready[j+1];
                ready[j+1] = p;
            }
        }
    }
}

/*Append a process that finishes IO burst back to the ready queue*/
int append_io_to_ready_queue (Proc * ready_procs[], Proc * procs, int procs_num, int * ctr_ready, int t, bool add)
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
    if (add)
    {
        for (int i = 0; i < ctr; i++)
        {
            ready_procs[*ctr_ready] = &procs[(int)ready[i].id - 65];
            ready_procs[*ctr_ready]->stat = 2;
            *ctr_ready += 1;
        }
        return idx;
    }
    else 
    {
        if (ctr > 0)
        {
            int i = 0;
            // if (ready_procs[0] != NULL && ready_procs[0]->stat == 2)
            //     i = 0;
            // else if (ready_procs[0] == NULL)
            //     i = 0;
            
            if (*ctr_ready != 0)
            {
                if (ready_procs[0]->stat != 2)
                    i = 1;
                for (int j = (*ctr_ready - 1); j >= i; j--)
                {
                    ready_procs[j + ctr] = ready_procs[j];
                }
            }
            int c = 0;
            for (int j = i; j < i + ctr; j++)
            {
                ready_procs[j] = &procs[(int)ready[c++].id - 65];
                ready_procs[j]->stat = 2;
                Proc* temp_ready[26];
                int k = 0;
                for (; k <= j; k++)
                    temp_ready[k] = ready_procs[k];
                for (int m = ctr + i; m < *ctr_ready + ctr; m++)
                {
                    temp_ready[k] = ready_procs[m];
                    k++;
                }
                temp_ready[k] = NULL;
                char q[60];
                get_Q(temp_ready, procs_num, q);
                printf("time %dms: Process %c completed I/O; added to ready queue [Q %s]\n", t, ready_procs[j]->id, q);
            }
            *ctr_ready += ctr;
            // print_info(ready_procs[0], procs_num, *ctr_ready);
        }
    }
    return idx;
}

/*Append a new process to the ready queue, while applying the sorting rules*/
int append_new_to_ready_queue (Proc * ready_procs[], Proc * procs, int procs_num, int * ctr_ready, int t, bool add)
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
    if (add)
    {
        for (int i = 0; i < ctr; i++)
        {
            ready_procs[*ctr_ready] = &procs[(int)ready[i].id - 65];
            ready_procs[*ctr_ready]->stat = 2;
            *ctr_ready += 1;
        }
        return idx;
    }
    else if (ctr > 0)
    {
        int i = 0;
        // if (ready_procs[0] != NULL && ready_procs[0]->stat == 2)
        //     i = 0;
        // else if (ready_procs[0] == NULL)
        //     i = 0;
        // printf("add and ctr_ready = %d\n", *ctr_ready);
        if (*ctr_ready != 0)
        {
            if (ready_procs[0]->stat != 2)
                i = 1;
            for (int j = (*ctr_ready - 1); j >= i; j--)
            {
                ready_procs[j + ctr] = ready_procs[j];
            }
        }
        int c = 0;
        for (int j = i; j < i + ctr; j++)
        {
            ready_procs[j] = &procs[(int)ready[c++].id - 65];
            ready_procs[j]->stat = 2;
            Proc* temp_ready[26];
            int k = 0;
            for (; k <= j; k++)
                temp_ready[k] = ready_procs[k];
            for (int m = ctr + i; m < *ctr_ready + ctr; m++)
            {
                temp_ready[k] = ready_procs[m];
                k++;
            }
            temp_ready[k] = NULL;
            char q[60];
            get_Q(temp_ready, procs_num, q);
            printf("time %dms: Process %c arrived; added to ready queue [Q %s]\n", t, ready_procs[j]->id, q);
        }
        *ctr_ready += ctr;
        // print_info(ready_procs[0], procs_num, *ctr_ready);
    }
    return idx;
}

/*Remove a running process from the ready queue*/
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

/*returns: 1 - completed; 2 - blocking; 0 - others*/
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

int RR_check_proc_completion (Proc * ready[], int procs_num, int t, int ctr_ready)
{
    int rc = 0;
    if (ready[0]->stat == 6 && ready[0]->arrival_t == t)
    {
        if (ready[0]->cpu_b == 0 && ready[0]->remain_sample_t == 0)
        {
            ready[0]->stat = 1;
            rc = 1;
        }
        else
        {
            if (ready[0]->remain_sample_t == 0)
            {
                ready[0]->stat = 3;
                ready[0]->arrival_t = t + ready[0]->io_t[0];
                for (int i = 0; i < ready[0]->cpu_b; i++)
                {
                    ready[0]->io_t[i] = ready[0]->io_t[i+1];
                }
                rc = 2;
            }
            else
            {
                ready[0]->stat = 2;
                rc = 3;
            }
        }
    }
    return rc;
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

void burst_begin (Proc *proc, int t)
{
    if (proc->stat != 5 || proc->cpu_b == 0)
        perror("ERROR: <Invalid Ready Queue.>");
    proc->stat = 4;
    proc->cpu_b -= 1;
    proc->arrival_t = t + proc->cpu_t[0];
    proc->sample_t = proc->cpu_t[0]; // sample_t: current running cpu burst time
    proc->remain_sample_t = proc->sample_t; // when a process just starts, remanining time == full cpu burst time

    for (int i = 0; i < proc->cpu_b; i++)
    {
        proc->cpu_t[i] = proc->cpu_t[i+1];
    }
}

bool RR_burst_begin (Proc *proc, int t, int slice)
{
    if ((proc->stat != 5 && proc->stat != 4) || (proc->cpu_b == 0 && proc->remain_sample_t == 0))
    {
        perror("ERROR: <Invalid Ready Queue.>");
    }
    proc->stat = 4;
    if (proc->remain_sample_t == 0)
    {
        proc->sample_t = proc->cpu_t[0]; // sample_t: current running cpu burst time
        proc->cpu_b -= 1;
        proc->remain_sample_t = proc->sample_t;
        if (proc->cpu_t[0] < slice)
        {
            proc->arrival_t = t + proc->cpu_t[0];
            proc->remain_sample_t = 0;
        }
        else
        {
            proc->arrival_t = t + slice;
            proc->remain_sample_t -= slice;
        }
        for (int i = 0; i < proc->cpu_b; i++)
        {
            proc->cpu_t[i] = proc->cpu_t[i+1];
        }
        return false;
    }
    else
    {
        if (proc->remain_sample_t < slice)
        {
            proc->arrival_t = t + proc->remain_sample_t;
            proc->remain_sample_t = 0;
        }
        else
        {
            proc->arrival_t = t + slice;
            proc->remain_sample_t -= slice;
        }
        return true;
    }
}

/*Update remaining tau and CPU burst time*/
void update_remain_t (Proc * procs, int proc_num)
{
    for (int i = 0; i < proc_num; i++)
    {
        if (procs[i].stat == 4)
        {
            procs[i].remain_sample_t -= 1;
            procs[i].remain_tau -= 1;            
        }
    }
}

void print_info(Proc *procs, int procs_num, int ctr_ready)
{
    printf("\n=========================Printing information============================\n");
    for (int i = 0; i < procs_num; i++)
    {
        // printf("Process %c -- original_burst_t: %d, sample_t: %d, remain_sample_t: %d, stat: %d, arrival_t: %d, tau: %d, remain_tau: %d, preempt? %d;\n", 
        // procs[i].id, procs[i].original_burst_t, procs[i].sample_t, procs[i].remain_sample_t, procs[i].stat, procs[i].arrival_t, procs[i].tau, procs[i].remain_tau, procs[i].preempt);
        printf("Process %c -- status: %d, arrival_t: %d, stat: %d, original_burst_t: %d, sample_t: %d, remain_sample_t: %d\n", 
        procs[i].id, procs[i].stat, procs[i].arrival_t, procs[i].stat, procs[i].original_burst_t, procs[i].sample_t, procs[i].remain_sample_t);
        printf("cpu_t: ");
        for(int j = 0; j < procs[i].cpu_b; j++)
        {
            printf("%d ", procs[i].cpu_t[j]);
        }
        printf("\n");

        printf("io_t: ");
        for(int j = 0; j < procs[i].cpu_b; j++)
        {
            printf("%d ", procs[i].io_t[j]);
        }
        printf("\n");
    }
    printf("\n");
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

int update_est_burst (Proc * proc, int cs_t, int t)
{
    if (proc->stat == 4 && proc->arrival_t == t)
    {   
        cxt_s_out(proc, cs_t, t);
        int last_tau = proc->tau;
        if (proc->original_burst_t != -1)
            proc->tau = ceil(alpha * proc->original_burst_t + (1 - alpha) * proc->tau);
        else
            proc->tau = ceil(alpha * proc->sample_t + (1 - alpha) * proc->tau);

        proc->remain_tau = proc->tau;
        proc->preempt = false;
        proc->original_burst_t = -1;
        return last_tau;
    }
    return 0;
}

int RR_check_burst (Proc * ready[], int cs_t, int t, int slice, int procs_num, int ctr_ready)
{
    if (ready[0]->stat == 4 && ready[0]->arrival_t == t)
    {   
        if (procs_num > 1 && ready[1] != NULL)
        {
            if (ready[0]->remain_sample_t != 0)
            {
                char q[60];
                get_Q(ready, procs_num, q);
                printf("time %dms: Time slice expired; process %c preempted with %dms to go [Q %s]\n", 
                t, ready[0]->id, ready[0]->remain_sample_t, q);
                cxt_s_out(ready[0], cs_t, t);
                return 0;
            }
            // print_info(ready[0], procs_num, ctr_ready);
            cxt_s_out(ready[0], cs_t, t);
            return 1;
        }
        else
        {
            if (ready[0]->remain_sample_t == 0)
            {
                cxt_s_out(ready[0], cs_t, t);
                return 1;
            }
            else
            {
                RR_burst_begin(ready[0], t, slice);
                printf("time %dms: Time slice expired; no preemption because ready queue is empty [Q <empty>]\n", t);
                return 0;   
            }
        }
    }
    return 0;
}

bool check_preem (Proc *procs, Proc **ready, char q[], int procs_num, int t, int cs_t, int ctr_ready)
{
    if (ready[0] != NULL && ready[1] != NULL)
    {
        if (ready[0]->remain_tau > ready[1]->remain_tau)
        {
            printf("time %dms: Process %c (tau %dms) will preempt %c [Q %s]\n", t, ready[1]->id, ready[1]->tau, ready[0]->id, q);

            // ready[0]: process that needs to be preempted
            // ready[1]: process that will preempt ready[0]

            /* 
            keep track of the original burst in order to correctly calculate next tau
                -if this cpu burst was never preempted before, the original burst is the sample burst
                -if this cpu burst was preempted before, original burst remains the same
            */
            if (!ready[0]->preempt)
            {
                ready[0]->original_burst_t = ready[0]->sample_t;
                ready[0]->preempt = true;
            }

            // change stat of the process that will be preempted to 7
            // In cs_t/2: 
            //  1. change stat to 2
            //  2. context switch in for ready[1]
            //  3. swap ready[0] and ready[1]

            ready[0]->stat = 7; 
            ready[0]->arrival_t = t + cs_t / 2;

            // add the remaining cpu burst back to the cpu_t list to the preempted process
            for (int j = ready[0]->cpu_b + 1; j > 0; j--)
            {
                ready[0]->cpu_t[j] = ready[0]->cpu_t[j-1];
            }
            ready[0]->cpu_t[0] = ready[0]->remain_sample_t;
            ready[0]->cpu_b += 1;
        }
    }
    return true;
}

bool check_preem_from_io (Proc *procs, int procs_num, Proc **ready, int completed_i, int t, int ctr_ready, int cs_t)
{
    for (int i = 0; i < procs_num; i++)
    {   
        if (procs[i].stat == 4)
        {
            if (procs[i].remain_tau > ready[completed_i]->remain_tau)
            {
                /*
                    procs[i]: process that is being preempted
                    ready[completed_i]: process that is preempting procs[i]
                */

                // context switch ready[completed_i] in cs_t/2
                ready[completed_i]->stat = 2;
                ready[completed_i]->arrival_t = t + cs_t / 2;

                /*Add the remaining burst time of the current burst back to the cpu_t*/
                for (int j = procs[i].cpu_b + 1; j > 0; j--)
                {
                    procs[i].cpu_t[j] = procs[i].cpu_t[j-1];
                }
                procs[i].cpu_t[0] = procs[i].remain_sample_t;
                procs[i].cpu_b += 1;
                
                // context switch out, but need to be put back to ready queue in cs_t/2
                procs[i].stat = 8;
                procs[i].arrival_t = t + cs_t/2;


                if (!procs[i].preempt)
                {
                    procs[i].original_burst_t = procs[i].sample_t;
                    procs[i].preempt = true;
                }
                
                return true;
            }
        }
    }
    return false;
}

/*Check the ready queue (and begin to burst) before appending new ready procs*/
// void check_rdy_que(Proc *procs,Proc **ready, int cs_t, int procs_num, int t,  bool srt_flag, int ctr_ready, bool prem)
void check_rdy_que(Proc *procs,Proc **ready, int cs_t, int procs_num, int t,  bool srt_flag, int ctr_ready)
{
    if (ready[0] != NULL)
    {
        if (ready[0]->stat == 2)
        {
            cxt_s_in(ready, cs_t, t);
        }

        if(ready[0]->stat == 7 && ready[0]->arrival_t == t)
        {
            ready[0]->stat = 2;

            ready[1]->stat = 5;
            ready[1]->arrival_t = t + 2;
            
            Proc *temp = ready[0];
            ready[0] = ready[1];
            ready[1] = temp;

            sort_queue (ready, ctr_ready, true);
        }

        if (ready[0]->stat == 8 && ready[0]->arrival_t == t)
        {
            ready[0]->stat = 2;
        }
        
        if (ready[0]->stat == 5 && ready[0]->arrival_t == t)
        {
            burst_begin(ready[0], t);

            char q[60];
            get_Q(ready, procs_num, q);

            if (srt_flag)
            {
                if (strlen(q) == 0)
                    printf("time %dms: Process %c (tau %dms) started using the CPU with %dms burst remaining [Q <empty>]\n", 
                    t, ready[0]->id, ready[0]->tau, ready[0]->remain_sample_t);
                else
                {
                    printf("time %dms: Process %c (tau %dms) started using the CPU with %dms burst remaining [Q %s]\n", 
                    t, ready[0]->id, ready[0]->tau, ready[0]->remain_sample_t, q);
                    /*Check preemption when the process starts to burst*/
                    check_preem(procs, ready, q, procs_num, t, cs_t, ctr_ready);
                }
            }
            else
            {
                if (strlen(q) == 0)
                    printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst [Q <empty>]\n", 
                    t, ready[0]->id, ready[0]->tau, ready[0]->arrival_t - t);
                else
                    printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst [Q %s]\n",
                    t, ready[0]->id, ready[0]->tau, ready[0]->arrival_t - t, q);
            }
        }
    }
}

void RR_check_rdy_que(Proc *procs,Proc **ready, int cs_t, int procs_num, int t, int ctr_ready, int slice)
{
    if (ready[0] != NULL)
    {
        if (ready[0]->stat == 2)
        {
            cxt_s_in(ready, cs_t, t);
        }
        
        if (ready[0]->stat == 5 && ready[0]->arrival_t == t)
        {
            if (RR_burst_begin(ready[0], t, slice))
            {
                char q[60];
                get_Q(ready, procs_num, q);
                if (strlen(q) == 0)
                    printf("time %dms: Process %c started using the CPU with %dms burst remaining [Q <empty>]\n", 
                    t, ready[0]->id, ready[0]->remain_sample_t + ready[0]->arrival_t - t);
                else
                    printf("time %dms: Process %c started using the CPU with %dms burst remaining [Q %s]\n",
                    t, ready[0]->id, ready[0]->remain_sample_t + ready[0]->arrival_t - t, q);
            }
            else
            {
                char q[60];
                get_Q(ready, procs_num, q);
                if (strlen(q) == 0)
                    printf("time %dms: Process %c started using the CPU for %dms burst [Q <empty>]\n", 
                    t, ready[0]->id, ready[0]->remain_sample_t + ready[0]->arrival_t - t);
                else
                    printf("time %dms: Process %c started using the CPU for %dms burst [Q %s]\n",
                    t, ready[0]->id, ready[0]->remain_sample_t + ready[0]->arrival_t - t, q); 
            }
        }
    }
}

/*Check if CPU burst/context switch completes; update estimated burst time*/
void check_cpub_context(Proc **ready, int cs_t, int procs_num, int t, int *ctr_ready)
{
    if (ready[0] != NULL)
    {
        // printf("process %c: original burst time is: %d, sample_t: %d\n", ready[0]->id, ready[0]->original_burst_t, ready[0]->sample_t);
        int last_tau = update_est_burst(ready[0], cs_t, t);
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
