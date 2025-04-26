#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "scheduler_dynamic.h"
#include "scheduler_static.h"
#include <limits.h>

int compare_by_arrival(const void *a, const void *b)
{
    return ((Process *)a)->arrival_time - ((Process *)b)->arrival_time;
}

//--------IMPLEMENTACAO MODO STATIC--------------

void run_scheduler_static(ProcessQueue *queue, SchedulingAlgorithm algo, int quantum, int tempo_total)
{
    if (queue->size > 0 && queue->list[0].priority != -1)
    {
        // Processos simples (não periódicos)
        switch (algo)
        {
        case FCFS:
            run_fcfs_static(queue, tempo_total);
            break;
        case SJF:
            run_sjf_static(queue, tempo_total);
            break;
        case PRIORITY_PREEMPTIVE:
        case PRIORITY_NON_PREEMPTIVE:
            run_priority_static(queue, algo == PRIORITY_PREEMPTIVE, tempo_total);
            break;
        case ROUND_ROBIN:
            run_round_robin_static(queue, quantum, tempo_total);
            break;
        default:
            printf("Algoritmo não suportado para processos simples.\n");
        }
    }
    else
    {
        // Processos periódicos
        switch (algo)
        {
        case RATE_MONOTONIC:
            run_rm_static(queue, tempo_total);
            break;
        case EDF:
            run_edf_static(queue, tempo_total);
            break;
        default:
            printf("Algoritmo não suportado para processos periódicos.\n");
        }
    }
}

void run_fcfs_static(ProcessQueue *queue, int tempo_total)
{
    // Ordena os processos pela chegada
    qsort(queue->list, queue->size, sizeof(Process), compare_by_arrival);

    int current_time = 0;
    int total_wait = 0, total_turnaround = 0, executed = 0;

    printf("\n[FCFS STATIC] Tempo limite = %d\n", tempo_total);

    // Itera sobre os processos na fila
    for (int i = 0; i < queue->size && current_time < tempo_total; i++)
    {
        Process p = queue->list[i];

        // Se o tempo atual for menor que o tempo de chegada, atualiza o tempo atual
        if (current_time < p.arrival_time)
        {
            current_time = p.arrival_time;
        }

        // Se o tempo total de execução exceder o tempo máximo, interrompe a execução
        int burst_time_to_execute = p.burst_time;
        if (current_time + burst_time_to_execute > tempo_total)
        {
            // Ajuste: o tempo de execução do processo será limitado ao tempo restante
            burst_time_to_execute = tempo_total - current_time;
        }

        // Calcula o tempo de espera e o turnaround
        int wait_time = current_time - p.arrival_time;
        int turnaround = wait_time + burst_time_to_execute;

        // Atualiza o tempo atual após a execução do processo
        current_time += burst_time_to_execute;

        // Exibe as informações do processo
        printf("Processo %d: Espera = %d, Turnaround = %d\n", p.id, wait_time, turnaround);

        // Acumula os totais para média
        total_wait += wait_time;
        total_turnaround += turnaround;
        executed++;
    }

    float avg_wait = executed ? (float)total_wait / executed : 0;
    float avg_turnaround = executed ? (float)total_turnaround / executed : 0;

    float throughput = (float)executed / tempo_total;
    float cpu_utilization = (current_time > tempo_total ? tempo_total : current_time) / tempo_total * 100;

    printf("Média de espera: %.2f\n", avg_wait);
    printf("Média de turnaround: %.2f\n", avg_turnaround);
    printf("Throughput: %.2f\n", throughput);
    printf("Utilização da CPU: %.2f%%\n", cpu_utilization);
}

