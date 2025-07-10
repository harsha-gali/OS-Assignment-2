#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ipc.h>
#include <stdbool.h>
#include <sys/msg.h>
pthread_mutex_t runways[11]={ PTHREAD_MUTEX_INITIALIZER };//declaring one mutex for each runway
pthread_mutex_t ActiveThreads=PTHREAD_MUTEX_INITIALIZER;//declaring a mutex for maintaing a consistent count for number of active threads
int threadsActive=0;
int RunwayCapacity[11]={0};
int NumberOfRunways=0;
typedef struct
{
    long type;
    int content[4];//0 is planeID, 1 is departure airport, 2 is arrival airport
    // 3 is  total weight
} Message;

typedef struct {
    int content[5];//0 is planeID, 1 is departure airport, 2 is arrival airport
    // 3 is  total weight, 4 is threadnumber
    int AirportNumber;
} Data;
void *departure(void*param)
{   
    Data *t=(Data*)param;
    int threadNumber=t->content[4];
    //printf("Thread %d Plane:%d Inside departure thread \n", threadNumber,t->content[0]);
    // printf("Thread %d Plane:%d Inside departure thread Finding Runway \n", threadNumber, t->content[0]);
    int runway=-1;
    int MinDifference=100000;
    bool flag=false;
    int possibleRunways=0;
    for(int i=0;i<NumberOfRunways;i++)
    {   
        int diff=RunwayCapacity[i]-t->content[3];
        if(diff>=0)//checking if the current runway has the capacity for the plane
        {   
            //printf("%d %d %d\n",RunwayCapacity[i],t->content[3],diff);
            flag=true;
            if(MinDifference==diff)
            {
                possibleRunways++;
                continue;
            }
            if(MinDifference>diff)//checking if the difference is min
            {   
                MinDifference=RunwayCapacity[i]-t->content[3];//updating MinDifference
                runway=i;//updating runway;
                possibleRunways=1;
            }
        }   
    }
    if(flag==false)//no runway has the capacity for the current plane hence backup runway needs to be used
    {   
        runway=NumberOfRunways;
        pthread_mutex_lock(&runways[runway]);
    }
    else
    {   
        //printf("Thread %d Plane:%d Inside departure thread trying to find runway\n", threadNumber, t->content[0]);
        int possibleRunwaysarr[possibleRunways];//creating an array for storing all the possible runways
        int curr=0;
        for(int i=0;i<NumberOfRunways;i++)
        {
            int diff=RunwayCapacity[i]-t->content[3];
            if(diff==MinDifference)
            {
                possibleRunwaysarr[curr]=i;
                //printf("%d ", i);
                curr++;
            }
        }
        //printf("\n");
        flag=true;
        while(flag)//trying to lock the runway
        {
            for(int i=0;i<possibleRunways;i++)
            {
                if(pthread_mutex_trylock(&runways[possibleRunwaysarr[i]])==0)//checking if succesful in locking
                {   
                    runway=possibleRunwaysarr[i];
                    flag=false;
                    break;
                }
            }
        }
    }
    //printf("Thread %d Plane:%d Inside departure thread Acquired runway %d\n", threadNumber, t->content[0],runway+1);
    sleep(3);//simulating boarding
    Message temp;
    temp.type=t->content[1];
    temp.content[0]=t->content[0];
    temp.content[1]=t->content[1];
    temp.content[2]=t->content[2];
    temp.content[3]=t->content[3];
    //printf("Thread %d Plane:%d informing ATC that plane has taken off\n", threadNumber,t->content[0]);
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
    if(msgsnd(msgid, (void *)&temp, sizeof(temp), 0) == -1)//informing ATC that the plane has successfully completed takeoff
    {
    	printf("error in sending message ");
        exit(1);
    }
    printf("Plane %d has completed boarding/loading and taken off from Runway No. %d of Airport No. %d. \n", t->content[0],runway+1,t->content[1]);
    pthread_mutex_unlock(&runways[runway]);//unlocking runway
    pthread_mutex_lock(&ActiveThreads);
    threadsActive--;//updating active threads count
    pthread_mutex_unlock(&ActiveThreads);
    pthread_exit(0);
}
void *arrival(void*param)
{
    Data *t=(Data*)param;
    //printf("Thread:%d Plane:%d Inside arrival thread \n", t->content[4],t->content[0]);
    //printf("Thread:%d Plane:%d Inside arrival thread Finding Runway \n", t->content[4],t->content[0]);
    int runway=-1;
    int MinDifference=100000;
    bool flag=false;
    int possibleRunways=0;
    for(int i=0;i<NumberOfRunways;i++)
    {   
        int diff=RunwayCapacity[i]-t->content[3];
        if(diff>=0)//checking if the current runway has the capacity for the plane
        {   
            ///printf("%d %d %d\n",RunwayCapacity[i],t->content[3],diff);
            flag=true;
            if(MinDifference==diff)
            {
                possibleRunways++;
                continue;
            }
            if(MinDifference>diff)//checking if the difference is min
            {   
                MinDifference=RunwayCapacity[i]-t->content[3];//updating MinDifference
                runway=i;//updating runway;
                possibleRunways=1;
            }
        }   
    }
    Message temp;
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
    if (msgrcv(msgid, (void *)&temp, sizeof(temp), 60+t->AirportNumber, 0) == -1)//waiting for take off confirmation
    {
    	printf("error in receiving message\n");
        exit(1);
    }
    //printf("Thread:%d Plane:%d Inside arrival thread, plane has taken off\n",t->content[4],t->content[0]);
    sleep(30);//simulating journey
    if(flag==false)//no runway has the capacity for the current plane hence backup runway needs to be used
    {   
        runway=NumberOfRunways;
        pthread_mutex_lock(&runways[runway]);
    }
    else
    {   
        //printf("Thread %d  Plane:%d Inside arrival thread trying to find runway\n", t->content[4], t->content[0]);
        int possibleRunwaysarr[possibleRunways];//creating an array for storing all the possible runways
        int curr=0;
        for(int i=0;i<NumberOfRunways;i++)
        {
            int diff=RunwayCapacity[i]-t->content[3];
            if(diff==MinDifference)
            {
                possibleRunwaysarr[curr]=i;
                //printf("%d ", i);
                curr++;
            }
        }
        //printf("\n");
        flag=true;
        while(flag)//trying to lock the runway
        {
            for(int i=0;i<possibleRunways;i++)
            {
                if(pthread_mutex_trylock(&runways[possibleRunwaysarr[i]])==0)//checking if succesful in locking
                {   
                    runway=possibleRunwaysarr[i];
                    flag=false;
                    break;
                }
            }
        }
    }
    //printf("Thread:%d Plane:%d Inside arrival thread Acquired runway %d\n",t->content[4],t->content[0],runway+1);
    //printf("Thread:%d Plane:%d Inside arrival thread, plane has landed\n",t->content[4],t->content[0]);
    sleep(2);//simulating landing
    sleep(3);//simulating de boarding
    temp.type=t->content[2]+10;
    temp.content[0]=t->content[0];
    temp.content[1]=t->content[1];
    temp.content[2]=t->content[2];
    temp.content[3]=t->content[3];
    //printf("Thread:%d Plane:%d informing ATC that plane has landed \n", t->content[4],t->content[0]);
    if(msgsnd(msgid, (void *)&temp, sizeof(temp), 0) == -1)//informing ATC that the plane has successfully completed takeoff
    {
    	printf("error in sending message ");
        exit(1);
    }
    printf("Plane %d has completed deboarding/unloading and landed on Runway No. %d of Airport No. %d.\n", t->content[0],runway+1,t->content[2]);
    
    pthread_mutex_unlock(&runways[runway]);//unlocking runway
    pthread_mutex_lock(&ActiveThreads);
    threadsActive--;//updating active threads count
    pthread_mutex_unlock(&ActiveThreads);
    pthread_exit(0);
}
int main()
{
    int AirportNumber;
    printf("Enter Airport Number:");
    scanf("%d", &AirportNumber);
    printf("Enter number of Runways:");
    scanf("%d", &NumberOfRunways);
    //printf("%d", NumberOfRunways);
    char buffer[100];
    printf("Enter loadCapacity of Runways (give as a space separated list in a single line):");
     // Flush input buffer
    int c;  
    while ((c = getchar()) != '\n' && c != EOF);

    scanf("%99[^\n]", buffer);
    //printf("%s \n", buffer);
    int start=0;
    int curr=0;
    char temp[10];
    for(int i=0;i<NumberOfRunways;i++)
    {
        while(curr<100 && buffer[curr]!=' ' && buffer[curr]!='\n')
        {
            curr++;
        }
        //printf("Start: %d Curr:%d \n",start,curr);
        strncpy(temp,buffer+start,(curr-start));
        //printf("%s ", temp);
        RunwayCapacity[i]=atoi(temp);
        if(start!=0 && curr!=100)
            RunwayCapacity[i]=RunwayCapacity[i]/10;
        //printf("%d \n",RunwayCapacity[i]);
        curr++;
        start=curr;
    }
    RunwayCapacity[NumberOfRunways]=15000;//Backup runway
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
    pthread_t threads[NumberOfRunways];
    while(1)
    {   
        //printf("Inside loop \n");
        Message temp;
        if (msgrcv(msgid, (void *)&temp, sizeof(temp), 50+AirportNumber, 0) == -1)
        {
    	   printf("error in receiving message\n");
            exit(1);
        }
       // printf("Recieved message \n");
        if(temp.content[0]==-1)//checking for termination
        {
            while(threadsActive!=0)//waiting for all threads to finish
            {
                temp.content[0]=temp.content[0];//dummy operation
            }
            break;
        }
        if(temp.content[1]==AirportNumber)//current airport is the departure airport
        {   
            //printf("Departure airport \n");
            pthread_mutex_lock(&ActiveThreads);
            Data* d = malloc(sizeof(Data));
            d->content[0]=temp.content[0];
            d->content[1]=temp.content[1];
            d->content[2]=temp.content[2];
            d->content[3]=temp.content[3];
            d->AirportNumber=AirportNumber;
            /*for(int i=0;i<4;i++)
            {
                printf("%d ",d->content[i]);
            }
            printf("\n");*/
            //printf("%d", &d);
            d->content[4]=threadsActive;
            pthread_create(&threads[threadsActive], NULL, departure, (void*)d);
            threadsActive++;
            pthread_mutex_unlock(&ActiveThreads);

        }
        else//current airport is the arrival airport
        {   
            //printf("Arrival airport \n");
            pthread_mutex_lock(&ActiveThreads);
            Data* d = malloc(sizeof(Data));
            d->content[0]=temp.content[0];
            d->content[1]=temp.content[1];
            d->content[2]=temp.content[2];
            d->content[3]=temp.content[3];
            d->AirportNumber=AirportNumber;
            /*for(int i=0;i<4;i++)
            {
                printf("%d ",d->content[i]);
            }
            printf("\n");*/
            //printf("%d", &d);
            d->content[4]=threadsActive;
            pthread_create(&threads[threadsActive], NULL, arrival, (void*)d);
            threadsActive++;
            pthread_mutex_unlock(&ActiveThreads);
        }
    }
    Message t;
    t.type=32;
    if(msgsnd(msgid, (void *)&t, sizeof(t), 0) == -1)//informing ATC that all planes have landed and the airport is termination
    {
    	printf("error in sending message ");
        exit(1);
    }
}