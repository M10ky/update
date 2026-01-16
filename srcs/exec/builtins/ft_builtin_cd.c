/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtin_cd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 15:12:25 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	cd_error_no_home(char *old_pwd)
{
	ft_putstr_fd("minishell: cd: HOME not set\n", 2);
	free(old_pwd);
	return (1);
}

static char	*resolve_dotdot_logical(char *pwd)
{
	char	*last_slash;
	char	*result;

	if (!pwd || pwd[0] != '/')
		return (NULL);
	if (ft_strcmp(pwd, "/") == 0)
		return (ft_strdup("/"));
	last_slash = ft_strrchr(pwd, '/');
	if (!last_slash || last_slash == pwd)
		return (ft_strdup("/"));
	result = ft_strndup(pwd, last_slash - pwd);
	return (result);
}

static char	*make_absolute_path(char *old_pwd, char *dir)
{
	char	*temp;
	char	*result;

	if (!old_pwd || !dir)
		return (NULL);
	if (dir[0] == '/')
		return (ft_strdup(dir));
	if (ft_strcmp(dir, "..") == 0)
		return (resolve_dotdot_logical(old_pwd));
	temp = ft_strjoin(old_pwd, "/");
	if (!temp)
		return (NULL);
	result = ft_strjoin(temp, dir);
	free(temp);
	return (result);
}

static int	try_logical_cd(char *old_pwd, char *dir, t_env *env)
{
	char	*target_path;
	int		chdir_ret;

	if (!old_pwd || dir[0] == '/')
		return (-1);
	target_path = make_absolute_path(old_pwd, dir);
	if (!target_path)
		return (-1);
	chdir_ret = chdir(target_path);
	if (chdir_ret == 0)
	{
		update_pwd_vars(env, old_pwd, target_path);
		free(target_path);
		return (0);
	}
	free(target_path);
	return (-1);
}

int	builtin_cd(char **args, t_env *env)
{
	char	*dir;
	char	*old_pwd_val;
	int		ret;

	if (!args || !args[1])
	{
		dir = get_env_value(env, "HOME");
		if (!dir)
			return (cd_error_no_home(NULL));
	}
	else
		dir = args[1];
	old_pwd_val = get_env_value(env, "PWD");
	if (!old_pwd_val)
		old_pwd_val = getcwd(NULL, 0);
	ret = try_logical_cd(old_pwd_val, dir, env);
	if (ret == 0)
		return (0);
	if (chdir(dir) != 0)
		return (handle_chdir_failure(dir, old_pwd_val, env));
	update_pwd_after_cd(env, old_pwd_val);
	return (0);
}
