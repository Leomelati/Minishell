/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lmartins <lmartins@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/08/27 10:37:11 by tguimara          #+#    #+#             */
/*   Updated: 2021/10/14 06:01:36 by lmartins         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <readline/readline.h>
#include "minishell.h"

#define TEMP_PATH "./temp"

static char	*find_executable(char *command, char *cur_dir, char **path);

static void	exec_builtin(t_pipeline **pipeline, t_config **shell_config)
{
	t_command	*command;
	t_env		*env;

	command = (*pipeline)->command_list;
	// env = (*shell_config)->env;
	if (!ft_strncmp(command->command, "pwd", 3))
		my_pwd();
	else if (!ft_strncmp(command->command, "env", 3))
		my_env(command->total_args, command->args, (*shell_config)->env);
	else if (!ft_strncmp(command->command, "export", 6))
		my_export(command->total_args, command->args, &(*shell_config)->env);
	else if (!ft_strncmp(command->command, "unset", 5))
		my_unset(command->total_args, command->args, &(*shell_config)->env);
	else if (!ft_strncmp(command->command, "cd", 2))
		my_cd(command->total_args, command->args);
	else if (!ft_strncmp(command->command, "echo", 4))
		my_echo(command->args);
	else if (!ft_strncmp(command->command, "exit", 4))
		my_exit(shell_config, pipeline);
	printf("Exec_builtin: %d\n", (*shell_config)->should_continue);
	// exit(1);
}

static int	read_from_source(t_command *command)
{
	char	*buffer;
	size_t	keyword_size;
	int		fd;

	keyword_size = ft_strlen(command->redirection_in[1]);
	fd = open(TEMP_PATH, O_WRONLY | O_TRUNC | O_CREAT);
	while (1)
	{
		buffer = readline("> ");
		if (!ft_strncmp(command->redirection_in[1], buffer, keyword_size))
			break ;
		write(fd, buffer, ft_strlen(buffer));
		write(fd, "\n", 1);
		free(buffer);
	}
	if (buffer)
		free(buffer);
	close(fd);
	return (open(TEMP_PATH, O_RDONLY));
}

/*
	bug 1: cat << oi > text.xt
	bug 2: ls | cat > text.xt

*/
static void	handle_redirection(t_command *command)
{
	int	redir_file;

	if (command->redirection_out && command->redirection_out[0])
	{
		if (!ft_strncmp(command->redirection_out[0], ">>", 2))
			redir_file = open(command->redirection_out[1],
					O_WRONLY | O_CREAT | O_APPEND);
		else
			redir_file = open(command->redirection_out[1],
					O_WRONLY | O_CREAT | O_TRUNC);
		dup2(redir_file, 1);
		close(redir_file);
	}
	if (command->redirection_in && command->redirection_in[0])
	{
		if (!ft_strncmp(command->redirection_in[0], "<", 2))
			redir_file = open(command->redirection_in[1], O_RDONLY | O_CREAT);
		else
			redir_file = read_from_source(command);
		dup2(redir_file, 0);
		close(redir_file);
		if (!ft_strncmp(command->redirection_in[0], "<<", 2))
			unlink(TEMP_PATH);
	}	
}

static void	handle_pipe(t_command *command, t_config *shell_config)
{
	static int	fd[2];
	static int	come_from_pipe;

	if (command->has_pipe == 1)
	{
		if (come_from_pipe)
		{
			dup2(fd[0], 0);
			close(fd[0]);
		}
		pipe(fd);
		dup2(fd[1], 1);
		close(fd[1]);
		come_from_pipe = 1;
	}
	else
	{
		dup2(shell_config->stdout, 1);
		dup2(fd[0], 0);
		close(fd[0]);
	}
}

void	exec(t_pipeline **pipeline, t_config **shell_config)
{
	int			child_pid;
	t_command	*command;

	command = (*pipeline)->command_list;
	while (command)
	{
		if ((*pipeline)->total_commands > 1)
			handle_pipe(command, *shell_config);
		child_pid = fork();
		if (child_pid == 0)
		{
			handle_redirection(command);
			if (command->builtin)
				exec_builtin(pipeline, shell_config);
			else
				execve(command->exec_path, command->args,
					t_env_to_array((*shell_config)->env));
			//my_env(command->total_args, command->args, (*shell_config)->env);
		}
		else
		{
			waitpid(child_pid, &(*shell_config)->last_exit_status, 0);
			dup2((*shell_config)->stdin, 0);
			dup2((*shell_config)->stdout, 1);
		}
		command = command->next;
	}
}
