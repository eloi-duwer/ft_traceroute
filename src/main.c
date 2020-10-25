/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eduwer <eduwer@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/06/30 18:34:20 by eduwer            #+#    #+#             */
/*   Updated: 2020/10/24 19:01:09 by eduwer           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ft_traceroute.h>

static void	get_address(char *str, t_infos *infos)
{
	struct addrinfo hints;
	struct addrinfo *ret;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;
	hints.ai_flags = AI_CANONNAME;
	if (getaddrinfo(str, NULL, &hints, &ret) != 0)
		perror_and_exit("Cannot convert address to ip");
	ft_memset(&infos->addr, 0, sizeof(struct sockaddr_in));
	infos->addr.sin_addr.s_addr = ((struct sockaddr_in *)ret->ai_addr)->sin_addr.s_addr;
	infos->addr.sin_family = AF_INET;
	infos->addr_name = str;
	freeaddrinfo(ret);
}

static void	parse_args(int ac, char **av, t_infos *infos)
{
	int i;

	i = 1;
	while (i < ac)
	{
		if (av[i][0] == '-')
		{
			if (ft_strcmp(av[i], "-h") == 0)
				print_help();
			else if (ft_strcmp(av[i], "-n") == 0)
				infos->no_dns_resolution = true;
			else
			{
				printf("Unrecognized option: %s\n", av[i]);
				exit(1);
			}
			
		}
		else if (infos->addr_name == NULL)
			get_address(av[i], infos);
		else
			print_help();
		i++;
	}
}

static void	create_socket(t_infos *infos)
{
	struct timeval	timeout;		

	timeout.tv_sec = infos->timeout;
	timeout.tv_usec = 0;
	infos->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (infos->sockfd == -1)
		perror_and_exit("Cannot open socket");
	if (setsockopt(infos->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1
		|| setsockopt(infos->sockfd, IPPROTO_IP, IP_HDRINCL, (int[1]){1}, sizeof(int)) == -1)
	{
		perror_and_exit("Error while setting socket options");
	}
}

static void	print_start_traceroute(t_infos *infos)
{
	char	dest[100];

	if (inet_ntop(AF_INET, &infos->addr.sin_addr.s_addr, dest, 99) == NULL)
		perror_and_exit("Error during inet_ntop");
	printf("traceroute to %s (%s), %d hops max, %ld byte packets\n", infos->addr_name, dest, infos->ttl, infos->packet_size);
}

static void	init_infos(t_infos *infos)
{
	infos->ttl = 30;
	infos->packet_size = 60;
	infos->seq = 0;
	infos->timeout = 2;
	infos->no_dns_resolution = false;
	infos->first_ttl = 1;
}

void	start_traceroute(t_infos *infos)
{
	unsigned short	ttl;
	int				i;
	struct timeval	time_sent;
	uint32_t		last_ip;
	bool			reached[3];

	ttl = infos->first_ttl;
	while (ttl <= infos->max_ttl)
	{
		i = 0;
		last_ip = 0;
		printf("%d ", ttl);
		while (i < 3)
		{
			send_ping(ttl, &time_sent, infos);
			reached[i] = await_pong(&time_sent, &last_ip, infos);
			++i;
		}
		printf("\n");
		if (reached[0] == true && reached[1] == true && reached[2] == true)
			return;
		ttl++;
	}
}

int		main(int argc, char **argv)
{
	t_infos	infos;

	if (argc == 1)
		print_help();
	init_infos(&infos);
	parse_args(argc, argv, &infos);
	create_socket(&infos);
	print_start_traceroute(&infos);
	start_traceroute(&infos);
	return (0);
}
