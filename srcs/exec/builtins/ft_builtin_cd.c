/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtin_cd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 10:08:20 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	cd_error_no_home(char *old_pwd)
{
	ft_putstr_fd("minishell: cd: HOME not set\n", 2);
	free(old_pwd);
	return (1);
}

static int	cd_error_chdir(char *dir, char *old_pwd)
{
	ft_putstr_fd("minishell: cd: ", 2);
	ft_putstr_fd(dir, 2);
	ft_putstr_fd(": ", 2);
	ft_putstr_fd(strerror(errno), 2);
	ft_putstr_fd("\n", 2);
	free(old_pwd);
	return (1);
}

// static int	handle_getcwd_failure(t_env *env, char *old_pwd, char *dir)
// {
// 	ft_putstr_fd("cd: error retrieving current directory: ", 2);
// 	ft_putstr_fd("getcwd: cannot access parent directories: ", 2);
// 	ft_putstr_fd("No such file or directory\n", 2);
// 	update_pwd_vars_unnormalized(env, old_pwd, dir);
// 	free(old_pwd);
// 	return (0);
// }

static int	finalize_cd(t_env *env, char *old_pwd, char *dir)
{
	char	*test_cwd;

	test_cwd = getcwd(NULL, 0);
	if (!test_cwd)
	{
		ft_putstr_fd("cd: error retrieving current directory: ", 2);
		ft_putstr_fd("getcwd: cannot access parent directories: ", 2);
		ft_putstr_fd("No such file or directory\n", 2);
		update_pwd_vars_unnormalized(env, old_pwd, dir);
		free(old_pwd);
		return (0);
	}
	free(test_cwd);
	update_pwd_vars(env, old_pwd, dir);
	free(old_pwd);
	return (0);
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

/* Convertit un chemin relatif en absolu en utilisant $PWD */
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

int	builtin_cd(char **args, t_env *env)
{
	char	*dir;
	char	*old_pwd;
	char	*absolute_path;

	old_pwd = get_old_pwd(env);
	if (!old_pwd)
	{
		ft_putstr_fd("cd: error retrieving current directory: ", 2);
		ft_putstr_fd("getcwd: cannot access parent directories: ", 2);
		ft_putstr_fd("No such file or directory\n", 2);
		return (1);
	}
	if (!args || !args[1])
		dir = get_env_value(env, "HOME");
	else
		dir = args[1];
	if (!dir)
		return (cd_error_no_home(old_pwd));

	/* Convertir en chemin absolu en utilisant $PWD */
	absolute_path = make_absolute_path(old_pwd, dir);
	if (!absolute_path)
		return (cd_error_chdir(dir, old_pwd));

	/* Essayer chdir() avec le chemin absolu */
	if (chdir(absolute_path) != 0)
	{
		free(absolute_path);
		return (cd_error_chdir(dir, old_pwd));
	}

	/* Mettre à jour PWD avec le chemin relatif original */
	finalize_cd(env, old_pwd, dir);
	free(absolute_path);
	return (0);
}
