/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmartins <lmartins@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/10/07 22:06:59 by tguimara          #+#    #+#             */
/*   Updated: 2021/10/15 06:05:17 by lmartins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	**bultin_init()
{
	char		*builtins_string;
	char		**builtin_list;

	builtins_string = ft_strdup("echo,cd,pwd,export,unset,env,exit");
	builtin_list = ft_split(builtins_string, ',');
	if (!builtin_list)
		return ((char **) NULL);
	if (builtins_string)
		free(builtins_string);
	g_shell_config->free_list->bultin = true;
	return (builtin_list);
}

int	is_builtin(char *command, char **builtin_list)
{
	int		i;
	size_t	command_size;
	size_t	max_size;

	command_size = ft_strlen(command);
	max_size = command_size;
	i = 0;
	while (builtin_list && builtin_list[i])
	{
		if (ft_strlen(builtin_list[i]) > max_size)
			max_size = ft_strlen(builtin_list[i]);
		if (!ft_strncmp(command, builtin_list[i], max_size))
			return (1);
		if (max_size != command_size)
			max_size = command_size;
		i++;
	}
	return (0);
}