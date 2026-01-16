/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtins.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 09:01:06 by miokrako         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../../includes/exec.h"

static int	is_n_flag(char *arg)
{
	int	i;

	i = 1;
	if (ft_strncmp(arg, "-n", 2) != 0)
		return (0);
	while (arg[i])
	{
		if (arg[i] != 'n')
			return (0);
		i++;
	}
	return (1);
}

// static char	*readlink_proc_self_cwd(void)
// {
// 	char	buf[PATH_MAX];
// 	ssize_t	len;

// 	len = readlink("/proc/self/cwd", buf, sizeof(buf) - 1);
// 	if (len == -1)
// 		return (NULL);
// 	buf[len] = '\0';
// 	return (ft_strdup(buf));
// }

int	builtin_echo(char **args)
{
	int	i;
	int	n_flag;

	i = 1;
	n_flag = 0;
	while (args[i] && is_n_flag(args[i]))
	{
		n_flag = 1;
		i++;
	}
	while (args[i])
	{
		ft_putstr_fd(args[i], 1);
		if (args[i + 1])
			ft_putstr_fd(" ", 1);
		i++;
	}
	if (!n_flag)
		ft_putstr_fd("\n", 1);
	return (0);
}

// ft_builtins.c
int builtin_pwd(t_shell *shell)
{
    char *cwd;
    char *pwd_env;

    // ✅ Tentative 1 : getcwd()
    cwd = getcwd(NULL, 0);
    if (cwd)
    {
        printf("%s\n", cwd);
        free(cwd);
        return (0);
    }

    // ✅ Tentative 2 : $PWD
    pwd_env = get_env_value(shell->env, "PWD");
    if (pwd_env && pwd_env[0] != '\0')
    {
        printf("%s\n", pwd_env);
        return (0);
    }

    // ❌ Échec : Afficher l'erreur POSIX
    ft_putstr_fd("pwd: error retrieving current directory: ", 2);
    ft_putstr_fd("getcwd: cannot access parent directories: ", 2);
    ft_putstr_fd("No such file or directory\n", 2);
    return (1);
}

int	builtin_env(t_env *env)
{
	while (env)
	{
		if (env->value && env->value[0] != '\0')
			printf("%s=%s\n", env->key, env->value);
		env = env->next;
	}
	return (0);
}
