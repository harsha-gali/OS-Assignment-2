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
    char Decision='N';
    while(Decision!='Y')
    {
        printf("Do you want the Air Traffic Control System to terminate?(Y for Yes and N for No)");
        sleep(0.5);
        scanf(" %c",&Decision);
    }
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
    Message temp;
    temp.type=31;
    if(msgsnd(msgid, (void *)&temp, sizeof(temp), 0) == -1)
    {
    	printf("error in sending message ");
        exit(1);
    }
}