/*
 packetqueue.h
 Copyright (C) 2010 Stefano Ciccarelli (step76)

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

#ifndef WF2Q_H
#define WF2Q_H

#include "packetqueue.h"
#include "heap.h"

/* helpful functions */
#define max(x,y) (x<y)?y:x
#define min(x,y) (x>y)?y:x

/* default size of the per-flow queue (in bytes) */
#define DEF_QUEUE_SIZE 50*1500

/* default flow weight */
#define DEF_FLOW_WEIGHT 0.0

/* maximum numbers of flows */
#define MAXFLOWS 10


class WF2Q {
  public: 
    WF2Q();
    bool init (int flowid, int size, int weight);
    bool enque(pPacket* pkt);
    pPacket* deque();

  private:
    /* flow structure */
    struct flowState {
      /* packet queue associated to the corresponding flow */
      heap q_;
      int qmaxSize_;     /* maximum queue size (in bytes) */
      int qcrtSize_;     /* current queue size (in bytes) */
      double weight_;    /* Weight of the flow */
      double S_;         /* Starting time of flow , not checked for wraparound*/
      double F_;         /* Ending time of flow, not checked for wraparound */
    } fs_[MAXFLOWS];

    double V;            /* Virtual time , not checked for wraparound!*/
};

WF2Q::WF2Q()
{
  /* initialize flow's structure */
  for (int i = 0; i < MAXFLOWS; i++) {
    fs_[i].qmaxSize_  = DEF_QUEUE_SIZE;
    fs_[i].qcrtSize_  = 0;
    fs_[i].weight_    = DEF_FLOW_WEIGHT;
    fs_[i].S_     = 0.0;
    fs_[i].F_     = 0.0;
  }
  V = 0.0;
}


bool WF2Q::init(int flowId, int size, int weight)
{
  /* initialize the specified flow id */
 
  if (flowId >= MAXFLOWS)
    return false;

  if (size < 0)
    return false;

  fs_[flowId].qmaxSize_ = size * 1500;
  fs_[flowId].weight_   = (double)weight;
  
  return true;
}

bool WF2Q::enque(pPacket *pkt)
{
  int flowId    = pkt->flowId;
  int pktSize   = pkt->size;

  if (flowId >= MAXFLOWS)
    return false;

  /* if buffer full, drop the packet; else enqueue it */
  if (fs_[flowId].qcrtSize_ + pktSize <= fs_[flowId].qmaxSize_) {
    if (!fs_[flowId].qcrtSize_) {

      /* If queue for the flow is empty, calculate start and finish times */
      fs_[flowId].S_ = max(V, fs_[flowId].F_);
      fs_[flowId].F_ = fs_[flowId].S_ + ((double)pktSize/fs_[flowId].weight_);
      /* the weight_ parameter better not be 0! */

      /* update system virtual clock */
      double minS = fs_[flowId].S_;
      for (int i = 0; i < MAXFLOWS; i++)
        if (fs_[i].qcrtSize_)
          minS = min(fs_[i].S_, minS);
      V = max(minS, V);
      //syslog(LOG_NOTICE, "V %f", V);
    }

    fs_[flowId].q_.push((void*)pkt); 
    fs_[flowId].qcrtSize_ += pktSize;
    
    return true;
  } else {
    // packet dropped
    return false;
  }
}

/* 
 * Dequeue the packet.
 */
pPacket* WF2Q::deque(void)
{
  pPacket* pkt = NULL;
  pPacket* nextPkt = NULL;
  int     i;
  int     pktSize = 0;
  double  minF = 0;
  int     flow = -1;

  /* look for the candidate flow with the earliest finish time */
  for (i = 0; i < MAXFLOWS; i++) {
    if (!fs_[i].qcrtSize_)
      continue;
    if (fs_[i].S_ <= V)
      if (fs_[i].F_ < minF || flow == -1) {
        flow = i;
        minF = fs_[i].F_;
      }
  }

  if (flow == -1)
    return pkt;

  /* pop the packet from the selected flow */
  pkt = (pPacket*)(fs_[flow].q_.pop());
  pktSize = pkt->size;
  fs_[flow].qcrtSize_ -= pktSize;

  /* Set the start and the finish times of the remaining packets in the queue */
  if ((nextPkt = (pPacket*)(fs_[flow].q_.front())) != NULL) {
    fs_[flow].S_ = fs_[flow].F_;
    fs_[flow].F_ = fs_[flow].S_ + ((double)nextPkt->size/fs_[flow].weight_);
    /* the weight parameter better not be 0 */
  }

  /* update the virtual clock */
  double W    = 0.0;
  double minS = -1;
  for (i = 0; i < MAXFLOWS; i++) {
    W += fs_[i].weight_;
    if (fs_[i].qcrtSize_)
      if (minS == -1)
        minS = fs_[i].S_;
      else
        minS = min(fs_[i].S_, minS);
  }
  V = max(minS, (V + ((double)pktSize/W)));
  //syslog(LOG_NOTICE, "V %f", V);
  
  return pkt;
}

#endif
