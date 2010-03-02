/*
 throttled.cpp
 Copyright (C) 2010 quest and lws
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "throttled.h"

/*
 Main function - start program here.
*/
int main(int argc, char** argv)
{  
  std::list<threadData*>::const_iterator iter;

  // Open the console for syslog()
  openlog(argv[0], LOG_CONS | LOG_PID, LOG_DAEMON);
  
  // Check commandline args
  checkccargs(argc, argv);

  // Make sure euid is root
  if (geteuid() != 0) {
    fprintf(stderr, "You must run this as root - use sudo\n");
    exit(1);
  }

  // set application priority to the value specified in throttled.h
  setpriority(PRIO_PROCESS, 0, NICEVALUE);
  printf("Priority has been set to %i\n", getpriority(PRIO_PROCESS, 0));

  // Signals so we can quit gracefully
  signal(SIGHUP, sigquitproc);
  signal(SIGINT, sigquitproc);
  signal(SIGKILL, sigquitproc);
  signal(SIGTERM, sigquitproc);
  signal(SIGQUIT, sigquitproc);
  signal(SIGUSR1, sigspeedchangeproc);
  signal(SIGUSR2, sigspeedchangeproc);

  // Create and bind the sockets
  for( iter = threads.begin(); iter != threads.end(); iter++)
    makesocket(*iter);

  printf("Waiting for data...\n");

  // Throw the app into the background
  daemon(0,1);

  queueMut = (pthread_mutex_t *)malloc(sizeof (pthread_mutex_t));
  pthread_mutex_init (queueMut, NULL);
  sendMut = (pthread_mutex_t *)malloc(sizeof (pthread_mutex_t));
  pthread_mutex_init (sendMut, NULL);
  aQueueNotEmpty = (pthread_cond_t *)malloc(sizeof (pthread_cond_t));
  pthread_cond_init (aQueueNotEmpty, NULL);

  // make the threads
  for( iter = threads.begin(); iter != threads.end(); iter++)
    makerecievethread(*iter); 

  // Make sure per-thread stack is big enough for threads
  pthread_attr_init(&sendpattr);
  pthread_attr_getstacksize(&sendpattr, &sendssize);
 
  if (sendssize < BUFSIZE + 1024) {
    sendssize = BUFSIZE + 1024;
    pthread_attr_setstacksize(&sendpattr, sendssize);
  }

  pthread_create(&sendid, &sendpattr, sendpackets, NULL);

  //join the threads so we can quit
  jointhreads();

  //clean up some of our mess
  for( iter = threads.begin(); iter != threads.end(); iter++)
  {
    close((*iter)->sockid);
    delete *iter;
  }

  syslog(LOG_NOTICE, "Program Terminated");
  closelog();

  return 0;
}

/*
 Function to check commandline args
*/
void checkccargs(int argc, char** argv)
{
  //initialize variables to be used in getopt
  threadData *curport = NULL;
  std::list<threadData*>::iterator iter;
  int c;
  int numofports = 0;

  //actual getopt call, this should be straightfoward
  while ((c = getopt(argc, argv, "vhTs:r:i:d:w:")) != EOF) {
    switch (c) {
      case 'v':
        printf("throttled %s\n\nCopyright (C) 2008 quest, lws and step76.\n", THROTTLED_VERSION);
        exit(1);
        break;
      case 'h':
        usage(argv[0]);
        exit(1);
        break;        
      case 's':
        maxrate = atoi(optarg);
        if (maxrate <= 0) {
          fprintf(stderr, "Max bytes/sec must be greater than 0... Exiting...\n");
          exit(1);
        }
        break;
      case 'r':
        rulenum = atoi(optarg);
        if (rulenum <= 0 || rulenum > 65535) {
          fprintf(stderr, "IPFW rule number must be between 1 and 65535... Exiting...\n");
          exit(1);
        }
        break;
      case 'i':
        speedchange = atoi(optarg);
        if (speedchange <=0)
        {
          fprintf(stderr, "Speed change must be greater than 0... Exiting...\n");
          exit(1);
        }
        break;
      case 'T':
        itunesfix = !itunesfix;
        printf("iTunes TTL fix has been %s!\n", itunesfix ? "enabled" : "disabled");
        break;
      case 'd':
        if( curport && !curport->weight )
        {
          fprintf(stderr, "You must specify a weight for the last divert port before specifying another divert port... Exiting...\n");
          exit(1);
        }

        numofports++;
        curport = new threadData;
        curport->bindport = atoi(optarg);
        curport->weight = 0;
        if (curport->bindport <= 0 || curport->bindport > 65535) {
          fprintf(stderr, "Divert port must be between 1 and 65535... Exiting...\n");
          exit(1);
        }
        break;

      case 'w':
        if( atoi(optarg) < 1 || atoi(optarg) > 100)
        {
          fprintf(stderr, "Weight must be between 1 and 100... Exiting...\n");
          exit(1);
        }
        if( curport)
        {
          if( ! curport->weight )
          {
            curport->weight = atoi(optarg);
            curport->flowId = threads.size();
            threads.push_back(curport);
          }
          else
          {
            fprintf(stderr, "You may not specify two weights for the same port... Exiting...\n");
            exit(1);
          }
        }
        else
        {
          fprintf(stderr, "You must specify a divert port before specifying a weight... Exiting...\n");
          exit(1);
        }
        break;
        
      default:
        usage(argv[0]);
        exit(1); 
    }
  }

  // Maxrate is required, test for it here.
  if (maxrate == 0) {
    usage(argv[0]);
    exit(1);
  }

  // IPFW rule number is required, test for it here.
  if (rulenum == 0) {
    usage(argv[0]);
    exit(1);
  }

  if( numofports == 0 )
  {
    numofports++;
    curport = new threadData;
    curport->bindport = 17777;
    curport->weight = 1;
    curport->flowId = threads.size();
    threads.push_back(curport);      
  }

  if( curport->weight == 0 )
  {
    if( numofports > 1 )
    {
      fprintf(stderr, "You did not specify a weight for the last divert port... Exiting...\n");
      exit(1);
    } else {
      curport->weight = 1;
      curport->flowId = threads.size();
      threads.push_back(curport);
    }
  }

  maxsendratio = (1000000000 / maxrate);

  printf("Max bytes/sec has been set to %li\n", maxrate);
  printf("Speed change set to %li bytes\n", speedchange);   
}

