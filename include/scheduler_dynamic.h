#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "scheduler_common.h"
#include "process.h"

// Funções para os algoritmos de escalonamento - modo dinâmico
void run_fcfs(ProcessQueue* queue);
void run_sjf(ProcessQueue* queue);
void run_priority(ProcessQueue* queue, int preemptive);
void run_round_robin(ProcessQueue* queue, int quantum);
void run_rm(ProcessQueue* queue);
void run_edf(ProcessQueue* queue);

void run_scheduler(ProcessQueue* queue, SchedulingAlgorithm algo, int quantum);

SchedulingAlgorithm parse_algo(const char* str);

#endif
