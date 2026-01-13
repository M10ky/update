/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wait_children.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/08 20:13:41 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/13 21:55:25 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static void	check_signal(int status, int *sig_int, int *sig_quit)
{
	if (WIFSIGNALED(status))
	{
		if (WTERMSIG(status) == SIGINT)
			*sig_int = 1;
		else if (WTERMSIG(status) == SIGQUIT)
			*sig_quit = 1;
	}
}

static void	wait_loop(pid_t last_pid, int *last_status, int sig[2])
{
	pid_t	wpid;
	int		status;

	while (1)
	{
		wpid = waitpid(-1, &status, 0);
		if (wpid == -1)
			break ;
		check_signal(status, &sig[0], &sig[1]);
		if (wpid == last_pid)
			*last_status = status;
	}
}

void	wait_all_children(pid_t last_pid, t_shell *shell)
{
	int	last_status;
	int	sig[2];

	sig[0] = 0;
	sig[1] = 0;
	last_status = 0;
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	wait_loop(last_pid, &last_status, sig);
	setup_prompt_signal();
	update_exit_status(shell, last_status, sig[0], sig[1]);
}