/*
 Function for displaying program usage
*/
void usage(char *appname) {
  printf( "usage: %s [-Thv] -s speed -r rule [-i increment] [-d port] [-w weight]\n"
      "-s speed\tMax speed in bytes/second (required)\n"
      "-r rule\t\tIPFW rule number to remove when quit (required)\n" 
      "-i increment\tAmount to change the throttle in bytes/sec (USR1 - Down | USR2 - Up)\n"
      "-T\t\tEnable iTunes TTL fix\n"
      "-h\t\tThis help screen\n"
      "-v\t\tVersion information\n"
      "-d port\t\tDivert port (optional, may specify more than one)\n"
      "\t-w weight\tWeight for the divert port specified prior to this option.\n"
      , appname);
}

/*
 Function to delete divert rules, which are bound to throttled, on exit.
 This was way slicker in 0.3.2 but didn't work on FreeBSD 5.x
 I contacted Luigi (the ipfw maintainer) and he told me to do it this way.
 Arg.
*/
void deleterules()
{
  char buffer[30];
  int n;
  n = sprintf(buffer, "/sbin/ipfw del %i", rulenum);
  system(buffer);
}

/*
 Function to create and bind a socket
 */
void makesocket (struct threadData *thedata)
{
  // Creating a raw divert socket
  printf("Creating a socket for divert port %i\n", thedata->bindport);
  thedata->sockid = socket(AF_INET, SOCK_RAW, IPPROTO_DIVERT);
  
  // Make sure create didn't error out
  if (thedata->sockid == -1) {
    fprintf(stderr, "Failure creating divert socket... Exiting...\n");
    exit(1);
  }
  
  // Setup for binding the socket
  thedata->sockport.sin_family = AF_INET;
  thedata->sockport.sin_port = htons(thedata->bindport);
  thedata->sockport.sin_addr.s_addr = 0;
  memset(&(thedata->sockport.sin_zero), '\0', 8);
  
  thedata->bindid = bind(thedata->sockid, (struct sockaddr*)&(thedata->sockport), sizeof(struct sockaddr_in));
  
  // Make sure bind didn't error out
  if (thedata->bindid != 0) {
    close (thedata->sockid);
    fprintf(stderr, "Failure binding to port %i... Exiting...\n", thedata->bindport);
    exit(1);
  }
}

/*
 Function to spawn a new thread for the divert socket.
 This doesn't do much now, but in the past it initilized some values, and maybe will again in the future.
*/
void makerecievethread(threadData *thedata)
{
  // Create the pthreads

  // inizialize flow
  PacketQueue.init(thedata->flowId, (50 / (threads.size() + 1)), thedata->weight);

  // Make sure per-thread stack is big enough for threads
  pthread_attr_init(&(thedata->pattr));
  pthread_attr_getstacksize(&(thedata->pattr), &(thedata->ssize));

  if (thedata->ssize < BUFSIZE + 1024) {
    thedata->ssize = BUFSIZE + 1024;
    pthread_attr_setstacksize(&(thedata->pattr), thedata->ssize);
  }

  pthread_create(&(thedata->receiveid), &(thedata->pattr), receivepackets, (void *)thedata);
}

