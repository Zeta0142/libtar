/*
**  Copyright 1998-2003 University of Illinois Board of Trustees
**  Copyright 1998-2003 Mark D. Roth
**  All rights reserved.
**
**  decode.c - libtar code to decode tar header blocks
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <internal.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <pwd.h>
#include <grp.h>

#ifdef STDC_HEADERS
# include <string.h>
#endif

char *
safer_name_suffix (char const *file_name)
{
	char const *p, *t;
	p = t = file_name;
	while (*p == '/') t = ++p;
	while (*p)
	{
		while (p[0] == '.' && p[0] == p[1] && p[2] == '/')
		{
			p += 3;
			t = p;
		}
		/* advance pointer past the next slash */
		while (*p && (p++)[0] != '/');
	}

	if (!*t)
	{
		t = ".";
	}

	if (t != file_name)
	{
		/* TODO: warn somehow that the path was modified */
	}
	return (char*)t;
}


/* determine full path name */
char *
th_get_pathname(TAR *t)
{
	if (t->th_buf.gnu_longname)
		return safer_name_suffix(t->th_buf.gnu_longname);

	/* allocate the th_pathname buffer if not already */
	if (t->th_pathname == NULL)
	{
		t->th_pathname = malloc(MAXPATHLEN * sizeof(char));
		if (t->th_pathname == NULL)
			/* out of memory */
			return NULL;
	}

	/*
	 * Old GNU headers (also used by newer GNU tar when doing incremental
	 * dumps) use the POSIX prefix field for many other things, such as
	 * mtime and ctime. New-style GNU headers don't, but also don't use the
	 * POSIX prefix field. Thus, only honor the prefix field if the archive
	 * is actually a POSIX archive. This is the same logic as GNU tar uses.
	 */
	if (strncmp(t->th_buf.magic, TMAGIC, TMAGLEN - 1) != 0 || t->th_buf.prefix[0] == '\0')
	{
		snprintf(t->th_pathname, MAXPATHLEN, "%.100s", t->th_buf.name);
	}
	else
	{
		snprintf(t->th_pathname, MAXPATHLEN, "%.155s/%.100s",
			 t->th_buf.prefix, t->th_buf.name);
	}

	/* will be deallocated in tar_close() */
	return safer_name_suffix(t->th_pathname);
}


uid_t
th_get_uid(TAR *t)
{
	int uid;
	struct passwd *pw;

	pw = getpwnam(t->th_buf.uname);
	if (pw != NULL)
		return pw->pw_uid;

	/* if the password entry doesn't exist */
	sscanf(t->th_buf.uid, "%o", &uid);
	return uid;
}


gid_t
th_get_gid(TAR *t)
{
	int gid;
	struct group *gr;

	gr = getgrnam(t->th_buf.gname);
	if (gr != NULL)
		return gr->gr_gid;

	/* if the group entry doesn't exist */
	sscanf(t->th_buf.gid, "%o", &gid);
	return gid;
}


mode_t
th_get_mode(TAR *t)
{
	mode_t mode;

	mode = (mode_t)oct_to_int(t->th_buf.mode);
	if (! (mode & S_IFMT))
	{
		switch (t->th_buf.typeflag)
		{
		case SYMTYPE:
			mode |= S_IFLNK;
			break;
		case CHRTYPE:
			mode |= S_IFCHR;
			break;
		case BLKTYPE:
			mode |= S_IFBLK;
			break;
		case DIRTYPE:
			mode |= S_IFDIR;
			break;
		case FIFOTYPE:
			mode |= S_IFIFO;
			break;
		case AREGTYPE:
			if (t->th_buf.name[strlen(t->th_buf.name) - 1] == '/')
			{
				mode |= S_IFDIR;
				break;
			}
			/* FALLTHROUGH */
		case LNKTYPE:
		case REGTYPE:
		default:
			mode |= S_IFREG;
		}
	}

	return mode;
}


/* decode a number in GNU extension format */
off_t
gnu_size_decode(uint8_t gnu[12])
{
    if (gnu[0] != 0x80 && gnu[0] != 0xff)
		return (off_t)0;

	uint64_t u = 0;
    for (int i = 1; i < 12; i++)
        u = (u << 8) | gnu[i];

    return (off_t)u;
}


size_t
th_get_size(TAR *t)
{
	if (t->options & TAR_GNU && (uint8_t)(t->th_buf.size[0]) == 0x80)
		return (size_t)gnu_size_decode((uint8_t *)t->th_buf.size);

	return oct_to_size(t->th_buf.size);
}

