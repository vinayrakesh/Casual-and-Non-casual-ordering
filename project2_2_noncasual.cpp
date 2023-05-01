#include <iostream>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>

#define PORT 10000
#define IP "225.0.0.75"
using namespace std;

void *sender(void *);
void *receiver(void *);

int msg_socket, point_socket;
int log_clock, process, point, msg, send_clock_time, sender_port;
struct sockaddr_in point_address, port_master, mul_address, mul_r_address;
struct ip_mreq mulreq;
char * msg_input;
char msg_broadcast[256];

struct message_multicast{
	int pid;
	char rv_msg[256];
	int arr[3];
};

struct message_multicast msg_array[50];

int array1[3] = {0,0,0};
int n = 3;


int main(int argc, char *argv[])
{
	u_int on = 1;
	char msg1[256],msg2[256],msg3[256],msg4[256],buff1[256];
	int a1, a2, a3;
	int initial_val_time, time_drift, send_time_drift;
	unsigned int rank = atoi(argv[1]);
	msg_input = argv[2];
	pthread_t send_th, recv_th;
	

	//create multicast socket
	msg_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(msg_socket < 0)
	{
		cout << "Socket creation is failed please etry again" << endl;
		return 0;
	}
	
	//After socket creation lets setup destination addresses	
	memset((char*) &mul_address, 0, sizeof(mul_address));
	mul_address.sin_family = AF_INET;
	mul_address.sin_addr.s_addr = inet_addr(IP);
	mul_address.sin_port = htons(PORT);

	//Similarly setup destination address also
	memset((char*) &mul_r_address, 0, sizeof(mul_r_address));
	mul_r_address.sin_family = AF_INET;
	mul_r_address.sin_addr.s_addr =htonl(INADDR_ANY);
	mul_r_address.sin_port = htons(PORT);

	//Now after setting up addresses we need to allow multiple sockets to use same port number 10000
	if(setsockopt(msg_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	{
		cout << "Error ocured while allowing multiple sockets to use same port number Please try again" << endl;
		return 0;
	}

	//After allowing the sockets to use same port now lets bind the destination address
	msg = bind(msg_socket, (struct sockaddr *) &mul_r_address, sizeof(mul_r_address));
	if(msg < 0)
	{
		cout << "Binding failed please try again" << endl;
		return 0;
	}

	//After all the setup lets use setsockopt() function to join the multicast
	mulreq.imr_multiaddr.s_addr = inet_addr(IP);
	mulreq.imr_interface.s_addr = htonl(INADDR_ANY);
	
	if(setsockopt(msg_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mulreq, sizeof(mulreq)) < 0)
	{
		cout << "Error in setsockopt" << endl;
		return 0;
	}
	
	//create point to point socket
	point_socket = socket(AF_INET, SOCK_DGRAM, 0);
	
	//set destination address
	point_address.sin_family = AF_INET;
	point_address.sin_addr.s_addr = INADDR_ANY;
	point_address.sin_port = htons(INADDR_ANY);

	//Lets req to use same address
	if(setsockopt(point_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	{
		cout << "Failed to reuse the address" << endl;
		return 0;
	}

	//After that lets bind point to point sockets
	point = bind(point_socket, (struct sockaddr *) &point_address, sizeof(point_address));
	if(point < 0)
	{
		cout << "Cannot bind point to point socket" << endl;
		return 0;
	}
	

	struct timeval time_out;
	time_out.tv_sec = 5;
	time_out.tv_usec = 0;
	if(setsockopt(point_socket,SOL_SOCKET,SO_RCVTIMEO,(char *)&time_out,sizeof(time_out))<0)
	{
	  cout << "Timeout eror" << endl;  
	}

	
	//Create threads for sender and reciever
	if(pthread_create(&send_th,NULL,sender,&rank)<0)
	{
	  cout << "Failed to create a thread for sender\n"; 
	  return 1;
	}
	if(pthread_create(&recv_th,NULL,receiver,&rank)<0)
	{
	  cout << "Failed to create thread for receiver\n";
	  return 1;
	}
	pthread_join(send_th,NULL);
	pthread_join(recv_th,NULL);
	return 0;
}

//Now lets create thread for sending messages to all processes
void *sender(void *r)
{
	unsigned int rank1 = *((unsigned int *) r);
	int p1 = 0, p2 = 0, p3 = 0;
	int process;
	cout << "Rank: " << rank1 << endl;
	cout << "Message Input: " << msg_input <<"\n";
	while(1)
	{
		for (int i = 0; i < 3; i++)
		{
			if(i == rank1)
			{
				if(i == 0)
				{
					p1++;
					process = 1;
					cout << "For Process: " << process;
					array1[i] = p1;
				}
				if(i == 1)
				{
					p2++;
					process = 2;
					cout << "For Process: " << process;
					array1[i] = p2;
				}
				if(i == 2)
				{
					p3++;
					process = 3;
					cout << " For Process: " << process;
					array1[i] = p3;
				}
			}
		}
		//cout << process << endl;
		char buff1[256];
		sprintf(buff1,"%d,%d,%d,%d,%s", process, array1[0], array1[1], array1[2], msg_input);
		cout << "The Message is "<< buff1 << endl;
		int send1 = sendto(msg_socket, buff1, sizeof(buff1), 0, (struct sockaddr *) &mul_address, sizeof(mul_address));
		sleep(10);
	}
	return 0;
}

void *receiver(void *r1)//thread for receiving the messages from all the processes
{
	char buff2[256];
	int count = 0;
	int pid;
	char rv_msg[256];
	int arr[3];
	int num = 0;
	int var_array1[3] = {0,0,0}, var_array2[3] = {0,0,0}, var_array3[3] = {0,0,0};
	char *variable, *message, *arr2[3];
	//int pid, a, a2, a3;
	socklen_t addr=sizeof(mul_r_address);
	while(recvfrom(msg_socket, buff2, sizeof(buff2), 0, (struct sockaddr *) &mul_r_address, &addr) >= 0)
	{
		count++;
		cout << endl ;
		cout << "Before the implementation of casual Ordering " << endl;
		cout << "The Message received is "<< buff2 << endl;
		cout << "Count: " << count << endl;
				
		message = strtok(buff2, ",");
		
		if(message != NULL)
		{
			pid = atoi(message);
			cout << "Processid: " << pid << endl;
			message = strtok(NULL, ",");
			arr[0] = atoi(message);
			message = strtok(NULL, ",");
			arr[1] = atoi(message);
			message = strtok(NULL, ",");
			arr[2] = atoi(message);
			message = strtok(NULL, " ,");		
			strcpy(rv_msg, message);
			cout << endl ;
			cout << "The Message is "<< rv_msg << endl;
			cout << "The Vector value is :- " << arr[0] << "," << arr[1] <<"," << arr[2] << endl;
			
				
		}
		sleep(10);
		bzero(buff2, 256);
	}
	
return 0;	
}

