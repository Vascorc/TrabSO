#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "scheduler_dynamic.h"
#include "scheduler_static.h"
#include <limits.h>
#include <unistd.h> // Para usar sleep
#define MAX_INSTANCES 5


int compare_by_burst(const void* a, const void* b) {
    return ((Process*)a)->burst_time - ((Process*)b)->burst_time;
}

int compare_by_priority(const void* a, const void* b) {
    return ((Process*)a)->priority - ((Process*)b)->priority;
}

int compare_by_arrivall(const void* a, const void* b) {
    return ((Process*)a)->arrival_time - ((Process*)b)->arrival_time;
}

int compare_by_deadline(const void* a, const void* b) {
    return ((Process*)a)->deadline - ((Process*)b)->deadline;
}

// FCFS correto (já existia)
void run_fcfs(ProcessQueue* queue) {
    qsort(queue->list, queue->size, sizeof(Process), compare_by_arrivall);

    int current_time = 0;
    int total_wait = 0, total_turnaround = 0, total_burst = 0;  // Adicionar total_burst

    printf("\n[FCFS] Escalonamento:\n");
    for (int i = 0; i < queue->size; i++) {
        Process p = queue->list[i];
        if (current_time < p.arrival_time)
            current_time = p.arrival_time;

        int wait_time = current_time - p.arrival_time;
        int turnaround = wait_time + p.burst_time;
        current_time += p.burst_time;
        total_burst += p.burst_time;  // Soma o tempo de execução do processo

        printf("Processo %d: chegada = %d, Espera = %d, Turnaround = %d\n",
               p.id, p.arrival_time, wait_time, turnaround);

        total_wait += wait_time;
        total_turnaround += turnaround;
    }

    float avg_wait = (float)total_wait / queue->size;
    float avg_turnaround = (float)total_turnaround / queue->size;
    float throughput = (float)queue->size / current_time;
    float cpu_utilization = (float)total_burst / current_time * 100; 

    printf("Média de espera: %.2f\n", avg_wait);
    printf("Média de turnaround: %.2f\n", avg_turnaround);
    printf("Throughput: %.2f processos/unidade de tempo\n", throughput);
    printf("Utilização da CPU: %.2f%%\n", cpu_utilization);
}


// SJF real
void run_sjf(ProcessQueue* queue) {
    int current_time = 0, completed = 0;
    int wait_time = 0, turnaround = 0, total_burst = 0;
    int* done = calloc(queue->size, sizeof(int));

    printf("\n[SJF] Escalonamento:\n");

    while (completed < queue->size) {
        int idx = -1;
        int min_burst = __INT_MAX__;
        for (int i = 0; i < queue->size; i++) {
            Process* p = &queue->list[i];
            if (!done[i] && p->arrival_time <= current_time && p->burst_time < min_burst) {
                min_burst = p->burst_time;
                idx = i;
            }
        }

        if (idx == -1) {
            current_time++;
            continue;
        }

        Process* p = &queue->list[idx];
        int wait = current_time - p->arrival_time;
        int turn = wait + p->burst_time;
        current_time += p->burst_time;

        printf("Processo %d: Espera = %d, Turnaround = %d\n", p->id, wait, turn);
        wait_time += wait;
        turnaround += turn;
        total_burst += p->burst_time;
        done[idx] = 1;
        completed++;
    }

    float avg_wait = (float)wait_time / queue->size;
    float avg_turnaround = (float)turnaround / queue->size;
    float throughput = (float)queue->size / current_time;
    float cpu_utilization = (float)total_burst / current_time * 100;

    printf("Média de espera: %.2f\n", avg_wait);
    printf("Média de turnaround: %.2f\n", avg_turnaround);
    printf("Throughput: %.2f processos/unidade de tempo\n", throughput);
    printf("Utilização da CPU: %.2f%%\n", cpu_utilization);
    free(done);
}

