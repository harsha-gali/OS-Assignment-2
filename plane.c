#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ipc.h>
#include <stdbool.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
typedef struct
{
    long type;
    int content[4];//0 is planeID, 1 is departure airport, 2 is arrival airport
    // 3 is  total weight
} Message;

int main()
{
    int PlaneID=0;
    int PlaneType=0;
    int SeatsOccupied=0;;
    int NumberofCargoItems=0;
    int AverageWeightCargo=0;
    int totalWeight=0;
    printf("Enter Plane ID:");
    scanf("%d" , &PlaneID);
    printf("Enter Type of Plane:");
    scanf("%d", &PlaneType);
    if(PlaneType==1)// Passenger Plane
    {
        printf("Enter Number of Occupied Seats:");
        scanf("%d", &SeatsOccupied);
        int PassengerPlanePipes[SeatsOccupied][2];
        int Weights[SeatsOccupied][2];
        totalWeight+=75*7;//Adding crew weight to total weight
        for(int i=0;i<SeatsOccupied;i++)
        {   
            if(pipe(PassengerPlanePipes[i]) == -1)
            {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
            pid_t pid=fork();
            if (pid < 0)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if(pid==0)//Child Process
            {
                close(PassengerPlanePipes[i][0]); // Close read end
                int LuggageWeight=0;
                printf("Enter Weight of Your Luggage:");
                scanf("%d", &LuggageWeight);
                int BodyWeight=0;
                printf("Enter Your Body Weight:");
                scanf("%d", &BodyWeight); 
                int TotalWeight=BodyWeight+LuggageWeight;
                write(PassengerPlanePipes[i][1], &TotalWeight, sizeof(TotalWeight));
                close(PassengerPlanePipes[i][1]); // Close write end
                //printf("child %d done\n",i);
                exit(EXIT_SUCCESS);
            }
            else//Parent Process
            {   
                //printf("parent %d start\n",i);
                wait(NULL);//wait for passenger to finish entering his details
                close(PassengerPlanePipes[i][1]); // Close write end
                read(PassengerPlanePipes[i][0], &Weights[i][0], sizeof(Weights[i][0]));//Reading Total Weight
                totalWeight+=Weights[i][0];//Updating totalWeight
                //printf("parent %d done\n",i);
                close(PassengerPlanePipes[i][0]);
            }
        }
    }
    else//Cargo Plane
    {   
        totalWeight+=75*2;//Adding crew weight to total weight
        printf("Enter Number of Cargo Items:");
        scanf("%d" , &NumberofCargoItems);

        printf("Enter Average Weight of Cargo Items:");
        scanf("%d" , &AverageWeightCargo);

        totalWeight+=NumberofCargoItems*AverageWeightCargo;//Updating totalWeight;
    }

    int DepartureAirport=0;
    printf("Enter Airport Number for Departure:");
    scanf("%d",&DepartureAirport);
    int ArrivalAirport=0;
    printf("Enter Airport Number for Arrival:");
    scanf("%d",&ArrivalAirport);
    key_t key;
    int msgid;
    system("touch msgq.txt");
    key = ftok("msgq.txt", 0);
    if (key == -1)
    {
        printf("error in creating unique key\n");
        exit(1);
    }
    msgid = msgget(key, 0644|IPC_CREAT);  
    Message m;
    m.type=PlaneID+20;
    m.content[0]=PlaneID;
    m.content[1]=DepartureAirport;
    m.content[2]=ArrivalAirport;
    m.content[3]=totalWeight;
    /*for(int i=0;i<4;i++)
    {
        printf("%d ",m.content[i]);
    }*/
    if (msgid == -1)
    {
        printf("error in creating message queue\n");
        exit(1);
    }
    if(msgsnd(msgid, (void*) &m, sizeof(m), 0) == -1){
    	   printf("error in sending message ");
        exit(1);
    }
   // printf("Message sent \n");
    Message temp;
    if (msgrcv(msgid, (void*) &temp, sizeof(m), PlaneID+40, 0) == -1){
    	   printf("error in receiving message\n");
        exit(1);
    }
    //printf("Message Recieved \n");
    if(temp.content[0]==0)
        printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n", PlaneID, DepartureAirport,  ArrivalAirport);
    exit(1);
}   