/*
 Function to join the thread back to the mother
 */
void jointhreads()
{
  threadData* thedata;
  std::list<threadData*>::const_iterator iter;
  for( iter = threads.begin(); iter != threads.end(); iter++)
  {
    thedata = *iter;
    // Join the pthreads (this is when the work begins)
    pthread_join(thedata->receiveid, NULL);
  }
  
  //clean up the mutexes and such for sendpackets.
  pthread_join(sendid, NULL);
  pthread_mutex_destroy(queueMut);
  free (queueMut);
  pthread_mutex_destroy(sendMut);
  free (sendMut);
  pthread_cond_destroy(aQueueNotEmpty);
  free (aQueueNotEmpty);
}

/*
 Allows app to quit gracefully on quit signal
 */
void sigquitproc(int signal)
{
  keeplooping = false;
  deleterules();
}

/*
 Allows you to change the speed of your throttle
 */

void sigspeedchangeproc(int signal)
{
  switch(signal) {
    case SIGUSR1:
      if ((maxrate - speedchange) > 1024)
        maxrate = maxrate - speedchange;
      break;
    case SIGUSR2:
      maxrate = maxrate + speedchange;
      break;
  }
  maxsendratio = (1000000000 / maxrate);
}

/*
 Function to recieve the packets -- queues packets up and sends off ACK or local packets with instant priority.
 *WARNING: This runs multiple times, once for each divert port.
 */
void* receivepackets(void *dataarg)
{
  pPacket* packettmp = NULL;
  threadData *ourdata = (threadData *)dataarg;
  unsigned char packet[BUFSIZE];
  struct sockaddr_in sendtowho;
  fd_set selectpoll, t_selectpoll;
  struct timeval s_timeout;
  long sizerecv;
  socklen_t sockaddrsize = sizeof(struct sockaddr_in);
  
  // Initialize variables for loop
  FD_ZERO(&selectpoll);
  FD_SET(ourdata->sockid, &selectpoll);
  t_selectpoll = selectpoll;
  s_timeout.tv_sec = 5;
  s_timeout.tv_usec = 0;
  
  while (keeplooping) {
    if (select((ourdata->sockid)+1, &t_selectpoll, NULL, NULL, &s_timeout) > 0) {

      sizerecv = recvfrom(ourdata->sockid, &packet, BUFSIZE, 0, (struct sockaddr *)&sendtowho, &sockaddrsize);
      
      if ((((allheaders*)packet)->ipheader.ip_p == IPPROTO_TCP) && itunesfix) {
        if (((allheaders*)packet)->tcpheader.th_sport == 3689)
          ((allheaders*)packet)->ipheader.ip_ttl = 64;
      }
      
      packettmp = new pPacket(ourdata->flowId, ourdata->sockid, packet, sizerecv, (struct sockaddr*)&sendtowho, sockaddrsize);
      pthread_mutex_lock(queueMut); {
        if (!PacketQueue.enque(packettmp)) {
          delete packettmp->packet;
          delete packettmp;
        }
      } pthread_mutex_unlock(queueMut);
      pthread_cond_signal(aQueueNotEmpty);
    } else {
      t_selectpoll = selectpoll;
      s_timeout.tv_sec = 5;
      s_timeout.tv_usec = 0;
    }
  }
  
  pthread_cond_signal(aQueueNotEmpty); // So our consumer doesn't get blocked forever...
  
  return NULL;
}

/*
 Function to send the queued packets, at a throttle'd rate
 */
void* sendpackets(void *dataarg)
{
  pPacket* aPacket = NULL;
  unsigned long long sleeptime;
  struct timespec nanosleeptime;
  long sizesent = 0;

  pthread_mutex_lock(sendMut); {
    while (keeplooping) {
      pthread_mutex_lock(queueMut); {
        aPacket = PacketQueue.deque();
      } pthread_mutex_unlock(queueMut); 
      if (aPacket != NULL) {
        sizesent = sendto(aPacket->sockid, aPacket->packet, aPacket->size, 0, (struct sockaddr *)&(aPacket->toaddr), aPacket->fromsize);

        delete aPacket->packet;
        delete aPacket;
        aPacket = NULL;

        //Where the actual throttling is done.
        if (sizesent > 0) {
          sleeptime = sizesent * maxsendratio;
          nanosleeptime.tv_sec = sleeptime / 1000000000;
          nanosleeptime.tv_nsec = sleeptime % 1000000000;
          nanosleep(&nanosleeptime, NULL);
        }
      } else {
        pthread_cond_wait(aQueueNotEmpty, sendMut);
      }
    }
  } pthread_mutex_unlock(sendMut);  

  return NULL;
}
