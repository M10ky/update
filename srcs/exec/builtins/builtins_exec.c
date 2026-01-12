/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtins_exec.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/11 23:05:52 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/12 07:57:43 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

char	**convert_args_for_builtins(t_arg *args)
{
	char	**result;
	int		count;
	int		i;

	if (!args)
	{
		result = (char **)malloc(sizeof(char *) * 1);
		if (!result)
			return (NULL);
		result[0] = NULL;
		return (result);
	}
	count = 0;
	while (args[count].value)
		count++;
	result = (char **)malloc(sizeof(char *) * (count + 1));
	if (!result)
		return (NULL);
	i = 0;
	while (i < count)
	{
		result[i] = args[i].value;
		i++;
	}
	result[i] = NULL;
	return (result);
}

static int	execute_builtin_part1(t_shell *shell, char **args_array)
{
	if (ft_strcmp(args_array[0], "echo") == 0)
		return (builtin_echo(args_array));
	if (ft_strcmp(args_array[0], "cd") == 0)
		return (builtin_cd(args_array, shell->env));
	if (ft_strcmp(args_array[0], "pwd") == 0)
		return (builtin_pwd());
	if (ft_strcmp(args_array[0], "export") == 0)
		return (ft_export(shell, args_array));
	return (-1);
}

static int	execute_builtin_part2(t_shell *shell, char **args_array)
{
	if (ft_strcmp(args_array[0], "unset") == 0)
		return (ft_unset(shell, args_array));
	if (ft_strcmp(args_array[0], "env") == 0)
		return (builtin_env(shell->env));
	if (ft_strcmp(args_array[0], "exit") == 0)
		return (builtin_exit(args_array, shell));
	return (1);
}

int	execute_builtin(t_command *cmd, t_shell *shell)
{
	int		result;
	char	**args_array;

	if (!cmd || !cmd->args || !cmd->args[0].value)
		return (1);
	args_array = convert_args_for_builtins(cmd->args);
	if (!args_array)
		return (1);
	result = execute_builtin_part1(shell, args_array);
	if (result != -1)
	{
		free(args_array);
		return (result);
	}
	result = execute_builtin_part2(shell, args_array);
	if (ft_strcmp(args_array[0], "exit") != 0)
		free(args_array);
	return (result);
}
