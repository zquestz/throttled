/*
 packetqueue.h
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

#ifndef PACKETQUEUE_H
#define PACKETQUEUE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct pPacket
{
	char *packet;
	long size;
	struct sockaddr	toaddr;
	socklen_t fromsize;
	int	sockid;
	int flowId;

	pPacket(int in_flowId, int s, const void *msg, size_t len, const struct sockaddr *to, int tolen )
	{
		flowId = in_flowId;
		sockid = s;
		packet = new char[len];
		size = len;
		memcpy(packet, msg, len);
		toaddr = (*to);
		fromsize = tolen;
	}

	pPacket()
	{
	}

	~pPacket()
	{
	}
};

#endif