void run_sjf_static(ProcessQueue *queue, int tempo_total)
{
    int current_time = 0, completed = 0;
    int wait_time = 0, turnaround = 0;
    int *done = calloc(queue->size, sizeof(int));

    printf("\n[SJF STATIC] Tempo limite = %d\n", tempo_total);

    while (completed < queue->size && current_time < tempo_total)
    {
        int idx = -1;
        int min_burst = INT_MAX;
        for (int i = 0; i < queue->size; i++)
        {
            Process *p = &queue->list[i];
            if (!done[i] && p->arrival_time <= current_time && p->burst_time < min_burst)
            {
                min_burst = p->burst_time;
                idx = i;
            }
        }

        if (idx == -1)
        {
            current_time++;
            continue;
        }

        Process *p = &queue->list[idx];
        if (current_time + p->burst_time > tempo_total)
            break;

        int wait = current_time - p->arrival_time;
        int turn = wait + p->burst_time;
        current_time += p->burst_time;

        printf("Processo %d: Espera = %d, Turnaround = %d\n", p->id, wait, turn);
        wait_time += wait;
        turnaround += turn;
        done[idx] = 1;
        completed++;
    }

    float avg_wait = completed ? (float)wait_time / completed : 0;
    float avg_turnaround = completed ? (float)turnaround / completed : 0;
    float throughput = (float)completed / tempo_total;
    float cpu_utilization = (float)(current_time) / tempo_total * 100;

    printf("Média de espera: %.2f\n", avg_wait);
    printf("Média de turnaround: %.2f\n", avg_turnaround);
    printf("Throughput: %.2f\n", throughput);
    printf("Utilização da CPU: %.2f%%\n", cpu_utilization);

    free(done);
}

void run_priority_static(ProcessQueue *queue, int preemptive, int tempo_total)
{
    int current_time = 0, completed = 0;
    int wait_time = 0, turnaround = 0;
    int *remaining = malloc(sizeof(int) * queue->size);
    int *done = calloc(queue->size, sizeof(int));

    for (int i = 0; i < queue->size; i++)
        remaining[i] = queue->list[i].burst_time;

    printf("\n[PRIORITY STATIC %s] Tempo limite = %d\n", preemptive ? "Preemptivo" : "Não-Preemptivo", tempo_total);

    while (completed < queue->size && current_time < tempo_total)
    {
        int idx = -1;
        int best_prio = INT_MAX;
        for (int i = 0; i < queue->size; i++)
        {
            Process *p = &queue->list[i];
            if (!done[i] && p->arrival_time <= current_time && p->priority < best_prio && remaining[i] > 0)
            {
                best_prio = p->priority;
                idx = i;
            }
        }

        if (idx == -1)
        {
            current_time++;
            continue;
        }

        Process *p = &queue->list[idx];

        if (preemptive)
        {
            remaining[idx]--;
            current_time++;
            if (remaining[idx] == 0)
            {
                int wait = current_time - p->arrival_time - p->burst_time;
                int turn = current_time - p->arrival_time;
                printf("Processo %d: Espera = %d, Turnaround = %d\n", p->id, wait, turn);
                wait_time += wait;
                turnaround += turn;
                done[idx] = 1;
                completed++;
            }
        }
        else
        {
            if (current_time + p->burst_time > tempo_total)
                break;

            int wait = current_time - p->arrival_time;
            current_time += p->burst_time;
            int turn = current_time - p->arrival_time;
            printf("Processo %d: Espera = %d, Turnaround = %d\n", p->id, wait, turn);
            wait_time += wait;
            turnaround += turn;
            done[idx] = 1;
            completed++;
        }
    }

    float avg_wait = completed ? (float)wait_time / completed : 0;
    float avg_turnaround = completed ? (float)turnaround / completed : 0;
    float throughput = (float)completed / tempo_total;
    float cpu_utilization = (float)(current_time) / tempo_total * 100;

    printf("Média de espera: %.2f\n", avg_wait);
    printf("Média de turnaround: %.2f\n", avg_turnaround);
    printf("Throughput: %.2f\n", throughput);
    printf("Utilização da CPU: %.2f%%\n", cpu_utilization);

    free(remaining);
    free(done);
}

