#ifndef PROCESS_H
#define PROCESS_H

typedef enum {
    STATIC,
    DYNAMIC
} GenerationMode;

typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int priority;
    int remaining_time;
    int deadline;     // Para EDF ou Rate Monotonic
    int period;       // Para processos periódicos
} Process;

typedef struct {
    Process* list;
    int size;
    int capacity;
} ProcessQueue;

ProcessQueue* create_process_queue(int capacity);
void destroy_process_queue(ProcessQueue* queue);
void add_process(ProcessQueue* queue, Process proc);
Process generate_random_process(int id, int current_time);
void load_processes_from_file(char* filename, ProcessQueue *queue, int is_periodic);
#endif
