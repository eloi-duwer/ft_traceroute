/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduwer <eduwer@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/06/30 18:31:52 by eduwer            #+#    #+#             */
/*   Updated: 2021/03/12 16:03:46 by eduwer           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __FT_TRACEROUTE_H__
# define __FT_TRACEROUTE_H__
# include <arpa/inet.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>
# include <netdb.h>
# include <netinet/ip_icmp.h>
# include <signal.h>
# include <errno.h>
# include <stdbool.h>
# include <stddef.h>
# include <float.h>
# include <math.h>
# include <libft.h>

typedef	struct		s_infos {
	size_t				packet_size;
	unsigned short		seq;
	unsigned short		first_ttl;
	unsigned short		max_ttl;
	int					sockfd;
	int					n_queries;
	struct sockaddr_in	addr;
	char				*addr_name;
	int					timeout;
	bool				no_dns_resolution;
}					t_infos;

typedef struct		s_echo_req {
	struct iphdr	ip_header;
	struct icmphdr	icmp_header;
}					t_echo_req;

void	send_ping(unsigned short ttl, struct timeval *time_sent, t_infos *infos);
bool	await_pong(struct timeval *time_sent, uint32_t *last_ip, t_infos *infos);
void	perror_and_exit(char *str);
double	get_ms(struct timeval *time);
void	print_help(void);
void	print_packet(t_echo_req *packet);
char	*get_domain(uint32_t ip);

#endif

