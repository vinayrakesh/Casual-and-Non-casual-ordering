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

#define PORT 9000
#define IP "250.0.0.25"

using namespace std;

void *sender(void *);
void *receiver(void *);

int mul_socket, point_socket;
int log_clock, process, point, msg, send_clock_time, sender_port;
struct sockaddr_in point_address, port_master, mul_address, mul_r_address;
struct ip_mreq mulreq;

char * msg_input;
char msg_broadcast[256];

struct msg_buffer{
	char msg[256];
	int buffer_array[50];
};

struct msg_multicast{
	int pid;
	char rv_msg[256];
	int array[10];
};

int arr1[3] = {0,0,0};
int n = 3;

struct msg_multicast msg_array[50];

int main(int argc, char *argv[])
{
	u_int on = 1;
	char msg1[256],msg2[256],msg3[256],msg4[256],buff1[256];
	int a1, a2, a3;
	int initial_val_time, time_drift, send_time_drift_time;
	unsigned int rank = atoi(argv[1]);
	msg_input = argv[2];
	pthread_t send_th, recv_th;
	
	//Lets Create multicast sockets
	mul_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(mul_socket < 0)
	{
		cout << "Socket Creation failed\n";
		return 0;
	}
	
	//Now let us setup destination address	
	memset((char*) &mul_address, 0, sizeof(mul_address));
	mul_address.sin_family = AF_INET;
	mul_address.sin_addr.s_addr = inet_addr(IP);
	mul_address.sin_port = htons(PORT);

	//Similarly setup the destination address at receiver side
	memset((char*) &mul_r_address, 0, sizeof(mul_r_address));
	mul_r_address.sin_family = AF_INET;
	mul_r_address.sin_addr.s_addr =htonl(INADDR_ANY);
	mul_r_address.sin_port = htons(PORT);

	//Lets us allow the mutliple sockets to use  same port number using setsockopt function
	if(setsockopt(mul_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	{
		cout << "Failed to use same port by multiple sockets" << endl;
		return 0;
	}

	//bind to destination address
	msg = bind(mul_socket, (struct sockaddr *) &mul_r_address, sizeof(mul_r_address));
	if(msg < 0)
	{
		cout << "Binding failed\n";
		return 0;
	}

	//use setsockopt() to join the multicast group
	mulreq.imr_multiaddr.s_addr = inet_addr(IP);
	mulreq.imr_interface.s_addr = htonl(INADDR_ANY);
	
	if(setsockopt(mul_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mulreq, sizeof(mulreq)) < 0)
	{
		cout << "Error in setting up" << endl;
		return 0;
	}
	
	//Now let us create point to point socket
	point_socket = socket(AF_INET, SOCK_DGRAM, 0);
	
	//set destination address
	point_address.sin_family = AF_INET;
	point_address.sin_addr.s_addr = INADDR_ANY;
	point_address.sin_port = htons(INADDR_ANY);

	//Request to use same address
	if(setsockopt(point_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
	{
		cout << "Cannot reuse the address\n";
		return 0;
	}

	//now Bind point to point socket
	point = bind(point_socket, (struct sockaddr *) &point_address, sizeof(point_address));
	if(point < 0)
	{
		cout << "Point to Point binding is failed" << endl;
		return 0;
	}
	

	struct timeval time_out;
	time_out.tv_sec = 5;
	time_out.tv_usec = 0;
	if(setsockopt(point_socket,SOL_SOCKET,SO_RCVTIMEO,(char *)&time_out,sizeof(time_out))<0)
	{
	  cout << "Timeout eror\n";  
	}
	
	//Create threads for sender and reciever for communication
	
	if(pthread_create(&send_th,NULL,sender,&rank)<0)
	{
	  cout << "Failed to create a thread for sender" << endl; 
	  return 1;
	}
	if(pthread_create(&recv_th,NULL,receiver,&rank)<0)
	{
	  cout << "Failed to create a thread for sender" << endl;
	  return 1;
	}
	pthread_join(send_th,NULL);

	pthread_join(recv_th,NULL);
	return 0;
}

void *sender(void *r)//thread for sending the msgs to all the processes
{
	unsigned int rank1 = *((unsigned int *) r);
	int a1= 0,a2=0,a3=0;
	int process;
	cout << "Rank: " << rank1 << endl;
	cout << "Message Input: " << msg_input << endl;
	while(1)
	{
	for (int i = 0; i < 3; i++)
	{
	   if(i == rank1)
		{
		if(i == 0)
		{
		a1++;
		process = 1;
		cout << "Process: " << process;
		arr1[i] = a1;
		}
		if(i == 1)
		{
		a2++;
		process = 2;
		cout << "Process: " << process;
		arr1[i] = a2;
		}
		if(i == 2)
		{
		a3++;
		process = 3;
		cout << "Process: " << process;
 		arr1[i] = a3;
		}
		}
	}
	
		char buff1[256];
		sprintf(buff1,"%d,%d,%d,%d,%s", process, arr1[0], arr1[1], arr1[2], msg_input);
		cout << "Message: " << buff1 << endl;
		int send1 = sendto(mul_socket, buff1, sizeof(buff1), 0, (struct sockaddr *) &mul_address, sizeof(mul_address));
		sleep(10);
	}
	return 0;
}

//Now lets create threads for sending messages to all other processes.
void *receiver(void *r1)
{
	unsigned int rank2 = *((unsigned int *) r1);
	char buff2[256];
	int count = 0;
	int pid;
	char rv_msg[256];
	msg_buffer msg_buf;
	int array[3];
	int num = 0;
	int var_array1[3] = {0,0,0}, var_array2[3] = {0,0,0}, var_array3[3] = {0,0,0};
	char *var, *msg, *arr2[3];
	socklen_t addr=sizeof(mul_r_address);
	
	while(recvfrom(mul_socket, buff2, sizeof(buff2), 0, (struct sockaddr *) &mul_r_address, &addr) >= 0)
	{
		count++;
		cout << endl ;
		cout << "CASUAL ORDERING Implemetation "<< endl;
		cout << "Message received  " << buff2 << endl;
		cout << "Count: " << count << endl;
		msg = strtok(buff2, ",");
		if(msg != NULL)
		{
			pid = atoi(msg);
			cout << "Processid: " << pid << endl;
			msg = strtok(NULL, ",");
			array[0] = atoi(msg);
			msg = strtok(NULL, ",");
			array[1] = atoi(msg);
			msg = strtok(NULL, ",");
			array[2] = atoi(msg);			
			msg = strtok(NULL, " ,");
			strcpy(rv_msg, msg);
				
		}
		if(rank2 == 0)
		{
			if (pid == 1)
			{
				cout <<"Trying to receive message from Process 2 " << endl;
				if (var_array1[1] > array[1])
				{
					cout << "Message has been buffered" << endl;
					msg_buf.buffer_array[0] = array[0];
					msg_buf.buffer_array[1] = array[1];
					msg_buf.buffer_array[2] = array[2];
				}
				else
				{
					var_array1[0] = var_array1[0] + array[0];
					var_array1[1] = var_array1[1] + array[1];
				var_array1[2] = var_array1[2] + array[2];
					cout << endl ;
					cout << "Message Delivered as expected" << endl;
					cout <<"The Vector value " << var_array1[0] << "," << var_array1[1] <<"," << var_array1[2] << endl;
				}
			}
			else if (pid == 2)
			{
				cout <<"Trying to receive message from Process 3 " << endl;
				if (var_array1[2] > array[2])
				{
					cout << "Message has been buffed" << endl;
					msg_buf.buffer_array[0] = array[0];
					msg_buf.buffer_array[1] = array[1];
					msg_buf.buffer_array[2] = array[2];
				}
				else
				{
					var_array1[0] = var_array1[0] + array[0];
					var_array1[1] = var_array1[1] + array[1];
					var_array1[2] = var_array1[2] + array[2];
					cout << endl ;
					cout << "Message recieved as expected" << endl;
					cout <<"The vector value: " << var_array1[0] << "," << var_array1[1] <<"," << var_array1[2] << endl;
				}
			}
		}

		if(rank2 == 1)
		{
			if (pid == 0)
			{
				cout <<"Receiving Message from Process 1: " << endl;
				if (var_array1[0] > array[0])
				{
					cout << "Message has been buffered" << endl;
					msg_buf.buffer_array[0] = array[0];
					msg_buf.buffer_array[1] = array[1];
					msg_buf.buffer_array[2] = array[2];
				}
				else
				{
					var_array1[0] = var_array1[0] + array[0];
					var_array1[1] = var_array1[1] + array[1];
					var_array1[2] = var_array1[2] + array[2];
					cout << endl ;
					cout << "Message recived as expected" << endl;
					cout <<"The vector value " << var_array1[0] << "," << var_array1[1] <<"," << var_array1[2] << endl;
				}
			}
			else if (pid == 2)
			{
				cout <<"Receiving Message from Process 3: " << endl;
				if (var_array1[2] > array[2])
				{
					cout << "Message is buffered" << endl;
					msg_buf.buffer_array[0] = array[0];
					msg_buf.buffer_array[1] = array[1];
					msg_buf.buffer_array[2] = array[2];
				}
				else
				{
					var_array1[0] = var_array1[0] + array[0];
					var_array1[1] = var_array1[1] + array[1];
					var_array1[2] = var_array1[2] + array[2];
					cout << endl ;
					cout << "Message recived as expected" << endl;
					cout <<"The vector value is " << var_array1[0] << "," << var_array1[1] <<"," << var_array1[2] << endl;
				}
			}
		}

		if(rank2 == 2)
		{
			if (pid == 0)
			{
				cout <<"Receiving Message from Process 1: " << endl;
				if (var_array1[0] > array[0])
				{
					cout << "Message has been buffed" << endl;
					msg_buf.buffer_array[0] = array[0];
					msg_buf.buffer_array[1] = array[1];
					msg_buf.buffer_array[2] = array[2];
				}
				else
				{
					var_array1[0] = var_array1[0] + array[0];
					var_array1[1] = var_array1[1] + array[1];
					var_array1[2] = var_array1[2] + array[2];
					cout << endl ;
					cout << "Message Delivered from source" << endl;
					cout <<"The vector value is" << var_array1[0] << "," << var_array1[1] <<"," << var_array1[2] << endl;
				}
			}
			else if (pid == 1)
			{
				cout <<"Receiving Message from Process 2: " << endl;
				if (var_array1[1] > array[1])
				{
					cout << "Message has been buffed" << endl;
					msg_buf.buffer_array[0] = array[0];
					msg_buf.buffer_array[1] = array[1];
					msg_buf.buffer_array[2] = array[2];
				}
				else
				{
					var_array1[0] = var_array1[0] + array[0];
					var_array1[1] = var_array1[1] + array[1];
					var_array1[2] = var_array1[2] + array[2];
					cout << endl ;
					cout << "Message Delivered from other source" << endl;
					cout <<"The vector value is : " << var_array1[0] << "," << var_array1[1] <<"," << var_array1[2] << endl;
				}
			}
		}			
				
		sleep(10);
		bzero(buff2, 256);
	}
return 0;		
}


	
