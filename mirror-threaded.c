#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>

#define BUFSIZE 3333
#define PORT 5000
#define l1 1
#define l2 4
#define l3 5
#define l4 6
#define QUEUE 3

void ledcontrol(int in)
{
	int led1,led2,led3, led4;
	int state1,state2,state3,state4;
	led1 = in / 2;
	state1 = in % 2;
	led2 = led1 / 2;
	state2 = led1 % 2;
	led3 = led2 / 2;
	state3 = led2 % 2;
	led4 = led3 / 2;
	state4 = led3 % 2;
	
	if(in < 16) {
		digitalWrite(l1 ,state4);
		digitalWrite(l2 ,state3);
		digitalWrite(l3 ,state2);
		digitalWrite(l4 ,state1);

	} else {
		digitalWrite(l1 ,0);
		digitalWrite(l2 ,0);
		digitalWrite(l3 ,0);
		digitalWrite(l4 ,0);
	}
}
void *anzeige(void* arg)
{
	wiringPiSetup();
	pinMode(l1 ,OUTPUT);
	pinMode(l2 ,OUTPUT);
	pinMode(l3 ,OUTPUT);
	pinMode(l4 ,OUTPUT);
	int clientfd = *(int *)arg; 
	char inbuffer[BUFSIZE];
	char outbuffer[BUFSIZE];
	
	while (1)
	{
		int count = read(clientfd, inbuffer, sizeof(inbuffer));// count: Anzahl der gelesenen Bytes
		int in = atoi(inbuffer);  // atoi --> string in int
		
		ledcontrol(in);
		
		
		write(clientfd, outbuffer, count);	// Sendebuffer in Clientsocket schreiben
		
		
		if (inbuffer[0] == 'q'||inbuffer[0] == 'Q')
		{
			close(clientfd);
			return NULL;
		}
	}	
}


int main()
{
	
	int server_socket, rec_socket;
	unsigned int len;
	struct sockaddr_in serverinfo, clientinfo;

	// Serversocket konfigurieren
	server_socket = socket(AF_INET, SOCK_STREAM,0); // TCP
	serverinfo.sin_family = AF_INET; // IPv4
	// hoert auf allen Interaces: 0.0.0.0 bzw. :::
	serverinfo.sin_addr.s_addr = htonl(INADDR_ANY);
	serverinfo.sin_port = htons(PORT);
	
	// Verbinde Socket mit der IP-Adresse und Port
	if(bind(server_socket,(struct sockaddr *)&serverinfo,sizeof(serverinfo)) != 0) 
	{
		printf("Fehler beim Socket\n");
		return 1; // Rueckgabe Fehlercode
	}
	
	listen(server_socket, QUEUE); // Server wartet auf connect vom Client
	
	// Endlosschleife Server zur Abarbeitung der Client-Anfragen
	while(1)
	{	
		printf("Server wartet...\n");
		// Verbindung vom Client eingetroffen
		rec_socket = accept(server_socket,(struct sockaddr *)&clientinfo, &len);
		printf("Verbindung vom %s:%d\n", inet_ntoa(clientinfo.sin_addr), ntohs(clientinfo.sin_port));
		
		pthread_t child; // Thread Struktur
		// Thread mit Funktion mirror(rec_socket) erzeugen
		if(pthread_create(&child, NULL, anzeige, &rec_socket) != 0) 
		{
			perror("child error"); // Fehlerfall: Abbruch
			return 1;
		}
		else // Kind erzeugt:
		{ 
			printf("Abgekoppelt!\n");
			pthread_detach(child); // abgekoppelt vom Hauptprozess
		}  
	}
	return 0;
}