void run_round_robin_static(ProcessQueue *queue, int quantum, int tempo_total)
{
    int current_time = 0, completed = 0;
    int wait_time = 0, turnaround = 0, total_burst = 0;
    int *remaining = malloc(sizeof(int) * queue->size);
    int *last_execution = calloc(queue->size, sizeof(int));

    for (int i = 0; i < queue->size; i++)
    {
        remaining[i] = queue->list[i].burst_time;
        last_execution[i] = -1;
    }

    printf("\n[RR-Static] Quantum = %d | Tempo limite = %d\n", quantum, tempo_total);

    while (current_time < tempo_total)
    {
        int executed_any = 0;
        for (int i = 0; i < queue->size; i++)
        {
            Process *p = &queue->list[i];
            if (p->arrival_time <= current_time && remaining[i] > 0)
            {
                int exec_time = (remaining[i] > quantum) ? quantum : remaining[i];
                if (current_time + exec_time > tempo_total)
                    exec_time = tempo_total - current_time;

                current_time += exec_time;
                total_burst += exec_time;
                remaining[i] -= exec_time;
                executed_any = 1;

                if (remaining[i] == 0)
                {
                    int wait = current_time - p->arrival_time - p->burst_time;
                    int turn = current_time - p->arrival_time;
                    printf("Processo %d: Espera = %d, Turnaround = %d\n", p->id, wait, turn);
                    wait_time += wait;
                    turnaround += turn;
                    completed++;
                }

                if (current_time >= tempo_total)
                    break;
            }
        }

        if (!executed_any)
            current_time++;
    }

    float avg_wait = completed ? (float)wait_time / completed : 0;
    float avg_turnaround = completed ? (float)turnaround / completed : 0;
    float throughput = (float)completed / tempo_total;
    float cpu_utilization = (float)total_burst / tempo_total * 100;

    printf("Média de espera: %.2f\n", avg_wait);
    printf("Média de turnaround: %.2f\n", avg_turnaround);
    printf("Throughput: %.2f processos/unidade de tempo\n", throughput);
    printf("Utilização da CPU: %.2f%%\n", cpu_utilization);

    free(remaining);
    free(last_execution);
}

void run_rm_static(ProcessQueue *queue, int tempo_total)
{
    int *remaining_time = calloc(queue->size, sizeof(int));
    int *next_release = calloc(queue->size, sizeof(int));
    int *deadline_misses = calloc(queue->size, sizeof(int));
    int *current_deadline = malloc(sizeof(int) * queue->size);
    int total_cpu_time = 0;
    int current_time = 0;

    printf("\n[RM-Static] Escalonamento Rate Monotonic | Tempo limite = %d\n", tempo_total);

    for (int i = 0; i < queue->size; i++)
    {
        next_release[i] = queue->list[i].arrival_time;
        current_deadline[i] = next_release[i] + queue->list[i].period;
    }

    while (current_time < tempo_total)
    {
        int selected = -1;
        int best_period = INT_MAX;

        // Checando os novos lançamentos
        // Checando os novos lançamentos
        for (int i = 0; i < queue->size; i++)
        {
            if (current_time == next_release[i])
            {
                // Verificando falha de deadline
                if (remaining_time[i] > 0 && current_time > current_deadline[i])
                {
                    deadline_misses[i]++;
                    printf("MISS: Processo %d perdeu o deadline! (Tempo: %d)\n", queue->list[i].id, current_time);
                }
                remaining_time[i] = queue->list[i].burst_time;
                next_release[i] += queue->list[i].period;                      // Atualizando o lançamento do processo
                current_deadline[i] = next_release[i] + queue->list[i].period; // Atualizando o deadline
            }
        }

        // Selecionando o processo com o menor período
        for (int i = 0; i < queue->size; i++)
        {
            if (remaining_time[i] > 0 && queue->list[i].period < best_period)
            {
                best_period = queue->list[i].period;
                selected = i;
            }
        }

        // Executando o processo selecionado
        if (selected != -1)
        {
            remaining_time[selected]--;
            total_cpu_time++;
            printf("Tempo %d: Processo %d executando\n", current_time, queue->list[selected].id);
        }
        else
        {
            printf("Tempo %d: CPU Ociosa\n", current_time);
        }

        current_time++;
    }

    int total_misses = 0;
    for (int i = 0; i < queue->size; i++)
        total_misses += deadline_misses[i];

    // Cálculo da utilização da CPU
    float utilization = (float)total_cpu_time / tempo_total * 100.0;

    // Ajuste de throughput para refletir o número real de processos completados
    float throughput = (float)total_cpu_time / tempo_total * queue->size;

    printf("\n--- Estatísticas RM (Static) ---\n");
    printf("Total de deadline misses: %d\n", total_misses);
    printf("Utilização da CPU: %.2f%%\n", utilization);
    printf("Throughput aproximado: %.2f processos/unidade de tempo\n", throughput);

    free(remaining_time);
    free(next_release);
    free(deadline_misses);
    free(current_deadline);
}

