/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtin_cd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 12:21:45 by miokrako         ###   ########.fr       */
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

int builtin_cd(char **args, t_env *env)
{
    char *dir;
    char *old_pwd_val;
    char *target_path;
    int  chdir_ret;

    // 1. Déterminer la cible
    if (!args || !args[1])
    {
        dir = get_env_value(env, "HOME");
        if (!dir)
            return (cd_error_no_home(NULL));
    else
        dir = args[1];
    old_pwd_val = get_env_value(env, "PWD");
    if (!old_pwd_val)
        old_pwd_val = getcwd(NULL, 0); // Repli si pas de var PWD
    target_path = NULL;
    chdir_ret = -1;

    if (old_pwd_val && dir[0] != '/')
    {
        target_path = make_absolute_path(old_pwd_val, dir); // Votre fonction existante
        if (target_path)
        {
            chdir_ret = chdir(target_path);
            if (chdir_ret == 0)
            {
                update_pwd_vars(env, old_pwd_val, target_path); // Met à jour PWD avec le chemin logique
                free(target_path);
                if (old_pwd_val && old_pwd_val != get_env_value(env, "PWD")) free(old_pwd_val);
                return (0);
            }
            free(target_path); // Échec logique (dossier n'existe pas, etc.)
        }
    }
    chdir_ret = chdir(dir);

    if (chdir_ret != 0)
    {
        ft_putstr_fd("minishell: cd: ", 2);
        perror(dir);
        if (old_pwd_val && old_pwd_val != get_env_value(env, "PWD"))
			free(old_pwd_val);
        return (1);
    }

    char *cwd = getcwd(NULL, 0);
    if (!cwd)
    {
        ft_putstr_fd("cd: error retrieving current directory: getcwd: cannot access parent directories: No such file or directory\n", 2);
    }
    else
    {
        update_env_var(env, "PWD", cwd);
        free(cwd);
    }
    if (old_pwd_val)
    {
        update_env_var(env, "OLDPWD", old_pwd_val);
        if (old_pwd_val != get_env_value(env, "PWD"))
			free(old_pwd_val);
    }
    return (0);
}
