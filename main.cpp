#include <iostream>
#include <string>
#include <cstdlib>
#define PROCESS_COUNT 10
#define QUANTUM 5

enum STATE {
    NEW,
    TERMINATED,
    RUNNING,
    WAITING,
    READY,
};

typedef struct Register {
    void *eip;
    void *esp;
    int eax;
    int ebx;
} REGISTER;


typedef struct Process {
    std::string name;
    void *previous;
    void *next;
    int pid;
    STATE state;
    REGISTER * _register;
} PROCESS;


typedef struct Queue {
    int count;
    void * first;
    void * last;
} QUEUE;


typedef struct Scheduler {
    int quantum;
    PROCESS* running;
    QUEUE jobQueue;
} SCHEDULER;

QUEUE jobQueue;
QUEUE readyQueue;
QUEUE deviceQueue;


Scheduler scheduler;
PROCESS * bootProcess;

int GetRandomNumber(int min, int max) {
    int random = std::rand();
    return min + (random % (max - min)) + 1;
}

void AddProcess(void * queue, void* processToAdd) {
    QUEUE *q = (QUEUE *) queue;
    PROCESS *process = (PROCESS *) processToAdd;
    process->next = nullptr;

    if (q->first == nullptr) {
        q->first = process;
        q->last = process;
        q->count = 1;

        return;
    }

    process = (PROCESS *) q->last;
    process->next = processToAdd;
    q->last = processToAdd;
    q->count++;
}


void* RemoveProcess(QUEUE * queue, int pid) {
    PROCESS* process;
    PROCESS* previousProcess;

    previousProcess = (PROCESS *) queue->first;
    for (process = previousProcess; process != nullptr; process = (PROCESS *) process->next) {
        if (process->pid == pid) {
            // When Job Queue has only one process
            if (queue->count == 1) {
                queue->first = nullptr;
                queue->last = nullptr;
                queue->count = 0;
            }

                // When Process to find is first of job queue
            else if (process == queue->first) {
                queue->first = process->next;
            }

                // When process to find is last of job queue
            else if (process == queue->last) {
                queue->last = process->previous;
            }

            else {
                previousProcess->next = process->next;
            }

            queue->count--;
            return process;
        }

        previousProcess = process;
    }

    return nullptr;
}


PROCESS* GetFirstProcess(QUEUE* queue) {
    PROCESS* process;
    if (queue->count <= 0) {
        return nullptr;
    }

    process = (PROCESS *) queue->first;
    return (PROCESS *) RemoveProcess(queue, process->pid);
}

void ExecuteProcess() {
    PROCESS * process;
    int random = 0;

    random = GetRandomNumber(1, 100);
    if (random % 2 == 0) {
        process = GetFirstProcess(&deviceQueue);
        if (process != nullptr) {
            std::cout << "[ INFO ] I/O Instruction of process " << process->name << " is ended !" << std::endl;
            process->state = READY;
            AddProcess(&readyQueue, process);
        }
    }

    process = GetFirstProcess(&readyQueue);
    process->state = RUNNING;
    std::cout << "[ INFO ] CPU executed process " << process->name << std::endl;

    random = GetRandomNumber(1, 100);
    if (random % 3 == 0) {
        process->state = WAITING;
        AddProcess( (QUEUE *)&deviceQueue, process );
        std::cout << "[ INFO ] I/O Interrupt occurred in the process " << process->name << std::endl;
        return;
    }

    random = GetRandomNumber(1, 100);
    if (random % 10 == 0){
        process->state = TERMINATED;
        std::cout << "[ INFO ] Process " << process->name << " was ended !" << std::endl;
        return;
    } else {
        process->state = READY;
        AddProcess( (QUEUE *)&readyQueue, process );
    }
}


void InitializeScheduler(void) {
    scheduler.quantum = 5;

    bootProcess->pid = 0;
    bootProcess->state = READY;

    AddProcess( &jobQueue, &bootProcess );
    AddProcess( &readyQueue, &bootProcess );
}


void SetRunningProcess(PROCESS* process) {
    scheduler.running->state = READY;
    scheduler.running = process;
    scheduler.running->state = RUNNING;
}


PROCESS* GetRunningProcess(void) {
    return scheduler.running;
}

PROCESS* GetNextProcess(void) {
    if (readyQueue.count == 0) {
        return NULL;
    }

    return (PROCESS *) GetFirstProcess( (QUEUE *) &readyQueue );
}

void AddProcessToDeviceQueue(PROCESS* process) {

}

void AddProcessToReadyList(PROCESS* process) {
    process->state = READY;
    AddProcess( &readyQueue, process );

    std::cout << "[ INFO ] Process " << process->name << " was added to ReadyQueue!" << std::endl;
}

void AddProcessToJobQueue(void * process) {
    PROCESS * proc = (PROCESS *) process;
    proc->state = NEW;
    AddProcess(&jobQueue, proc);
    std::cout << "[ INFO ] Process " << proc->name << " was added to JobQueue!" << std::endl;

    AddProcessToReadyList(proc);
}

void PrintQueue(QUEUE * queue) {
    PROCESS * process = (PROCESS *) queue->first;
    PROCESS * previousProcess;

    for (int i = 0; i < queue->count; i++) {
        std::cout << process->name;
        if (i != queue->count - 1) {
            std::cout << " -> ";
        }

        previousProcess = process;
        process = (PROCESS *) process->next;
    }
    std::cout << std::endl;
}

void PrintJobQueue() {
    std::cout << "JobQueue : ";
    PrintQueue(&jobQueue);
}

void PrintReadyQueue() {
    std::cout << "ReadyQueue : ";
    PrintQueue(&readyQueue);
}

void PrintDeviceQueue() {
    std::cout << "DeviceQueue : ";
    PrintQueue(&deviceQueue);
}

void process(void) {
    int a, b, c = 0;
    a = 10;
    b = 20;
    c = a + b;
}




int main() {
    std::srand(5345);

    PROCESS * process;
    PROCESS processes[PROCESS_COUNT];

    for (int i = 0; i < PROCESS_COUNT; i++) {
        std::string pname = "PCB";
        pname.append(std::to_string(i + 1));

        processes[i].pid = (i + 1);
        processes[i].state = READY;
        processes[i].name = pname;

        AddProcessToJobQueue( &processes[i] );
        PrintJobQueue();
        PrintReadyQueue();
    }

    int count = 100;
    while (count > 0) {
        ExecuteProcess();

        PrintReadyQueue();
        PrintDeviceQueue();

        count--;
    }

}
