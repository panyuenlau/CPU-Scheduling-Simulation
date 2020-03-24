#ifndef PROC_H
#define PROC_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

typedef struct
{
    char id;            // name of the proc
    int stat;           /* status {0: new, 1: completed, 2: ready, 3: blocking, 4: running, 
                         5: context switch on to CPU, 6: context switch out of CPU} */
    int cpu_b;          // number of cpu bursts it has
    int arrival_t;      // arrival time
    int cpu_t[100];     // length of each burst
    int io_t[100];      // length of each io
    int cpu_b_static;   // reserved for final calculation
    int arrival_t_static; // reserved for final calculation
    int burst_t;        // keep track of burst time
    int wait_t;         // keep track of wait time
    int tau;            // estimated burst time
    int sample_t;
} Proc;

float alpha; // defined as a global variable for ease

void FCFS(Proc *procs, Proc **ready, int procs_num, int t, int cs_t, int ctr_ready);
void SJF(Proc *procs, Proc **ready, int procs_num, int t, int cs_t, int ctr_ready);
void SRT(Proc *procs, Proc **ready, int procs_num, int t, int cs_t, int ctr_ready);


/*Some helper functions*/
double get_rand(int ub, double lambda);
void gen_procs(Proc *procs, int seed, int procs_num, int ub, double lambda);

int sort (const void * a, const void * b);

void get_Q (Proc * ready[], int procs_num, char * queue);

int append_io_to_ready_queue (Proc * ready_procs[], Proc * procs, int procs_num, int * ctr_ready, int t);

int append_new_to_ready_queue (Proc * ready_procs[], Proc * procs, int procs_num, int * ctr_ready, int t);

void SJF_sort (Proc * ready[], int ctr_ready);
void SRT_sort (Proc * ready[], int ctr_ready);


void burst_begin (Proc * proc, int t);
int check_burst (Proc * proc, int cs_t, int t);

void rm_running_proc (Proc * ready[], int procs_num, int * ctr_ready);

int check_proc_completion (Proc * ready[], int procs_num, int t);

void cxt_s_in (Proc * proc[], int cs_t, int t);
void cxt_s_out (Proc * proc, int cs_t, int t);


/*A helper function to check if all processes are completed*/
int check_all_procs(Proc *procs, int procs_num);


/*Check the ready queue (and begin to burst) before appending new ready procs*/
void check_rdy_que(Proc **ready, int cs_t, int procs_num, int t);

void check_cpub_context(Proc **ready, int cs_t, int procs_num, int t, int *ctr_ready);

void burst_context(Proc **ready, int cs_t, int procs_num, int t);

#endif