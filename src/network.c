/*
 * Copyright (C) 2014-2016 Marc Chalain <marc.chalain@gmail.com>
 *
 * This file is part of uplaymusic.
 *
 * libdlna is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * libdlna is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libdlna; if not, write to the Free Software
 * Foundation, Inc, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <sched.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "network.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#define IPV6

static int http_request_get (int fd, char *page)
{
	int ret = -1;
	int len;
	char *wbuff;
	/**
	 * generate the request
	 **/
	wbuff = malloc(1024 + 1);
	len = 0;
	sprintf(wbuff+len, "GET %s HTTP/1.0\r\n", page);
	len += strlen(page) + 15;
/*
 	sprintf(wbuff+len, "User-Agent: mpg123/1.12.1\r\n");
	len += 27;
	sprintf(wbuff+len, "Host: 10.1.2.9\r\n");
	len += 22;
	sprintf(wbuff+len, "Accept: audio/mpeg, audio/x-mpeg, audio/mp3, audio/x-mp3, audio/mpeg3, audio/x-mpeg3, audio/mpg, audio/x-mpg, audio/x-mpegaudio, audio/mpegurl, audio/mpeg-url, audio/x-mpegurl, audio/x-scpls, audio/scpls, application/pls\r\n");
	len += 227;
*/
	sprintf(wbuff+len, "\r\n");
	len += 2;
	
	/**
	 * send the request
	 **/
	ret = write(fd, wbuff, len);
	free(wbuff);
  return ret;
}

static int http_wait (int fd)
{
	int ret = -1;
	int bytesAv = 0;
	//int ret;
	fd_set rfds;
	int maxfd;
	struct timeval timeout = {.tv_sec=3, .tv_usec=0,};
	struct timeval *ptimeout;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	maxfd = fd +1;

	ptimeout = NULL;
	ret = select(maxfd, &rfds, NULL, NULL, ptimeout);
	if (ret > 0 && FD_ISSET(fd, &rfds))
	{
		
		ioctl (fd,FIONREAD,&bytesAv);
		ret = bytesAv;
	}
  return ret;
}

static int http_recieve_header (int fd, char *header, int len)
{
  int i = 0, j = 0;

	for (i = 0; i < len; i++)
	{
		int ret = -1;
		ret = recv(fd, header + i, 1, 0);
		if (ret <= 0)
		{
			LOG_ERROR("read from socket error %d %s", ret, strerror(errno));
			return -1;
		}
		/* search end of header */
		/* od value for i*/
		if ((j & 0x01) == 0x1)
		{
			if (header[i] == '\n')
				j ++;
			else
				j = 0;
		}
		else
		{
			if (header[i] == '\r')
				j ++;
			else
				j = 0;
		}
		if (j == 4)
			break;
	}
	header[i] = 0;
	return i;
}

static int parse_header (char *header, struct http_info *info)
{
	/**
	 * parse the header
	 **/
	char *value;

	info->mime[0] = 0;
	info->location[0] = 0;
	info->length = -1;
	if ((value = strcasestr(header, "CONTENT-LENGTH: ")))
	{
		sscanf(value + 16,"%u[^\r]", &info->length);
	}
	if ((value = strcasestr(header, "CONTENT-TYPE: ")))
	{
		sscanf(value + 14,"%99[^\r]", info->mime);
	}
	if ((value = strcasestr(header, "Location: ")))
	{
		sscanf(value + 10,"%199[^\r]", info->location);
	}
	if (value != NULL)
		return (value - header);
	else
	  return -1;
}

static int
http_get_transaction(int fd, char *page, struct http_info *info)
{
	int ret = -1;
	char *rbuff;

	ret = http_request_get (fd, page);
	if (ret < 0)
	{
		close (fd);
		return -1;
	}

	ret = http_wait (fd);
	if (ret < 0)
	{
		close (fd);
		return ret;
	}

  /* no data still available */
  if (ret == 0)
		ret = 1024;
	rbuff = calloc(ret + 1, sizeof (char));
	ret = http_recieve_header (fd, rbuff, ret);
	if (ret < 0)
	{
		free (rbuff);
		close (fd);
		return ret;
	}

	if (info)
		parse_header (rbuff, info);
	free (rbuff);
	if (info && strlen (info->location) > 0)
	{
		close (fd);
		fd = http_get (info->location, info);
	}
	return fd;
}

int
http_get(char *uri, struct http_info *info)
{
	int fd = -1;
	int port = 0;
	char proto[10];
	char ip[100];
	char page[200];
	int err = -1;

	memset(proto, 0, 10);
	memset(ip, 0, 100);
	memset(page, 0, 200);
	port = 80;

	page[0]='/';
	if (sscanf(uri, "%9[^:]://%99[^:]:%i/%198[^\n]", proto, ip, &port, page+1) == 4) { err = 0;}
	else if (sscanf(uri, "%9[^:]://%99[^/]/%198[^\n]", proto, ip, page+1) == 3) { err = 0;}
	else if (sscanf(uri, "%9[^:]://%99[^:]:%i[^\n]", proto, ip, &port) == 3) { err = 0;}
	else if (sscanf(uri, "%9[^:]://%99[^\n]", proto, ip) == 2) { err = 0;}

	if (!err)
	{
#ifndef IPV6
		struct sockaddr_in server;
		struct hostent *entity;

		if((server.sin_addr.s_addr = inet_addr(ip)) == INADDR_NONE)
		{
			entity = gethostbyname (ip);
			if (!entity)
				return -1;
			memcpy(&server.sin_addr, entity->h_addr_list[0], entity->h_length);
		}

		server.sin_port = htons(port);
		server.sin_family = AF_INET;

		if((fd = socket(PF_INET, SOCK_STREAM, 6)) < 0)
		{
			LOG_ERROR("Cannot create socket: %s", strerror(errno));
			return -1;
		}
		if(connect(fd, (struct sockaddr *)&server, sizeof(server)))
			return -1;
#else
		struct addrinfo hints;

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
		hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
		hints.ai_flags = 0;
		hints.ai_protocol = 0;          /* Any protocol */
		if (strncmp(proto,"http",4))
		{
			LOG_ERROR("bad protocol type %s", proto);
			return -1;
		}
		struct addrinfo *result, *rp;
		char aport[6];
		sprintf(aport,"%u",port);

		err = getaddrinfo(ip, aport, &hints, &result);
		if (err)
			return err;

		for (rp = result; rp != NULL; rp = rp->ai_next)
		{
			int yes = 0;

			fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
			if (fd == -1)
				continue;
			else if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
			{
				LOG_DEBUG("error reuse opt: %s", strerror(errno));
				close(fd);
				fd = -errno;
				continue;
			}

			if (connect(fd, rp->ai_addr, rp->ai_addrlen) != -1)
				break;                  /* Success */

			close(fd);
		}
		freeaddrinfo(result);
		if (rp == NULL)
			return -1;
#endif

		fd = http_get_transaction(fd, uri, info);

	}
	return fd;
}

#ifdef HTTP_GET_MAIN
int main(int argc, char **argv)
{
	if (argc > 1);
	{
		struct http_info info;
		int fd;
		fd = http_get(argv[1], &info);
		printf("content length = %u\n",info.length);
		printf("content type = %s\n",info.mime);
		if (fd > 0)
			close(fd);
	}
	return 0;
}
#endif
