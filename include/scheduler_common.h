#ifndef SCHEDULER_COMMON_H
#define SCHEDULER_COMMON_H

typedef enum {
    FCFS,
    SJF,
    PRIORITY_NON_PREEMPTIVE,
    PRIORITY_PREEMPTIVE,
    ROUND_ROBIN,
    RATE_MONOTONIC,
    EDF
} SchedulingAlgorithm;

#endif
