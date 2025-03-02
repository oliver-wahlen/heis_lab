#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "driver/elevio.h"

//husk å legge til i når folk går ut av etasje å slukke ALLE lysene til den etasjen.


void get_order(int *floors_order, int *floor_mask, int n_floors, int current_floor){
    //check if any button pressed
    for(int f = 0; f < N_FLOORS; f++){
        for(int b = 0; b < N_BUTTONS; b++){
            int btnPressed = elevio_callButton(f, b);
            
            if(btnPressed){
                
                printf("Floor: %d Type: %d \n", f, b);
                
                //setter lys på
                elevio_buttonLamp(f, b, btnPressed);
                //legger til etasje til orderene
                floors_order[f]=1;
                
                //legger til polaritet i masken
                switch (b){
                    case 0:
                        if(f==0){ //sjekker endepunkt
                            floor_mask[0]=1;
                            break;
                        }
                        floor_mask[f*2-1]=1;
                        break;
                    case 1:
                        printf("Dette ble presset ned: %d \n", f);
                        if(f==(n_floors-1)){//sjekker endepunkt
                            floor_mask[n_floors*2-3]=-1;
                            break;
                        }
                        floor_mask[f*2]=-1;
                        break;
                    //VIKTIG HER ANTAR MAN AT ALLE HAR GÅTT AV/PÅ, ie ingen sjekk for f==current_floor
                    case 2:
                        //logikk som gjør at cab knapp legger til polaritet relativt til hvor cab er, slik vil stopp
                        if(f-current_floor>0){
                            if(f==0){ //sjekker endepunkt
                                floor_mask[0]=1;
                            }else{
                            floor_mask[f*2-1]=1;
                            }
                            break;
                        } else if (f-current_floor<0){
                            if(f==(n_floors-1)){//sjekker endepunkt
                                floor_mask[n_floors*2-3]=-1;
                                break;
                            }
                            floor_mask[f*2]=-1;
                            break;
                        } 
                    default:
                        break;
                }
            }         
        }
    }
}


int main(){
    const int n_floors= N_FLOORS; //bruker const istedenfor define N_FLOORS pga type check sikkerhet
    int floors_order[n_floors];
    int floor_mask[n_floors*2-2]; //fordi hver floor er opp og ned, unntatt top and bottom
    //forsikre om at startverdien er 0
    memset(floors_order, 0, sizeof(floors_order));
    memset(floor_mask, 0, sizeof(floor_mask));

    elevio_init();
    
    printf("=== Example Program ===\n");
    printf("Press the stop button on the elevator panel to exit\n");

    //elevio_motorDirection(DIRN_UP);

    while(1){
        int floor = elevio_floorSensor();
        //printf("Floor: %d \n", floor);
        
        if(floor == 0){
            elevio_motorDirection(DIRN_UP);
        }

        if(floor == N_FLOORS-1){
            elevio_motorDirection(DIRN_DOWN);
        }

        //test for get_order
        int current_floor=2;
        get_order(floors_order, floor_mask, n_floors, current_floor);
        printf("1: %d 2: %d 3: %d 4: %d \n", floors_order[0], floors_order[1], floors_order[2], floors_order[3]);
        printf("a: %d b: %d c: %d d: %d e: %d f: %d \n\n", floor_mask[0], floor_mask[1], floor_mask[2], floor_mask[3], floor_mask[4], floor_mask[5]);

        /* DEFAULT CODE
        for(int f = 0; f < N_FLOORS; f++){
            for(int b = 0; b < N_BUTTONS; b++){
                int btnPressed = elevio_callButton(f, b);
                //if(btnPressed){
                //    printf("Floor: %d Type: %d \n", f, b);
                //}    
                    
                elevio_buttonLamp(f, b, btnPressed);
            }
        }*/

        if(elevio_obstruction()){
            elevio_stopLamp(1);
         } else {
            elevio_stopLamp(0);
        }
        
        if(elevio_stopButton()){
            elevio_motorDirection(DIRN_STOP);
            break;
        }
        //adjust to decrease delay
        nanosleep(&(struct timespec){0, 1000}, NULL);
    }
    
    return 0;
}