// Priority real (com/sem preempção)
void run_priority(ProcessQueue* queue, int preemptive) {
    int current_time = 0, completed = 0;
    int wait_time = 0, turnaround = 0, total_burst = 0;
    int* remaining = malloc(sizeof(int) * queue->size);
    int* done = calloc(queue->size, sizeof(int));

    for (int i = 0; i < queue->size; i++)
        remaining[i] = queue->list[i].burst_time;

    printf("\n[PRIORITY %s] Escalonamento:\n", preemptive ? "Preemptivo" : "Não-Preemptivo");

    while (completed < queue->size) {
        // ======== AGING =========
        for (int i = 0; i < queue->size; i++) {
            Process* p = &queue->list[i];
            if (!done[i] && p->arrival_time <= current_time && remaining[i] > 0) {
                int waiting_time = current_time - p->arrival_time;
                if (waiting_time > 10) { // ajustável
                    p->priority--; // aumenta prioridade
                    if (p->priority < 0) p->priority = 0;
                }
            }
        }
        // ========================

        int idx = -1;
        int best_prio = __INT_MAX__;
        for (int i = 0; i < queue->size; i++) {
            Process* p = &queue->list[i];
            if (!done[i] && p->arrival_time <= current_time && p->priority < best_prio && remaining[i] > 0) {
                best_prio = p->priority;
                idx = i;
            }
        }

        if (idx == -1) {
            current_time++;
            continue;
        }

        Process* p = &queue->list[idx];

        if (preemptive) {
            current_time++;
            remaining[idx]--;
            total_burst++;
            if (remaining[idx] == 0) {
                int wait = current_time - p->arrival_time - p->burst_time;
                int turn = current_time - p->arrival_time;
                printf("Processo %d: Espera = %d, Turnaround = %d\n", p->id, wait, turn);
                wait_time += wait;
                turnaround += turn;
                done[idx] = 1;
                completed++;
            }
        } else {
            int wait = current_time - p->arrival_time;
            current_time += p->burst_time;
            int turn = current_time - p->arrival_time;
            printf("Processo %d: Espera = %d, Turnaround = %d\n", p->id, wait, turn);
            wait_time += wait;
            turnaround += turn;
            total_burst += p->burst_time;
            done[idx] = 1;
            completed++;
        }
    }

    float avg_wait = (float)wait_time / queue->size;
    float avg_turnaround = (float)turnaround / queue->size;
    float throughput = (float)queue->size / current_time;
    float cpu_utilization = (float)total_burst / current_time * 100;

    printf("Média de espera: %.2f\n", avg_wait);
    printf("Média de turnaround: %.2f\n", avg_turnaround);
    printf("Throughput: %.2f processos/unidade de tempo\n", throughput);
    printf("Utilização da CPU: %.2f%%\n", cpu_utilization);

    free(remaining);
    free(done);
}





