/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   await_pong.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduwer <eduwer@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/10 16:12:35 by eduwer            #+#    #+#             */
/*   Updated: 2020/10/24 18:57:24 by eduwer           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_traceroute.h>

void	fill_msghdr(struct msghdr *message, struct iovec *io, \
	t_echo_req *response, void *control_buffer, size_t ctrl_len, size_t packet_size)
{
	ft_bzero(io, sizeof(struct iovec));
	ft_bzero(message, sizeof(struct msghdr));
	ft_bzero(response, packet_size);
	ft_bzero(control_buffer, ctrl_len);
	io->iov_base = response;
	io->iov_len = packet_size;
	message->msg_name = NULL;
	message->msg_namelen = 0;
	message->msg_iov = io;
	message->msg_iovlen = 1;
	message->msg_control = control_buffer;
	message->msg_controllen = ctrl_len;
	message->msg_flags = 0;
}

static char	*get_icmp_response(int type)
{
	if (type == ICMP_ECHOREPLY)
		return ("ECHO REPL");
	else if (type == ICMP_DEST_UNREACH)
		return("UNREACHABLE");
	else if (type == ICMP_SOURCE_QUENCH)
		return("QUENCH");
	else if (type == ICMP_REDIRECT)
		return ("REDIRECT");
	else if (type == ICMP_ECHO)
		return ("ECHO REQ");
	else if (type == ICMP_TIME_EXCEEDED)
		return ("TTL EXCEEDED");
	else if (type == ICMP_PARAMETERPROB)
		return ("PARAM PROB");
	else if (type == ICMP_TIMESTAMP)
		return 	("TIMESTAMP REQ");
	else if (type == ICMP_TIMESTAMPREPLY)
		return ("TIMESTAMP REPL");
	else if (type == ICMP_INFO_REQUEST)
		return ("INFO REQ");
	else if (type == ICMP_INFO_REPLY)
		return 	("INFO REPL");
	else if (type == ICMP_ADDRESS)
		return 	("ADDRESS MASK REQ");
	else if (type == ICMP_ADDRESSREPLY)
		return ("ADDRESS MASK REPL");
	else
		return (NULL);
}

static void	print_res(struct timeval *time_sent, uint32_t last_ip, t_echo_req *res, t_infos *infos)
{
	char			sender[100];
	struct timeval	current_time;
	double			difftime;
	char			*domain;

	gettimeofday(&current_time, NULL);
	if (inet_ntop(AF_INET, &res->ip_header.saddr, sender, 99) == NULL)
		perror_and_exit("Error during inet_ntop");
	if (last_ip != res->ip_header.saddr)
	{
		if (infos->no_dns_resolution == false)
		{
			domain = get_domain(res->ip_header.saddr);
			printf(" %s (%s) ", domain != NULL ? domain : sender, sender);
		}
		else
			printf(" %s ", sender);
	}
	if (res->icmp_header.type != ICMP_TIME_EXCEEDED && res->icmp_header.type != ICMP_ECHOREPLY)
		printf(" %s ", get_icmp_response(res->icmp_header.type));
	else
	{
		difftime = get_ms(&current_time) - get_ms(time_sent);
		printf(" %.3f ms ", difftime);
	}
}

bool		await_pong(struct timeval *time_sent, uint32_t *last_ip, t_infos *infos)
{
	char			control_buffer[512];
	int				ret;
	t_echo_req		*response;
	struct iovec	io;
	struct msghdr	message;

	if ((response = (t_echo_req *)malloc(infos->packet_size)) == NULL)
		perror_and_exit("Error on malloc");
	response->icmp_header.type = ICMP_ECHO;
	while (response->icmp_header.type == ICMP_ECHO)
	{
		response->icmp_header.type = ICMP_ECHO;
		fill_msghdr(&message, &io, response, control_buffer, 512, infos->packet_size);
		ret = recvmsg(infos->sockfd, &message, 0);
		if (ret == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				printf(" * ");
			else
				perror_and_exit("Error during recvmsg");
			free(response);
			return (false);
		}
	}
	print_res(time_sent, *last_ip, response, infos);
	*last_ip = response->ip_header.saddr;
	if (response->icmp_header.type == ICMP_ECHOREPLY)
	{
		free(response);
		return (true);
	}
	free(response);
	return (false);
}
