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
    int NumberOfAirports=0;
    printf("Enter the number of airports to be handled/managed: ");
    scanf("%d", &NumberOfAirports);
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
    char str[]="Plane 0 has departed from Airport 0 and will land at Airport 0.";
    system("touch AirTrafficController.txt");
    FILE *file_ptr;
    file_ptr = fopen("AirTrafficController.txt", "w");
    if (file_ptr == NULL) {
        printf("Error opening file!\n");
        return 1;
    }
    int PlanesActive[10];
    for(int i=0;i<10;i++)
        PlanesActive[i]=0;
    bool flag=false;
    while(NumberOfAirports)
    {
        Message temp;
        //printf("Waiting for message \n");
        if (msgrcv(msgid, (void*) &temp, sizeof(temp), -32, 0) == -1)
        {
    	   printf("error in receiving message\n");
            exit(1);
        }
        /*printf("Message recieved %li \n", temp.type);
        for(int i=0;i<4;i++)
        {
            printf("%d ",temp.content[i]);
        }
        printf("\n");*/
        if(temp.type==31)///termination
        {   
            flag=true;
            temp.content[0]=-1;
            /*printf("sending termination order to all planes \n ");
            for(int i=0;i<10;i++)//sending termination order to all planes  
            {   
                temp.type=41+i;
                if(msgsnd(msgid, (void*) &temp, sizeof(temp), 0) == -1)
                {
    	            printf("error in sending message ");
                    exit(1);
                }
            }*/
            //printf("sending termination order to airports \n ");
            for(int i=0;i<NumberOfAirports;i++)//sending termination order to airports
            {
                temp.type=51+i;
                temp.content[0]=-1;//signal for termination
                if(msgsnd(msgid, (void*) &temp, sizeof(temp), 0) == -1)
                {
    	            printf("error in sending message ");
                    exit(1);
                }
            }
            //break;//cleanup??
        }
        if(temp.type==32)//airport confirming termination
        {
            NumberOfAirports--;
        }
        if(temp.type<=10)//departure airport sending to ATC
        {   
            //printf("Received departure airport message to ATC \n");
            temp.type=60+temp.content[2];
            //printf("ATC sending to Arrival Airport \n");
            if(msgsnd(msgid, (void*)&temp, sizeof(temp), 0) == -1)//ATC sending to Arrival Airport
            {
    	        printf("error in sending message ");
                exit(1);
            }
            //printf("Message sent \n");
            str[6]=(char) temp.content[0]+48;
            str[34]=temp.content[1]+48;
            str[61]=temp.content[2]+48;
            //printf("Writing into text file \n");
            fprintf(file_ptr, "%s\n", str);//writing into text file;
            //printf("Done writing into text file \n");
            continue;
        }
        if(temp.type<=20)//arrival airport sending to ATC
        {   
            //printf("Received arrival airport  message to ATC \n");
            temp.type=40+temp.content[0];
            temp.content[0]=0;
            //printf("ATC sending to Plane \n");
            if(msgsnd(msgid, (void*) &temp, sizeof(temp), 0) == -1)//ATC sending to Plane
            {
    	        printf("error in sending message ");
                exit(1);
            }
            //printf("Message sent \n");
            continue;
        }
        if(temp.type<=30)//plane sending to ATC
        {   
            if(flag==true)//termination order has been given
            {
                temp.type=40+temp.content[0];
                temp.content[0]=-1;
                if(msgsnd(msgid, (void*) &temp, sizeof(temp), 0) == -1)
                {
    	            printf("error in sending message ");
                    exit(1);
                }
                continue;
            }
            //printf("Received plane  message to ATC \n");
            temp.type=50+temp.content[1];
            //printf("ATC sending to  departure airport \n");
            if(msgsnd(msgid, (void*) &temp, sizeof(temp) , 0) == -1)//ATC sending to  departure airport
            {
    	        printf("error in sending message ");
                exit(1);
            }
           // printf("Message sent \n");
            temp.type=50+temp.content[2];
            //printf("ATC sending to  arrival airport \n");
            if(msgsnd(msgid, (void*) &temp, sizeof(temp), 0) == -1)//ATC sending to  arrival airport
            {
    	        printf("error in sending message ");
                exit(1);
            }
            //printf("Message sent \n");
        }

    }
}