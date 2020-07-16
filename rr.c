/* -----------------------------------------------------------------------------------------------------------------------------------
 * NAME: THING XIN JIE				
 * STUDENT ID: 28319060				
 * CREATED: 03/10/2019			
 * LAST MODIFIED: 04/10/2019		
 * This program 
 * -------------------------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "process-visualiser.h"

/* Special enumerated data type for process state */
typedef enum {
	READY, RUNNING, EXIT
} process_state_t;


/* C data structure used as process control block. THe scheduler
 * should create one instance per running process in the system. 
 */
typedef struct {
	char process_name[11];	//a string that identifies the process
	
	// Time measured in seconds
	int entryTime;		// time process entered system
	int serviceTime;	// the total CPU time required by the process
	int remainingTime;	// remaining service time until completion
	int waitTime;		// time from entry to start execution of a process (from ready to running)
	int count;
	process_state_t state;	// current process state (e.g. READY)
} pcb_t;

void initPCB(FILE *fp, pcb_t *pcb, int *total_process, int *total_time);
void selection_sort(int* order, int *line, pcb_t *pcb);
void scheduler(int *total_time, pcb_t *pcb, int *order, int *line);
void drawBlock(pcb_t *pcb, int *order, int *line, int *i, int *j, int*qsize, int *ready_queue);
void readyQueueHandler(pcb_t *pcb, int *qsize, int *ready_queue);
int getTotalTime(pcb_t *pcb, int *order, int *line, int *total_time);

int main(int argc, char *argv[])
{
	FILE *fp;
	char* filename;
	int total_time, line;
	
	filename = "process-data.txt";
	fp = fopen(filename, "r");
	
	pcb_t *pcb = malloc(10 * sizeof(pcb_t));
	
	initInterface("white", "black");
	initPCB(fp, pcb, &line, &total_time);

	// rearrange order of processes based on entry time (FCFS) using selection sort
	int order[line];	
	for (int i=0; i < line; i++)
	{
		order[i] = i;
	}
	
	selection_sort(order, &line, pcb);
	total_time = getTotalTime(pcb, order, &line, &total_time);
	scheduler(&total_time, pcb, order, &line);
	
	waitExit();
	fclose(fp);

	return 0;
}
	
	
void initPCB(FILE *fp, pcb_t *pcb, int *total_process, int *total_time)
{
	int num_process = 0;
	int val=0, time=0;
	
	char chr;
	chr = fgetc(fp);
	while((chr = fgetc(fp)) != EOF){
		if (chr == '\n') {
			val++;
		}
	}
	
	fseek(fp, 0, SEEK_SET);
	
	while (num_process < val){		
		fscanf(fp, "%s %d %d", pcb[num_process].process_name, &pcb[num_process].entryTime, &pcb[num_process].serviceTime);
		pcb[num_process].remainingTime = pcb[num_process].serviceTime;
		pcb[num_process].count = -1;
		appendRow(pcb[num_process].process_name);
		time += pcb[num_process].serviceTime;
		num_process++;
	}
	
	*total_time = time;
	*total_process = val;

	return;
}

void selection_sort(int* order, int *line, pcb_t *pcb)
{	
	int position, temp;
	for (int i=0; i < *line-1; i++){
		position = i;
		for (int j = i+1; j < *line; j++)
		{
			if (pcb[order[position]].entryTime > pcb[order[j]].entryTime)
			{
				position = j;
			}
		}
		if (position != i)
		{
			temp = order[i];
			order[i] = order[position];
			order[position] = temp;
		}
	}
}

int getTotalTime(pcb_t *pcb, int *order, int *line, int *total_time)
{
	//check the longest time needed for all processes
	int extra = 0;
	if (pcb[order[*line-1]].entryTime + pcb[order[*line-1]].serviceTime > *total_time)
	{
		*total_time = pcb[order[*line-1]].entryTime + pcb[order[*line-1]].serviceTime;
	}
	
	// find extra time where no process is running within time slices
	for (int i=0; i < *line; i++)
	{
		if (pcb[order[i]].serviceTime % 2 != 0)
		{
			extra += 1;
		}
	}
	*total_time += extra;

	return *total_time;
}

void scheduler(int *total_time, pcb_t *pcb, int *order, int *line)
{
	// scheduler
	int j = 0, qsize =0;
	int ready_queue[*line];

	for (int i=0; i < *total_time; i++)
	{	
		sleep(1);
		
		// add process to ready queue
		if (j < *line)
		{
			if (i == pcb[order[j]].entryTime){
				pcb[order[j]].state = READY;
				pcb[order[j]].waitTime = 0;
				qsize+=1;
				ready_queue[qsize-1] = order[j];
				printf("%s entered the system at %d seconds</n>", pcb[order[j]].process_name, pcb[order[j]].entryTime);
				if (j == 0 || pcb[order[j-1]].state == EXIT)
				{
					pcb[order[j]].state = RUNNING;
					pcb[order[j]].count = 2;
					printf("%s is in running state.</n>", pcb[order[j]].process_name);
				}
				j++;
			}
		}
		// draw process state for each process	
		drawBlock(pcb, order, line, &i, &j, &qsize, ready_queue);
	}
}

void drawBlock(pcb_t *pcb, int *order, int *line, int *i, int *j, int *qsize, int *ready_queue)
{
	// draw process state for each process	
	readyQueueHandler(pcb, qsize, ready_queue);
	
	int h = 0;
	int cur;
	while (h < *line)
	{
		if(pcb[order[h]].state == EXIT){
			h++;
			continue;
		}

		if (pcb[order[h]].remainingTime == 0)
		{			
			pcb[order[h]].state = EXIT;
			h++;
			continue;
		}
		
		
		if (h >= *j)
		{
			appendBlank(order[h], 1);
		}
		
		else if (order[h] == ready_queue[0] && pcb[order[h]].count > 0)
		{
			appendBar(order[h], 1, "red", "process in running state", 0);
			pcb[order[h]].remainingTime -= 1;
			if (pcb[order[h]].remainingTime == 0)
			{
				printf("%s completed. Turnaround time: %d seconds. Total wait time: %d seconds.</n>", pcb[order[h]].process_name, *i - pcb[order[h]].entryTime + 1, pcb[order[h]].waitTime);				
			}		
		}

		else if (order[h] != ready_queue[0] && h < *j && pcb[order[h]].count < 0)
		{
			pcb[order[h]].waitTime += 1;
			appendBar(order[h], 1, "red", "process in ready state", 1);
		}
		h++;
	}
	pcb[ready_queue[0]].count -= 1;
}

void readyQueueHandler(pcb_t *pcb, int *qsize, int *ready_queue)
{
	if (pcb[ready_queue[0]].count == 0)
	{
		if (pcb[ready_queue[0]].remainingTime > 0)
		{
			pcb[ready_queue[0]].state = READY;
			pcb[ready_queue[0]].count = -1;
			*qsize += 1;
			ready_queue[*qsize-1] = ready_queue[0];
		}

		*qsize -= 1;
		for (int i=0; i< *qsize; i++)
		{
			ready_queue[i] = ready_queue[i+1];
		}
		
		pcb[ready_queue[0]].state = RUNNING;
		pcb[ready_queue[0]].count = 2;	
	}
}
