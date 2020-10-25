/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   send_ping.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduwer <eduwer@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/10 16:01:00 by eduwer            #+#    #+#             */
/*   Updated: 2020/10/24 18:09:27 by eduwer           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_traceroute.h>

static unsigned short	checksum_icmphdr(void *data, size_t size)
{
	unsigned long	sum;
	unsigned short	*ptr;

	sum = 0;
	ptr = (unsigned short *)data;
	while (size > 1)
	{
		sum += (((*ptr & 0xff00) >> 8) +
			((*ptr & 0xff) << 8));
		size -= sizeof(unsigned short);
		ptr++;
	}
	sum = ~((sum & 0xffff) + ((sum & 0xffff0000) >> 16));
	return ((unsigned short)sum);
}

static void				fill_ip_header(t_echo_req *req, unsigned short ttl, size_t packet_size, uint32_t daddr)
{
	req->ip_header.version = 4;
	req->ip_header.ihl = 5;
	req->ip_header.tos = 0;
	req->ip_header.tot_len = htons(packet_size);
	req->ip_header.id = 0;
	req->ip_header.frag_off = 0;
	req->ip_header.ttl = ttl;
	req->ip_header.protocol = IPPROTO_ICMP;
	req->ip_header.check = 0;
	req->ip_header.saddr = INADDR_ANY;
	req->ip_header.daddr = daddr;
}

static void				fill_icmp_header(t_echo_req *req, size_t packet_size, unsigned short seq)
{
	req->icmp_header.type = ICMP_ECHO;
	req->icmp_header.code = 0;
	req->icmp_header.un.echo.id = htons(getpid());
	req->icmp_header.un.echo.sequence = htons(seq);
	req->icmp_header.checksum = 0;
	req->icmp_header.checksum = htons(checksum_icmphdr(&req->icmp_header, packet_size - offsetof(t_echo_req, icmp_header)));
}

void					send_ping(unsigned short ttl, struct timeval *time_sent, t_infos *infos)
{
	t_echo_req	*req;
	int			ret;

	if ((req = (t_echo_req *)malloc(infos->packet_size)) == NULL)
		perror_and_exit("Error on malloc");
	ft_memset(req, 0, infos->packet_size);
	fill_ip_header(req, ttl, infos->packet_size, infos->addr.sin_addr.s_addr);
	fill_icmp_header(req, infos->packet_size, infos->seq);
	gettimeofday(time_sent, NULL);
	ret = sendto(infos->sockfd, req, infos->packet_size, 0, (struct sockaddr *)&infos->addr, sizeof(struct sockaddr));
	if (ret == -1)
		perror_and_exit("Error while sending packet");
	infos->seq++;
	free(req);
}
