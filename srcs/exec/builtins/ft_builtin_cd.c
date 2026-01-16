/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_builtin_cd.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miokrako <miokrako@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/18 21:31:02 by miokrako          #+#    #+#             */
/*   Updated: 2026/01/16 12:32:16 by miokrako         ###   ########.fr       */
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

// int	builtin_cd(char **args, t_env *env)
// {
// 	char	*dir;
// 	char	*old_pwd;
// 	char	*absolute_path;

// 	old_pwd = get_old_pwd(env);
// 	if (!old_pwd)
// 	{
// 		ft_putstr_fd("cd: error retrieving current directory: ", 2);
// 		ft_putstr_fd("getcwd: cannot access parent directories: ", 2);
// 		ft_putstr_fd("No such file or directory\n", 2);
// 		return (1);
// 	}
// 	if (!args || !args[1])
// 		dir = get_env_value(env, "HOME");
// 	else
// 		dir = args[1];
// 	if (!dir)
// 		return (cd_error_no_home(old_pwd));

// 	/* Convertir en chemin absolu en utilisant $PWD */
// 	absolute_path = make_absolute_path(old_pwd, dir);
// 	if (!absolute_path)
// 		return (cd_error_chdir(dir, old_pwd));

// 	/* Essayer chdir() avec le chemin absolu */
// 	if (chdir(absolute_path) != 0)
// 	{
// 		free(absolute_path);
// 		return (cd_error_chdir(dir, old_pwd));
// 	}

// 	/* Mettre à jour PWD avec le chemin relatif original */
// 	finalize_cd(env, old_pwd, dir);
// 	free(absolute_path);
// 	return (0);
// }
/* ft_builtin_cd.c */

int builtin_cd(char **args, t_env *env)
{
    char *dir;
    char *old_pwd_val; // Renommé pour clarté (valeur de $PWD)
    char *target_path;
    int  chdir_ret;

    // 1. Déterminer la cible
    if (!args || !args[1])
    {
        dir = get_env_value(env, "HOME");
        if (!dir)
            return (cd_error_no_home(NULL)); // Pas besoin de free old_pwd ici
    }
    else
        dir = args[1];

    // 2. Sauvegarder l'ancien PWD pour la variable OLDPWD
    // Note: get_old_pwd doit juste lire $PWD, sans getcwd de secours si possible
    // pour respecter la logique POSIX pure en premier lieu.
    old_pwd_val = get_env_value(env, "PWD");
    if (!old_pwd_val)
        old_pwd_val = getcwd(NULL, 0); // Repli si pas de var PWD

    // 3. TENTATIVE LOGIQUE (Si PWD existe et chemin relatif)
    // C'est ce qui permet de gérer les liens symboliques et .. proprement
    target_path = NULL;
    chdir_ret = -1;

    // Si on a PWD et que c'est un chemin relatif (pas / au début)
    if (old_pwd_val && dir[0] != '/')
    {
        target_path = make_absolute_path(old_pwd_val, dir); // Votre fonction existante
        if (target_path)
        {
            chdir_ret = chdir(target_path);
            if (chdir_ret == 0)
            {
                // Succès logique !
                update_pwd_vars(env, old_pwd_val, target_path); // Met à jour PWD avec le chemin logique
                free(target_path);
                if (old_pwd_val && old_pwd_val != get_env_value(env, "PWD")) free(old_pwd_val);
                return (0);
            }
            free(target_path); // Échec logique (dossier n'existe pas, etc.)
        }
    }

    // 4. TENTATIVE PHYSIQUE (Repli)
    // Si la logique a échoué ou n'était pas applicable (chemin absolu ou pas de PWD)
    chdir_ret = chdir(dir);

    if (chdir_ret != 0)
    {
        // Vrai échec (ni logique ni physique ne marchent)
        // Ici, Bash affiche l'erreur.
        // MAIS pour le cas "cd .." coincé, c'est compliqué.
        // Si chdir("..") échoue, on est vraiment coincé.
        ft_putstr_fd("minishell: cd: ", 2);
        perror(dir);
        if (old_pwd_val && old_pwd_val != get_env_value(env, "PWD"))
			free(old_pwd_val);
        return (1);
    }

    // 5. SUCCÈS PHYSIQUE
    // On a réussi à bouger, mais on a peut-être perdu le fil logique.
    // On doit reconstruire PWD via getcwd.
    char *cwd = getcwd(NULL, 0);
    if (!cwd)
    {
        // Cas très spécifique : on a bougé (chdir ok) mais getcwd échoue.
        // C'est le message "error retrieving current directory".
        ft_putstr_fd("cd: error retrieving current directory: getcwd: cannot access parent directories: No such file or directory\n", 2);

        // POSIX dit : si getcwd échoue, PWD doit quand même être mis à jour si possible
        // avec le chemin concaténé si on l'avait, ou laissé tel quel.
        // Pour votre cas unset, on ne peut pas faire grand chose de plus.
    }
    else
    {
        update_env_var(env, "PWD", cwd);
        free(cwd);
    }

    // Mise à jour OLDPWD
    if (old_pwd_val)
    {
        update_env_var(env, "OLDPWD", old_pwd_val);
        if (old_pwd_val != get_env_value(env, "PWD"))
			free(old_pwd_val);
    }

    return (0);
}
