#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h> 
#include <string.h>
#include "driver/elevio.h"

void get_order(int *floors_order, int *floor_mask, int n_floors, int recent_floor);
void move(int *floors_order, int n_floors, int recent_floor, int *direction);
void pausing(int *floors_order, int n_floors, int i, int *global_paus, clock_t *start_time, double *time_taken);
void arriving(int *floors_order, int *floor_mask, int n_floors, int current_floor, int *direction, int *global_paus, clock_t *start_time, double *time_taken);

