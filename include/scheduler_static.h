#ifndef SCHEDULER_STATIC_H
#define SCHEDULER_STATIC_H

#include "process.h"
#include "scheduler_common.h"


void run_scheduler_static(ProcessQueue* queue, SchedulingAlgorithm algo, int quantum, int tempo_total);
void run_fcfs_static(ProcessQueue* queue, int tempo_total);
void run_sjf_static(ProcessQueue* queue, int tempo_total);
void run_priority_static(ProcessQueue* queue, int preemptive, int tempo_total);
void run_round_robin_static(ProcessQueue* queue, int quantum, int tempo_total);
void run_rm_static(ProcessQueue* queue, int tempo_total);
void run_edf_static(ProcessQueue* queue, int tempo_total);


#endif
