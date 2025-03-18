#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <stddef.h>
#include <unistd.h>

#include "libzetc.h"

#ifdef DEBUG
#define PRINT_DEBUG(args...) fprintf (stderr, args)
#else
#define PRINT_DEBUG(args...)
#endif

static char *ETCDIR = ".config";
static char *orig, *home, *etcdir;
static int started = 0;
static int blacklisted = 0;

// is the running exec blacklisted ?
static void am_i_blacklisted () {
	char running_exec[4096], *exec_blacklist, *str;
	int len;

	if ((len = readlink ("/proc/self/exe", running_exec, sizeof (running_exec) - 1))) {
		running_exec[len] = '\0';
		PRINT_DEBUG ("running exec: %s\n", running_exec);
		if ((exec_blacklist = getenv ("LIBETC_BLACKLIST"))) {
			PRINT_DEBUG ("blacklist: %s\n", exec_blacklist);
			while ((str = strrchr (exec_blacklist, ':'))) {
				if (0 == strcmp (++str, running_exec)) {
					blacklisted = 1;
					PRINT_DEBUG ("I am blacklisted !\n");
				}
				str--;
				str[0] = '\0';
			}
			if (0 == strcmp (exec_blacklist, running_exec)) {
				blacklisted = 1;
				PRINT_DEBUG ("I am blacklisted !\n");
			}
		}
	}
}

// find where to put the dotfiles
static void find_etcdir () {
	char *etc, *xdg_config_home;

	if (!(xdg_config_home = getenv ("XDG_CONFIG_HOME"))) {
		if (!(etc = getenv ("ETC"))) {
			etc = ETCDIR;
			PRINT_DEBUG("default value: %s\n", etc);
		}
		PRINT_DEBUG("$ETC: %s\n", etc);
		etcdir = malloc (strlen (home) + strlen (etc) + 1);
		sprintf (etcdir, "%s/%s", home, etc);
	} else {
		PRINT_DEBUG("$XDG_CONFIG_HOME: %s\n", xdg_config_home);
		etcdir = xdg_config_home;
	}
}

// mkdir etcdir if it does not exist
static void mkdir_etcdir (const char* etcdir) {
	struct stat info;

	if (-1 == stat (etcdir, &info)) {
		if (mkdir (etcdir, 0700)) {
			fprintf (stderr, "Unable to create config directory %s: ", etcdir);
			perror ("");
			exit (2);
		}
	} else {
		if (!S_ISDIR(info.st_mode)) {
			fprintf (stderr, "ERROR: %s exists and is not a directory\n", etcdir);
			exit (2);
		}
	}
}

// called only once on program startup
void start_up () {
	if (started)
		return;

	am_i_blacklisted ();

	home = getenv ("HOME");
	if (home == NULL) {
		started = 1;
		return;
	}

	orig = malloc (strlen (home) + 3);
	sprintf (orig, "%s/.", home);

	find_etcdir ();
	mkdir_etcdir (etcdir);

	PRINT_DEBUG("etcdir: %s\n", etcdir);
	started = 1;
}

// rename filename if it's a dotfile in $HOME
char *translate (const char *filename) {
	char *wd, *newfilename;

	if (!started) start_up(); // probably impossible
	if (home == NULL) return strdup (filename);
	if (blacklisted) return strdup (filename);

	if (!filename) {
		PRINT_DEBUG("Filename is NULL !\n");
		return NULL;
	}

	wd = get_current_dir_name ();

	if ((0 == strcmp (wd, home)) // if cwd == $HOME
	    && filename [0] == '.'   // and a dotfile
	    && (0 != strcmp (filename, "."))
	    && (0 != strncmp (filename, "./", 2))
	    && (0 != strncmp (filename, "..", 2))) {
		char tmpfilename [strlen (home) + strlen (filename) + 2];
		sprintf (tmpfilename, "%s/%s", home, filename);
		if (0 == strncmp (tmpfilename, etcdir, strlen(etcdir))) { // do not translate if trying to read/write in $XDG_CONFIG_HOME
			newfilename = strdup (filename);
		} else {
			filename++; // remove the dot
			newfilename = malloc (strlen (filename) + strlen (etcdir) + 2);
			sprintf (newfilename, "%s/%s", etcdir, filename);
			PRINT_DEBUG("RENAMED IN $HOME --> %s\n", newfilename);
		}
	} else if (0 == strncmp (filename, orig, strlen (orig)) // if file name is $HOME/.something
		   && 0!= strncmp (filename, etcdir, strlen (etcdir)) ) { // do not translate if trying to read/write in $XDG_CONFIG_HOME
		filename += strlen (home) + 2; // remove "$HOME/." from the filename
		newfilename = malloc (strlen (filename) + strlen (etcdir) + 2);
		sprintf (newfilename, "%s/%s", etcdir, filename);
		PRINT_DEBUG("RENAMED --> %s\n", newfilename);
	} else { // not a dotfile
		newfilename = strdup (filename);
	}

	free (wd);
	return newfilename;
}