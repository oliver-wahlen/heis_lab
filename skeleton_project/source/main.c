#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h> 
#include <string.h>
#include "driver/elevio.h"

#include "functionality.h"

int main(){
    const int n_floors= N_FLOORS; //bruker const istedenfor define N_FLOORS pga type check sikkerhet
    
    int floors_order[n_floors];
    int size_floor_mask=n_floors*2-2;
    int floor_mask[size_floor_mask]; //fordi hver floor er opp og ned, unntatt top and bottom
    //forsikre om at startverdien er 0
    memset(floors_order, 0, sizeof(floors_order));
    memset(floor_mask, 0, sizeof(floor_mask));

    int current_floor=-1;
    int recent_floor=-1;
    MotorDirection direction=DIRN_STOP;
    MotorDirection last_direction=DIRN_STOP;
    
    int global_paus=0;
    clock_t start_time, end_time;
    double time_taken;

    int check_stop=0;
    
    elevio_init();

    //sette alle lys til 0
    for(int f = 0; f < N_FLOORS; f++){
        for(int b = 0; b < N_BUTTONS; b++){
            int btnPressed = elevio_callButton(f, b);

                
            elevio_buttonLamp(f, b, btnPressed);
        }
    }
    elevio_doorOpenLamp(0);
    elevio_stopLamp(0);

    //finne etasje
    while (elevio_floorSensor()==-1){
        elevio_motorDirection(DIRN_DOWN);
    }
    elevio_motorDirection(DIRN_STOP);

    while(1){
        
        int current_floor = elevio_floorSensor();
        if(current_floor!=-1){recent_floor=current_floor;}
        elevio_floorIndicator(recent_floor);
        
        // stoppknap
        while(elevio_stopButton()){
            last_direction=direction;
            if(current_floor!=-1){
                elevio_doorOpenLamp(1);
                pausing(floors_order, n_floors, current_floor, &global_paus, &start_time, &time_taken);
            }
            elevio_stopLamp(1);
            elevio_motorDirection(DIRN_STOP);
            memset(floors_order, 0, sizeof(floors_order));
            memset(floor_mask, 0, sizeof(floor_mask));
            for(int f = 0; f < N_FLOORS; f++){
                for(int b = 0; b < N_BUTTONS; b++){
                    int btnPressed = elevio_callButton(f, b);

                        
                    elevio_buttonLamp(f, b, btnPressed);
                }
            }
        check_stop=1;
        }
        elevio_stopLamp(0);

        get_order(floors_order, floor_mask, n_floors, recent_floor);

        //handle stoppe midt mellom
        if(check_stop && current_floor!=recent_floor){
            if(floors_order[recent_floor] && last_direction==DIRN_DOWN){
                recent_floor-=1;
                check_stop=0;
            }
            if(floors_order[recent_floor] && last_direction==DIRN_UP){
                recent_floor+=1;
                check_stop=0;
            } 
            for(int i=0; i<n_floors; i++){
                if(floors_order[i]!=0){
                    check_stop=0;
                }
            }
        } 

        //if(current_floor!=recent_floor && floors_order[recent_floor]==1)


        //bevegelses modus
        if(global_paus == 0){
            arriving(floors_order, floor_mask, n_floors, current_floor, &direction, &global_paus, &start_time, &time_taken);

            move(floors_order, n_floors, recent_floor, &direction);
            elevio_motorDirection(direction); 
        } else { //stoppe modus
            elevio_motorDirection(DIRN_STOP);
            elevio_doorOpenLamp(1);  
            if(elevio_obstruction()){
                start_time=time(NULL);
            }
            end_time=time(NULL);
            time_taken = difftime(end_time, start_time);
        }
        //sjekke om skal bevege igjen
        if(time_taken>=3){
            elevio_buttonLamp(recent_floor, BUTTON_CAB, 0);
            elevio_buttonLamp(recent_floor, BUTTON_HALL_DOWN, 0);
            elevio_buttonLamp(recent_floor, BUTTON_HALL_UP, 0);
            global_paus=0;
            elevio_doorOpenLamp(0);
        }

        //update delay
        nanosleep(&(struct timespec){0, 1000}, NULL);
    }
    
    return 0;
}