void run_edf_static(ProcessQueue *queue, int tempo_total)
{
    int *remaining_time = calloc(queue->size, sizeof(int));
    int *next_release = calloc(queue->size, sizeof(int));
    int *deadline_misses = calloc(queue->size, sizeof(int));
    int *current_deadline = malloc(sizeof(int) * queue->size);
    int total_cpu_time = 0;
    int current_time = 0;

    printf("\n[EDF-Static] Escalonamento Earliest Deadline First | Tempo limite = %d\n", tempo_total);

    for (int i = 0; i < queue->size; i++)
    {
        next_release[i] = queue->list[i].arrival_time;
        current_deadline[i] = next_release[i] + queue->list[i].period;
    }

    while (current_time < tempo_total)
    {
        int selected = -1;
        int earliest_deadline = INT_MAX;

        // Checando os novos lançamentos
        for (int i = 0; i < queue->size; i++)
        {
            if (current_time == next_release[i])
            {
                // Verificando falha de deadline
                if (remaining_time[i] > 0 && current_time > current_deadline[i])
                {
                    deadline_misses[i]++;
                    printf("MISS: Processo %d perdeu o deadline! (Tempo: %d)\n", queue->list[i].id, current_time);
                }
                remaining_time[i] = queue->list[i].burst_time;
                next_release[i] += queue->list[i].period;
                current_deadline[i] = next_release[i];
            }
        }

        // Selecionando o processo com o deadline mais próximo
        for (int i = 0; i < queue->size; i++)
        {
            if (remaining_time[i] > 0 && current_deadline[i] < earliest_deadline)
            {
                earliest_deadline = current_deadline[i];
                selected = i;
            }
        }

        // Executando o processo selecionado
        if (selected != -1)
        {
            remaining_time[selected]--;
            total_cpu_time++;
            printf("Tempo %d: Processo %d executando\n", current_time, queue->list[selected].id);
        }
        else
        {
            printf("Tempo %d: CPU Ociosa\n", current_time);
        }

        current_time++;
    }

    int total_misses = 0;
    for (int i = 0; i < queue->size; i++)
        total_misses += deadline_misses[i];

    // Cálculo da utilização da CPU
    float utilization = (float)total_cpu_time / tempo_total * 100.0;

    // Ajuste de throughput para refletir o número real de processos completados
    float throughput = (float)total_cpu_time / tempo_total * queue->size;

    printf("\n--- Estatísticas EDF (Static) ---\n");
    printf("Total de deadline misses: %d\n", total_misses);
    printf("Utilização da CPU: %.2f%%\n", utilization);
    printf("Throughput aproximado: %.2f processos/unidade de tempo\n", throughput);

    free(remaining_time);
    free(next_release);
    free(deadline_misses);
    free(current_deadline);
}