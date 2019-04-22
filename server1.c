/******************************************************************************
            COP 6611
    Programming Homework 5: Sockets(server program)
                        by
                        Lalithabhinaya Mallu -U63881688
*******************************************************************************/
#include <sys/types.h> 
#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include<semaphore.h>
#include <pthread.h>

#define IP_ADDR "131.247.3.8"  
#define PORT_NUM 1050
#define BUFFLEN 15
#define THREADS 3 

struct shared_dat
   {
   char buf[BUFFLEN];     /* shared variable to store message*/
   };

struct shared_dat  *counter;
sem_t writerSem;          /* semaphore declaration*/ 
void * writer_thread(void *arg);

/****************************************************************
Main Method
****************************************************************/
int  main(int argc, char *argv[])
{
	int len;
	int sock,newsock;
	int on = 1;
	struct sockaddr_in  server,remote;
	int remotelength;
	int fd;
	int p_id,i=0;
	
	
	/* socket() call generates one socket for ipc */
	if ( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Opening Stream Socket");
		exit(1);		
	}
	

	 /* This should free the socket right away */
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0)
          {
                perror("Stream Socket option set failed.");
				close(sock);
                exit(1);
				
            /* some error has occurred, etc */
          }

	/* address naming for the internet domain socket */
	    server.sin_family = AF_INET;
      	server.sin_port = htons(PORT_NUM);
		server.sin_addr.s_addr = INADDR_ANY;
		
    /* Bind */
	if (bind(sock, (struct sockaddr *)&server,sizeof(server)) < 0)
	{
		perror("Error while binding socket to address");
		close(sock);
		exit(1);
		
	}	
    
   /*listen for connections */
    
	if(listen(sock,3) < 0)
	{
		perror("Error while listening");
		close(sock);
		exit(1);
	}
    
   /* Accept*/
 
        sem_init(&writerSem,0,1); 	
		pthread_t writer[THREADS];  /* process id for writer */
		pthread_attr_t attr[1];     /* attribute pointer array */

		counter = (struct shared_dat *) malloc(sizeof(struct shared_dat));
		fflush(stdout);
        /* Required to schedule thread independently.Otherwise use NULL in place of attr. */
                      
         pthread_attr_init(&attr[0]);
         pthread_attr_setscope(&attr[0], PTHREAD_SCOPE_SYSTEM);  /* system-wide contention */ 
	    remotelength = sizeof(remote);
		
		while(i<THREADS)
		{

         if ((newsock = accept(sock,(struct sockaddr *)&remote, &remotelength)) < 0) 
	      {
		    perror("Error while binding socket to address");
			close(sock);
		    exit(1);
			
		   }

		 else
		   {
            printf("connection accepted\n");

		    /* Create the threads */
             p_id=pthread_create(&writer[i], &attr[0],writer_thread, (void*) (intptr_t) newsock);

			if(p_id < 0)
			{
			   perror("Error while creating the process");
		       close(newsock);
			   exit(1);
			}

			/*else if(p_id == 0)
			{
			   close(sock);
			}*/
           }
		   
		   i++;
		}
        
         /* Wait for the threads to finish */
	      for(i=0;i<THREADS;i++)
		  {
		    pthread_join(writer[i], NULL);
		  }

   close(sock);
   sem_destroy(&writerSem);
   close(newsock);
   exit(0);
}


/****************************************************************
Writer_Thread
****************************************************************/

void * writer_thread(void *arg)
{
   int n;
   int sock1 = (intptr_t)(arg);
   
   sem_wait(&writerSem);                              /*Acquire the semaphore */
   n = recv(sock1,counter->buf,15,0);                 /*message length should be no more than 15 characters*/
   printf("recieved message from client\n");
   if (n < 0) 
   {
	   perror("ERROR while receiving from  from client");
	   close(sock1);
	   
   }
   
   sleep(2);                                           /* sleep for 2 seconds*/

   printf("sending message to client\n");
   n = send(sock1,counter->buf,15,0);
   if (n < 0)
   {
	   perror("ERROR while sending to client");
	   close(sock1);
    }
	       
   printf("Here is the message: %s\n",counter->buf); 
   close(sock1);
   fflush(stdout);
   sem_post(&writerSem);                               /* Release the semaphore*/
    
}