// Mantêm os outros como estavam (podem ser melhorados depois)
void run_round_robin(ProcessQueue* queue, int quantum) {
    int current_time = 0, completed = 0;
    int wait_time = 0, turnaround = 0, total_burst = 0;
    int* remaining = malloc(sizeof(int) * queue->size);
    for (int i = 0; i < queue->size; i++) remaining[i] = queue->list[i].burst_time;

    printf("\n[RR] Escalonamento com quantum = %d:\n", quantum);

    while (completed < queue->size) {
        int idle = 1;
        for (int i = 0; i < queue->size; i++) {
            Process* p = &queue->list[i];
            if (p->arrival_time <= current_time && remaining[i] > 0) {
                idle = 0;
                int exec_time = (remaining[i] > quantum) ? quantum : remaining[i];
                current_time += exec_time;
                total_burst += exec_time;
                remaining[i] -= exec_time;
                if (remaining[i] == 0) {
                    int wait = current_time - p->arrival_time - p->burst_time;
                    int turn = current_time - p->arrival_time;
                    printf("Processo %d: Espera = %d, Turnaround = %d\n", p->id, wait, turn);
                    wait_time += wait;
                    turnaround += turn;
                    completed++;
                }
            }
        }
        if (idle) current_time++;
    }

    float avg_wait = (float)wait_time / queue->size;
    float avg_turnaround = (float)turnaround / queue->size;
    float throughput = (float)queue->size / current_time;
    float cpu_utilization = (float)total_burst / current_time * 100;

    printf("Média de espera: %.2f\n", avg_wait);
    printf("Média de turnaround: %.2f\n", avg_turnaround);
    printf("Throughput: %.2f processos/unidade de tempo\n", throughput);
    printf("Utilização da CPU: %.2f%%\n", cpu_utilization);
    free(remaining);
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>  // Para usar clock()

void run_edf(ProcessQueue* queue) {
    int tempo_total ;
    int valor;
    printf("Insira o tempo total de simulação: ");
    scanf("%d", &valor);
    if (valor <= 0 || valor > 500) {
        printf("Erro: Tempo total de simulação inválido! Tempo padrão =100.\n");
         tempo_total = 100;  // duração da simulação
         
    }else{
        tempo_total = valor;
    }
   

    int* remaining_time = calloc(queue->size, sizeof(int));
    int* next_release = calloc(queue->size, sizeof(int));
    int* deadline_misses = calloc(queue->size, sizeof(int));
    int* current_deadline = malloc(sizeof(int) * queue->size);
    int* instancias = calloc(queue->size, sizeof(int));
    int total_cpu_time = 0;
    int current_time = 0;

    printf("\n[EDF] Escalonamento Earliest Deadline First:\n");

    // Inicializando variáveis
    for (int i = 0; i < queue->size; i++) {
        remaining_time[i] = 0;
        next_release[i] = queue->list[i].arrival_time;
        current_deadline[i] = next_release[i] + queue->list[i].period;
    }

    while (current_time < tempo_total) {
        int selected = -1;
        int earliest_deadline = __INT_MAX__;

        // Verifica e prepara os processos que precisam ser executados no tempo atual
        for (int i = 0; i < queue->size; i++) {
            if (current_time == next_release[i] && instancias[i] < MAX_INSTANCES) {
                if (remaining_time[i] > 0 && current_deadline[i] <= current_time) {
                    deadline_misses[i]++;
                    printf("MISS: Processo %d perdeu o deadline anterior!\n", queue->list[i].id);
                }

                remaining_time[i] = queue->list[i].burst_time;
                instancias[i]++;
                next_release[i] += queue->list[i].period;
                current_deadline[i] = current_time + queue->list[i].period;
            }
        }

        // Seleção do processo com o mais próximo deadline
        for (int i = 0; i < queue->size; i++) {
            if (remaining_time[i] > 0 && current_deadline[i] < earliest_deadline) {
                earliest_deadline = current_deadline[i];
                selected = i;
            }
        }

        if (selected != -1) {
            remaining_time[selected]--;
            total_cpu_time++;
            printf("Tempo %d: Processo %d executando\n", current_time, queue->list[selected].id);

            // Verifica se ultrapassou deadline
            if (remaining_time[selected] == 0 && current_time >= current_deadline[selected]) {
                deadline_misses[selected]++;
                printf("MISS: Processo %d completou após o deadline!\n", queue->list[selected].id);
            }
        } else {
            printf("Tempo %d: CPU Ociosa\n", current_time);
        }

        current_time++;
    }

    // Estatísticas
    int total_misses = 0;
    for (int i = 0; i < queue->size; i++)
        total_misses += deadline_misses[i];

    float utilization = (float)total_cpu_time / tempo_total * 100.0;
    float throughput = (float)(queue->size * MAX_INSTANCES) / tempo_total;

    printf("\n--- Estatísticas EDF ---\n");
    printf("Total de deadline misses: %d\n", total_misses);
    printf("Utilização da CPU: %.2f%%\n", utilization);
    printf("Throughput aproximado: %.2f processos/unidade de tempo\n", throughput);

    free(remaining_time);
    free(next_release);
    free(deadline_misses);
    free(current_deadline);
    free(instancias);
}


void run_rm(ProcessQueue* queue) {
    int tempo_total ;
    int valor;
    printf("Insira o tempo total de simulação: ");
    scanf("%d", &valor);
    if (valor <= 0 || valor > 500) {
        printf("Erro: Tempo total de simulação inválido! Tempo padrão =100.\n");
         tempo_total = 100;  // duração da simulação
         
    }else{
        tempo_total = valor;
    }
    int* remaining_time = calloc(queue->size, sizeof(int));
    int* next_release = calloc(queue->size, sizeof(int));
    int* deadline_misses = calloc(queue->size, sizeof(int));
    int* current_deadline = malloc(sizeof(int) * queue->size);
    int* instance_count = calloc(queue->size, sizeof(int));

    int total_cpu_time = 0;
    int current_time = 0;

    printf("\n[RM] Escalonamento Rate Monotonic:\n");

    // Inicializando variáveis
    for (int i = 0; i < queue->size; i++) {
        remaining_time[i] = 0;
        next_release[i] = queue->list[i].arrival_time;
        current_deadline[i] = next_release[i] + queue->list[i].period;
    }

    while (current_time < tempo_total) {
        // Verifica os processos que precisam ser executados no tempo atual
        for (int i = 0; i < queue->size; i++) {
            if (current_time == next_release[i] && instance_count[i] < MAX_INSTANCES) {
                if (remaining_time[i] > 0 && current_deadline[i] <= current_time) {
                    deadline_misses[i]++;
                    printf("MISS: Processo %d perdeu o deadline anterior!\n", queue->list[i].id);
                }

                remaining_time[i] = queue->list[i].burst_time;
                instance_count[i]++;
                next_release[i] += queue->list[i].period;
                current_deadline[i] = current_time + queue->list[i].period;
            }
        }

        int selected = -1;
        int shortest_period = INT_MAX;

        // Seleção do processo com o menor período
        for (int i = 0; i < queue->size; i++) {
            if (remaining_time[i] > 0 && queue->list[i].period < shortest_period) {
                shortest_period = queue->list[i].period;
                selected = i;
            }
        }

        if (selected != -1) {
            remaining_time[selected]--;
            total_cpu_time++;
            printf("Tempo %d: Processo %d executando\n", current_time, queue->list[selected].id);

            if (remaining_time[selected] == 0 && current_time >= current_deadline[selected]) {
                deadline_misses[selected]++;
                printf("MISS: Processo %d completou após o deadline!\n", queue->list[selected].id);
            }
        } else {
            printf("Tempo %d: CPU Ociosa\n", current_time);
        }

        current_time++;
    }

    // Estatísticas
    int total_misses = 0;
    for (int i = 0; i < queue->size; i++)
        total_misses += deadline_misses[i];

    float utilization = (float)total_cpu_time / tempo_total * 100.0;
    float throughput = (float)(queue->size * MAX_INSTANCES) / tempo_total;

    printf("\n--- Estatísticas RM ---\n");
    printf("Total de deadline misses: %d\n", total_misses);
    printf("Utilização da CPU: %.2f%%\n", utilization);
    printf("Throughput aproximado: %.2f processos/unidade de tempo\n", throughput);

    free(remaining_time);
    free(next_release);
    free(deadline_misses);
    free(current_deadline);
    free(instance_count);
}



void run_scheduler(ProcessQueue* queue, SchedulingAlgorithm algo, int quantum) {
    switch (algo) {
        case FCFS:
            run_fcfs(queue); break;
        case SJF:
            run_sjf(queue); break;
        case PRIORITY_PREEMPTIVE:
        case PRIORITY_NON_PREEMPTIVE:
            run_priority(queue, algo == PRIORITY_PREEMPTIVE); break;
        case ROUND_ROBIN:
            run_round_robin(queue, quantum); break;
        case RATE_MONOTONIC:
            run_rm(queue); break;
        case EDF:
            run_edf(queue); break;
        default:
            printf("Algoritmo não implementado\n");
    }
}
