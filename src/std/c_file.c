/*
 * cynapses libc functions
 *
 * Copyright (c) 2008 by Andreas Schneider <mail@cynapses.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * vim: ts=2 sw=2 et cindent
 */

#ifdef _WIN32
#include <windef.h>
#include <winbase.h>
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "c_file.h"
#include "c_string.h"

#include "c_private.h"

/* check if path is a file */
int c_isfile(const char *path) {
  csync_stat_t sb;
  _TCHAR *wpath = c_multibyte(path);
  int re = _tstat(wpath, &sb);
  c_free_multibyte(wpath);
  
  if (re< 0) {
    return 0;
  }

#ifdef __unix__
  if (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode)) {
#else
  if (S_ISREG(sb.st_mode)) {
#endif
    return 1;
  }

  return 0;
}

/* copy file from src to dst, overwrites dst */
#ifdef _WIN32
int c_copy(const char* src, const char *dst, mode_t mode) {
  int rc = -1;
  _TCHAR *wsrc = 0;
  _TCHAR *wdst = 0;
  (void) mode; /* unused on win32 */
  if(src && dst) {
    wsrc = c_multibyte(src);
    wdst = c_multibyte(dst);
    if (CopyFileW(wsrc, wdst, FALSE)) {
      rc = 0;
    }
    c_free_multibyte(wsrc);
    c_free_multibyte(wdst);
    if( rc < 0 ) {
      errno = GetLastError();
    }
  }
  return rc;
}
#else
int c_copy(const char* src, const char *dst, mode_t mode) {
  int srcfd = -1;
  int dstfd = -1;
  int rc = -1;
  ssize_t bread, bwritten;
  csync_stat_t sb;
  char buf[4096];

  /* Win32 does not come here. */
  if (c_streq(src, dst)) {
    return -1;
  }

  if (lstat(src, &sb) < 0) {
    return -1;
  }

  if (S_ISDIR(sb.st_mode)) {
    errno = EISDIR;
    return -1;
  }

  if (mode == 0) {
    mode = sb.st_mode;
  }

  if (lstat(dst, &sb) == 0) {
    if (S_ISDIR(sb.st_mode)) {
      errno = EISDIR;
      return -1;
    }
  }

  if ((srcfd = open(src, O_RDONLY, 0)) < 0) {
    rc = -1;
    goto out;
  }

  if ((dstfd = open(dst, O_CREAT|O_WRONLY|O_TRUNC, mode)) < 0) {
    rc = -1;
    goto out;
  }

  for (;;) {
    bread = read(srcfd, buf, sizeof(buf));
    if (bread == 0) {
      /* done */
      break;
    } else if (bread < 0) {
      errno = ENODATA;
      rc = -1;
      goto out;
    }

    bwritten = write(dstfd, buf, bread);
    if (bwritten < 0) {
      errno = ENODATA;
      rc = -1;
      goto out;
    }

    if (bread != bwritten) {
      errno = EFAULT;
      rc = -1;
      goto out;
    }
  }

#ifdef __unix__
  fsync(dstfd);
#endif

  rc = 0;
out:
  if (srcfd >= 0) {
    close(srcfd);
  }
  if (dstfd >= 0) {
      close(dstfd);
  }
  if (rc < 0) {
    unlink(dst);
  }
  return rc;
}
#endif // _WIN32

int c_rename( const char *src, const char *dst ) {
    _TCHAR *nuri = NULL;
    _TCHAR *ouri = NULL;
    int rc = 0;

    nuri = c_multibyte(dst);
    if (nuri == NULL) {
        return -1;
    }

    ouri = c_multibyte(src);
    if (ouri == NULL) {
	c_free_multibyte(nuri);
        return -1;
    }

#ifdef _WIN32
    {
#define MAX_TRIES_RENAME 3
        int err = 0;
        int cnt = 0;

        do {
            BOOL ok;
            ok = MoveFileExW(ouri,
                             nuri,
                             MOVEFILE_COPY_ALLOWED +
                             MOVEFILE_REPLACE_EXISTING +
                             MOVEFILE_WRITE_THROUGH);
            if (!ok) {
                /* error */
                err = GetLastError();
                if( (err == ERROR_ACCESS_DENIED   ||
                     err == ERROR_LOCK_VIOLATION  ||
                     err == ERROR_SHARING_VIOLATION) && cnt < MAX_TRIES_RENAME ) {
                    cnt++;
                    Sleep(cnt*100);
                    continue;
                }
            }
            break;
        } while( 1 );
        if( err != 0 ) {
            errno = err;
        }
    }
#else
    rc = rename(ouri, nuri);
#endif

    c_free_multibyte(nuri);
    c_free_multibyte(ouri);

    return rc;
}

int c_compare_file( const char *f1, const char *f2 ) {
  _TCHAR *wf1, *wf2;
  int fd1 = -1, fd2 = -1;
  size_t size1, size2;
  char buffer1[BUFFER_SIZE];
  char buffer2[BUFFER_SIZE];
  csync_stat_t stat1;
  csync_stat_t stat2;

  int rc = -1;

  if(f1 == NULL || f2 == NULL) return -1;

  wf1 = c_multibyte(f1);
  if(wf1 == NULL) {
    return -1;
  }
  wf2 = c_multibyte(f2);
  if(wf2 == NULL) {
    return -1;
  }

  /* compare size first. */
  rc = _tstat(wf1, &stat1);
  if(rc< 0) {
    goto out;
  }
  rc = _tstat(wf2, &stat2);
  if(rc < 0) {
    goto out;
  }

  /* if sizes are different, the files can not be equal. */
  if( stat1.st_size != stat2.st_size ) {
    rc = 0;
    goto out;
  }

#ifdef _WIN32
  _fmode = _O_BINARY;
#endif

  fd1 = _topen(wf1, O_RDONLY);
  if(fd1 < 0) {
    rc = -1;
    goto out;
  }
  fd2 = _topen(wf2, O_RDONLY);
  if(fd2 < 0) {
    rc = -1;
    goto out;
  }

  while( (size1 = read(fd1, buffer1, BUFFER_SIZE)) > 0 ) {
    size2 = read( fd2, buffer2, BUFFER_SIZE );

    if( size1 != size2 ) {
      rc = 0;
      goto out;
    }
    if(memcmp(buffer1, buffer2, size1) != 0) {
      /* buffers are different */
      rc = 0;
      goto out;
    }
  }

  rc = 1;

out:

  if(fd1 > -1) close(fd1);
  if(fd2 > -1) close(fd2);

  c_free_multibyte(wf1);
  c_free_multibyte(wf2);
  return rc;

}

