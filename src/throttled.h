/*
 throttled.h
 Copyright (C) 2010 Josh Ellithorpe (quest) and lws

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

#ifndef THROTTLED_H
#define THROTTLED_H

#define THROTTLED_VERSION "0.6.0"

// Global includes
#include <stdio.h>

// Include signals for linux and FreeBSD.
#ifdef __FreeBSD__
#include <sys/types.h>
#include <signal.h>
#endif
#ifdef __linux__
#include <sys/signal.h>
#endif

// More global includes.
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/resource.h>
#include <err.h>
#include <errno.h>
#include <sysexits.h>
#include <syslog.h>
#include <stdarg.h>
#include <list>
#include <string>

// Local includes
#include "packetqueue.h"
#include "wf2q+.h"

// Definitions for compiler variables
#define BUFSIZE 65535

// Define IPPROTO_DIVERT for linux.
#ifdef __linux__
#define IPPROTO_DIVERT 254
#endif

// Define your application priority (-20 through 20)
// FreeBSD crashes hard at -20... make sure to watch out!
#if defined(__FreeBSD__)
#define NICEVALUE 0
#else
#define NICEVALUE -19
#endif

// All packet header info
struct allheaders {
  struct ip ipheader;
  union {
    struct tcphdr tcpheader;
    struct udphdr udpheader;
  };
};

// Putting all our data in a struct so it can be easily located
typedef struct threadData threadData;
struct threadData
{
  int sockid, bindid, flowId;
  struct sockaddr_in sockport;
  struct in_addr addr;
  int bindport;
  int weight;
          
  pthread_t receiveid;
  pthread_attr_t pattr;
  size_t ssize;
};

// Global variables
char keeplooping = true;
bool itunesfix = false;
long maxrate = 0, speedchange = 1024;
int rulenum = 0;
long maxsendratio = 0;
WF2Q PacketQueue;

std::list<threadData*> threads;

pthread_t sendid;
pthread_attr_t sendpattr;
size_t sendssize;
pthread_mutex_t *sendMut;
pthread_mutex_t *queueMut;
pthread_cond_t *aQueueNotEmpty;

// Defining our functions
void checkccargs(int argc, char** argv);
void makesocket(threadData *ourdata);
void makerecievethread(threadData *ourdata);
void jointhreads();
void deleterules();
void sigquitproc(int signal);
void sigspeedchangeproc(int signal);
void usage(char *appname);

// Functions for pthreads
void* receivepackets(void *dataarg);
void* sendpackets(void *dataarg);

#endif
