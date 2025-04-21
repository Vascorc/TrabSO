#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "process.h"
#include "scheduler_dynamic.h"
#include "scheduler_static.h"


SchedulingAlgorithm parse_algo(const char* str) {
    if (strcmp(str, "FCFS") == 0) return FCFS;
    if (strcmp(str, "SJF") == 0) return SJF;
    if (strcmp(str, "PRIORITY") == 0) return PRIORITY_NON_PREEMPTIVE;
    if (strcmp(str, "PPRIO") == 0) return PRIORITY_PREEMPTIVE;
    if (strcmp(str, "RR") == 0) return ROUND_ROBIN;
    if (strcmp(str, "RM") == 0) return RATE_MONOTONIC;
    if (strcmp(str, "EDF") == 0) return EDF;
    return FCFS;
}

int main(int argc, char* argv[]) {
    int seed = (int)time(NULL);
    if (argc >= 6) {
        seed = atoi(argv[5]);
    }
    srand(seed);

    if (argc < 3) {
        printf("Uso: %s <ALGO> <DYNAMIC|STATICP|STATIC_RT> [argumentos adicionais]\n", argv[0]);
        return 1;
    }

    SchedulingAlgorithm algo = parse_algo(argv[1]);
    int is_dynamic = strcmp(argv[2], "DYNAMIC") == 0;
    int is_staticp = strcmp(argv[2], "STATIC_G") == 0;
    int is_static_rt = strcmp(argv[2], "STATIC_RT") == 0;

    ProcessQueue* queue = create_process_queue(10);

    if (is_dynamic) {
        int num_processes = (argc >= 4) ? atoi(argv[3]) : 5;
        if (num_processes <= 0) {
            printf("Erro: Número de processos inválido!\n");
            return 1;
        }

        for (int i = 0; i < num_processes; i++) {
            add_process(queue, generate_random_process(i + 1, i * 2));
        }

    } else if (is_staticp || is_static_rt) {
        if (argc < 4) {
            printf("Erro: No modo STATICP ou STATIC_RT, forneça o tempo máximo de simulação!\n");
            return 1;
        }

        int max_simulation_time = atoi(argv[3]);
        if (max_simulation_time <= 0) {
            printf("Erro: Tempo máximo de simulação inválido!\n");
            return 1;
        }

        if (is_staticp) {
            load_processes_from_file("data/example_input_general.txt",queue,0); // Função para carregar processos periódicos gerais
        } else if (is_static_rt) {
            load_processes_from_file("data/example_input_real_time.txt",queue,1); // Função para carregar processos de tempo real periódicos
        }

        printf("Tempo máximo de simulação: %d\n", max_simulation_time);
    }

    int quantum = (argc >= 5) ? atoi(argv[4]) : 2;

    if (is_dynamic) {
        run_scheduler(queue, algo, quantum);
    } else {
        int tempo_total = atoi(argv[3]);
        run_scheduler_static(queue, algo, quantum, tempo_total);
    }

    destroy_process_queue(queue);
    return 0;
}