/* 
 * udp.h fonction in order to send a multicast stream
 * mumudvb - UDP-ize a DVB transport stream.
 * 
 * (C) 2004-2009 Brice DUBOST
 * (C) Dave Chapman <dave@dchapman.com> 2001, 2002.
 * 
 *  The latest version can be found at http://mumudvb.braice.net
 * 
 * Copyright notice:
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *     
 */

/**@file
 * @brief Networking functions
 */

#include "network.h"
#include <string.h>
#include <errno.h> 
#include <fcntl.h>

/**@brief Send data
 *@todo document
 */
int
sendudp (int fd, struct sockaddr_in *sSockAddr, unsigned char *data, int len)
{
  return sendto (fd, data, len, 0, (struct sockaddr *) sSockAddr,
		 sizeof (*sSockAddr));
}



/** @brief create a sender socket.
 *
 * Create a socket for sending data, the socket is multicast, udp, with the options REUSE_ADDR et MULTICAST_LOOP set to 1
 */
int
makesocket (char *szAddr, unsigned short port, int TTL,
	    struct sockaddr_in *sSockAddr)
{
  int iRet, iLoop = 1;
  struct sockaddr_in sin;
  char cTtl = (char) TTL;
  char cLoop = 1;

  int iSocket = socket (AF_INET, SOCK_DGRAM, 0);

  if (iSocket < 0)
    {
      log_message( MSG_WARN, "socket() failed : %s\n",strerror(errno));
      exit (1);
    }

  sSockAddr->sin_family = sin.sin_family = AF_INET;
  sSockAddr->sin_port = sin.sin_port = htons (port);
  sSockAddr->sin_addr.s_addr = inet_addr (szAddr);

  iRet = setsockopt (iSocket, SOL_SOCKET, SO_REUSEADDR, &iLoop, sizeof (int));
  if (iRet < 0)
    {
      log_message( MSG_ERROR,"setsockopt SO_REUSEADDR failed : %s\n",strerror(errno));
      exit (1);
    }

  iRet =
    setsockopt (iSocket, IPPROTO_IP, IP_MULTICAST_TTL, &cTtl, sizeof (char));
  if (iRet < 0)
    {
      log_message( MSG_ERROR,"setsockopt IP_MULTICAST_TTL failed.  multicast in kernel? error : %s \n",strerror(errno));
      exit (1);
    }

  iRet = setsockopt (iSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
		     &cLoop, sizeof (char));
  if (iRet < 0)
    {
      log_message( MSG_ERROR,"setsockopt IP_MULTICAST_LOOP failed.  multicast in kernel? error : %s\n",strerror(errno));
      exit (1);
    }

  return iSocket;
}


/** @brief create a TCP receiver socket.
 *
 * Create a socket for waiting the HTTP connection
 */
int
makeTCPclientsocket (char *szAddr, unsigned short port, 
	    struct sockaddr_in *sSockAddr)
{
  int iRet, iLoop = 1;

  int iSocket = socket (AF_INET, SOCK_STREAM, 0); //TCP

  if (iSocket < 0)
    {
      log_message( MSG_ERROR, "socket() failed.\n");
      return -1;
    }

  sSockAddr->sin_family = AF_INET;
  sSockAddr->sin_port = htons (port);
  sSockAddr->sin_addr.s_addr = inet_addr (szAddr);/**@todo use inet_aton*/

  iRet = setsockopt (iSocket, SOL_SOCKET, SO_REUSEADDR, &iLoop, sizeof (int));
  if (iRet < 0)
    {
      log_message( MSG_ERROR,"setsockopt SO_REUSEADDR failed : %s\n", strerror(errno));
      return -1;
    }


  if (bind (iSocket, (struct sockaddr *) sSockAddr, sizeof (*sSockAddr)))
    {
      log_message( MSG_ERROR, "bind failed : %s\n", strerror(errno));
      return -1;
    }

  iRet = listen(iSocket,10);
  if (iRet < 0)
    {
      log_message( MSG_ERROR,"listen failed : %s\n",strerror(errno));
      return -1;
    }

  //Now we set this socket to be non blocking because we poll it
  int flags;
  flags = fcntl(iSocket, F_GETFL, 0);
  flags |= O_NONBLOCK;
  if (fcntl(iSocket, F_SETFL, flags) < 0)
    {
      log_message(MSG_ERROR,"Set non blocking failed : %s\n",strerror(errno));
      return -1;
    }


  return iSocket;
}

