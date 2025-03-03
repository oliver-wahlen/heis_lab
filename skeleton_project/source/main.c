#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "driver/elevio.h"

//husk å legge til i når folk går ut av etasje å slukke ALLE lysene til den etasjen.


void get_order(int *floors_order, int *floor_mask, int n_floors, int recent_floor){
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
    if(checkempty_or_pushstart!=-1){
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
        if (*direction==DIRN_UP && recent_floor==highest){
            *direction=DIRN_DOWN;
        }
        if (*direction==DIRN_DOWN && recent_floor==lowest){
            *direction=DIRN_UP;
        }
    }
}
void pausing(){ ///FIIIXXXX
    printf("Stopped \n");
    sleep(2);
}

void arriving(int *floors_order, int *floor_mask, int n_floors, int current_floor, int *direction){
    for(int i=0; i<n_floors; i++){//sjekker alle floors
        if(floors_order[i]==1 && i==current_floor){
            if(i==0){
                floors_order[0]=0;
                floor_mask[0]=0;
                pausing();
            }
            else if(i==(n_floors-1)){//altså siste etasje
                floors_order[i]=0;
                floor_mask[2*n_floors-3]=0;
                pausing(); 
            }
            else if(*direction==DIRN_UP && floor_mask[i*2-1]==1){
                
                pausing();
            }
        }
    }
}



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
    
    elevio_init();

    //elevio_motorDirection(DIRN_UP);

    while(1){
        int current_floor = elevio_floorSensor();
        if(current_floor!=-1){recent_floor=current_floor;}


        floors_order[2]=1;

        move(floors_order, n_floors, recent_floor, &direction);
        elevio_motorDirection(direction); 

        //printf("Floor: %d \n", floor);
        

        /*test for get_order
        int recent_floor=2;
        get_order(floors_order, floor_mask, n_floors, recent_floor);
        printf("1: %d 2: %d 3: %d 4: %d \n", floors_order[0], floors_order[1], floors_order[2], floors_order[3]);
        printf("a: %d b: %d c: %d d: %d e: %d f: %d \n\n", floor_mask[0], floor_mask[1], floor_mask[2], floor_mask[3], floor_mask[4], floor_mask[5]);
        '/
        */



        /* DEFAULT CODE
        
        if(floor == 0){
            elevio_motorDirection(DIRN_UP);
        }

        if(floor == N_FLOORS-1){
            elevio_motorDirection(DIRN_DOWN);
        }
        
        for(int f = 0; f < N_FLOORS; f++){
            for(int b = 0; b < N_BUTTONS; b++){
                int btnPressed = elevio_callButton(f, b);
                //if(btnPressed){
                //    printf("Floor: %d Type: %d \n", f, b);
                //}    
                    
                elevio_buttonLamp(f, b, btnPressed);
            }
        }

        if(elevio_obstruction()){
            elevio_stopLamp(1);
         } else {
            elevio_stopLamp(0);
        }
        
        if(elevio_stopButton()){
            elevio_motorDirection(DIRN_STOP);
            break;
        }
        */

        //adjust to decrease delay
        nanosleep(&(struct timespec){0, 1000}, NULL);
    }
    
    return 0;
}

