#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define RESERVE 500
#define DEPOT 1
#define START 2
#define ROUTE 3
#define END_ROUTE 4
#define CRASH 5
#define REFUEL 5000

int fuel = 5000;
int STAY = 1, D_STAY = 0;
int processNumber;
int processNr;
int numberOfBus;
int numberOfWays = 4;
int numberOfWaysTaken = 0;
int tag = 50;
int send[2];
int take[2];
MPI_Status mpi_status;

void Send(int busNumber, int state)
{
    send[0] = busNumber;
    send[1] = state;
    MPI_Send(&send, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    sleep(1);
}

void Depot(int processNumber)
{
    int busNumber, status;
    numberOfBus = processNumber - 1;
    sleep(2);

    while(numberOfWays <= numberOfBus)
    {
        MPI_Recv(&take, 2, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &mpi_status);
        busNumber = take[0];
        status = take[1];
        if(status == 1)
        {
            printf("Autobus %d czeka w bazie \n", busNumber);
        }
        if(status == 2)
        {
            printf("Autobus %d rusza w trase nr %d \n", busNumber, numberOfWaysTaken);
            numberOfWaysTaken--;
        }
        if(status == 3)
        {
            printf("Autobus %d w trasie... \n", busNumber);
        }
        if(status == 4)
        {
            if(numberOfWaysTaken < numberOfWays)
            {
                numberOfWaysTaken++;
                MPI_Send(&STAY, 1, MPI_INT, busNumber, tag, MPI_COMM_WORLD);
            }
            else
            {
                MPI_Send(&D_STAY, 1, MPI_INT, busNumber, tag, MPI_COMM_WORLD);
            }
        }
        if(status == 5)
        {
            numberOfBus--;
            printf("Liczba autobusow: %d\n", numberOfBus);
    	}
    }
}

void Bus()
{
    int state, sum, i;
    state = ROUTE;
    
    while(1)
    {
        if(state == 1)
        {
            if(rand()%2 == 1)
            {
                state = START;
                fuel = REFUEL;
                printf("Autobus %d czeka na zezwolenie na rozpoczecie podrozy \n", processNr);
                Send(processNr, state);
            }
            else
            {
                Send(processNr, state);
            }
        }
        else if(state == 2)
        {
            printf("Autobus %d rusza w trase \n", processNr);
            state = ROUTE;
            Send(processNr,state);
        }
        else if(state == 3)
        {
            fuel -= rand()%500;
            if(fuel <= RESERVE)
            {
                state = END_ROUTE;
                printf("Autobus czeka na zezwolenie na opuszczenie bazy \n");
                Send(processNr,state);
            }

            else
            {
                for(i = 0; rand()%10000; i++);
            }
        }
        else if(state == 4)
        {
            int temp;
            MPI_Recv(&temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &mpi_status);
            if(temp == STAY)
            {
                state = DEPOT;
                printf("Trasa autobusu %d zakonczona \n", processNr);
            }
            else
            {
                fuel -= rand()%500;
                if(fuel > 0)
                {
                    Send(processNr, state);
                }
                else
                {
                    state = CRASH;
                    printf("Stluczka \n");
                    Send(processNr, state);
                return;
                }
            }
        }
    }
}


int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &processNr);
    MPI_Comm_size(MPI_COMM_WORLD, &processNumber);
    srand(time(NULL));
    
    if(processNr == 0)
    	Depot(processNumber);
    else
    	Bus();
    
    MPI_Finalize();
    return 0;
}
