#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h> 
#include <string.h>
#include "driver/elevio.h"

#include "functionality.h"


void get_order(int *floors_order, int *floor_mask, int n_floors, int recent_floor){
    //check if any button pressed
    for(int f = 0; f < N_FLOORS; f++){
        for(int b = 0; b < N_BUTTONS; b++){
            int btnPressed = elevio_callButton(f, b);
            
            if(btnPressed){
                                
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
                        if(f==(n_floors-1)){//sjekker endepunkt
                            floor_mask[n_floors*2-3]=-1;
                            break;
                        }
                        floor_mask[f*2]=-1;
                        break;
                    //VIKTIG HER ANTAR MAN AT ALLE HAR GÅTT AV/PÅ, ie ingen sjekk for f==recent_floor
                    case 2:
                        //logikk som gjør at cab knapp legger til polaritet relativt til hvor cab er, slik vil stopp
                        if(f-recent_floor>0){
                            if(f==0){ //sjekker endepunkt
                                floor_mask[0]=1;
                            }else{
                            floor_mask[f*2-1]=1;
                            }
                            break;
                        } else if (f-recent_floor<0){
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


void move(int *floors_order, int n_floors, int recent_floor, int *direction){
    //sjekker om tom, ie ingen bestilling, og da stopper

    int checkempty_or_pushstart=-1;
    int lowest=n_floors;
    int highest=-1;
    for(int i=0; i<n_floors; i++){
        if(floors_order[i]!=0){
            checkempty_or_pushstart=i;
            if(i<lowest){
                lowest=i;
            }
            if(i>highest){
                highest=i;
            }
        }
    }

    if(checkempty_or_pushstart==-1){
        *direction=DIRN_STOP; 
        return;
    }
    
    //hvis ikke empty vil finne en eller annen etasje
    if(*direction==DIRN_STOP){
        if(checkempty_or_pushstart>recent_floor){
            *direction=DIRN_UP;
        }else if(checkempty_or_pushstart<recent_floor){
            *direction=DIRN_DOWN;
        }
    } else{
        if (*direction==DIRN_UP && recent_floor==(n_floors-1)){
            *direction=DIRN_STOP;
        }
        if (*direction==DIRN_DOWN && recent_floor==0){
            *direction=DIRN_STOP;
        }
        else if (*direction==DIRN_UP && recent_floor==highest){
            *direction=DIRN_DOWN;
        }
        else if (*direction==DIRN_DOWN && recent_floor==lowest){
            *direction=DIRN_UP;
        }
    }
}
void pausing(int *floors_order, int n_floors, int i, int *global_paus, clock_t *start_time, double *time_taken){
    floors_order[i]=0;
    elevio_motorDirection(DIRN_STOP);
    *global_paus=1;
    *time_taken=0;
    *start_time=time(NULL);
}

void arriving(int *floors_order, int *floor_mask, int n_floors, int current_floor, int *direction, int *global_paus, clock_t *start_time, double *time_taken){
    for(int i=0; i<n_floors; i++){//sjekker alle floors
        if(floors_order[i]==1 && i==current_floor){
            if(i==0){
                floor_mask[0]=0;
                pausing(floors_order, n_floors, i, global_paus, start_time, time_taken);        
            }
            else if(i==(n_floors-1)){//altså siste etasje
                floor_mask[2*n_floors-3]=0;
                pausing(floors_order, n_floors, i, global_paus, start_time, time_taken); 
            }
            else if(*direction==DIRN_UP && floor_mask[i*2-1]==1){
                floor_mask[i*2]=0; //siden antar alle går av
                floor_mask[i*2-1]=0;
                pausing(floors_order, n_floors, i, global_paus, start_time, time_taken);             
            }
            else if(*direction==DIRN_DOWN && floor_mask[i*2]==-1){
                floor_mask[i*2]=0; //siden antar alle går av
                floor_mask[i*2-1]=0;
                pausing(floors_order, n_floors, i, global_paus, start_time, time_taken);             
            } 
            else if(*direction==DIRN_STOP){ //hva hvis i ro hæ?
                floor_mask[i*2]=0; //siden antar alle går av
                floor_mask[i*2-1]=0;
                pausing(floors_order, n_floors, i, global_paus, start_time, time_taken);             
            }
        }
    }
}