// LD_PRELOAD wrapper to resolve relative paths to other directories than .
#define _GNU_SOURCE
#include <dlfcn.h>
#include <fnmatch.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/vfs.h>
#include <sys/xattr.h>

#define str(x) #x
#define STR(x) #x "/"
#define PATH(x) { STR(x), sizeof(STR(x)) },
static struct { const char* path; size_t len; } paths[] = {
#include "paths_gen.h"
};
static size_t pathnum = sizeof(paths) / sizeof(paths[0]);

char tmp[PATH_MAX];

#define init(name, ...) int (*orig##name)(__VA_ARGS__)
init(access, const char *pathname, int mode);
init(chdir, const char *pathname);
init(chmod, const char *pathname, mode_t mode);
init(chown, const char *pathname, uid_t owner, gid_t group);
init(chroot, const char *pathname);
init(creat, const char *pathname, mode_t mode);
init(getxattr, const char *pathname, const char *name, void *value, size_t size);
init(lchown, const char *pathname, uid_t owner, gid_t group);
init(lgetxattr, const char *pathname, const char *name, void *value, size_t size);
init(listxattr, const char *pathname, char *list, size_t size);
init(llistxattr, const char *pathname, char *list, size_t size);
init(lremovexattr, const char *path, const char *name);
init(lsetxattr, const char *pathname, const char *name, const void *value, size_t size, int flags);
init(lstat, const char *pathname, struct stat *buf);
init(open, const char *pathname, int flags);
init(readlink, const char *pathname, char *buf, size_t bufsiz);
init(removexattr, const char *pathname, const char *name);
init(rmdir, const char *pathname);
init(setxattr, const char *pathname, const char *name, const void *value, size_t size, int flags);
init(stat, const char *pathname, struct stat *buf);
init(statfs, const char *pathname, struct statfs *buf);
init(swapon, const char *pathname, int flags);
init(swapoff, const char *pathname);
init(truncate, const char *pathname, off_t length);
init(unlink, const char *pathname);
init(umount, const char *pathname);
init(umount2, const char *pathname, int flags);
init(utime, const char *pathname, const struct utimbuf *times);
init(utimes, const char *pathname, const struct timeval times[2]);

#define construct(name) orig##name = dlsym(RTLD_NEXT, str(name));
__attribute__((constructor)) void foo() {
  construct(access);
  construct(chdir);
  construct(chmod);
  construct(chown);
  construct(chroot);
  construct(creat);
  construct(getxattr);
  construct(lchown);
  construct(lgetxattr);
  construct(listxattr);
  construct(llistxattr);
  construct(lsetxattr);
  construct(lstat);
  construct(open);
  construct(readlink);
  construct(rmdir);
  construct(setxattr);
  construct(stat);
  construct(swapon);
  construct(swapoff);
  construct(truncate);
  construct(unlink);
  construct(umount);
  construct(umount2);
  construct(utime);
  construct(utimes);
}
/* todo?
 * link
 * mknod
 * mkdir
 * rename
 * symlink
 * ...
 * *at versions (paths are actually absolute?)
 * *32 / *64 versions (handled by glibc?)
 */

#define typefunc(type, f, ...)                                                \
  int saverrno = errno;                                                       \
  errno = 0;                                                                  \
  type ret = orig##f(pathname, ##__VA_ARGS__);                                \
  if (ret != -1) {                                                            \
    errno = saverrno;                                                         \
    return ret;                                                               \
  }                                                                           \
  if (errno != ENOENT || pathname[0] == '/') return ret;                      \
  size_t len = strlen(pathname);                                              \
                                                                              \
  for (size_t i = 0; i < pathnum; i++) {                                      \
    if (len + paths[i].len > PATH_MAX) continue;                              \
    memcpy(tmp, paths[i].path, paths[i].len);                                 \
    strcat(tmp, pathname);                                                    \
    errno = saverrno;                                                         \
    ret = orig##f(tmp, ##__VA_ARGS__);                                        \
    if (ret != -1) return ret;                                                \
  }                                                                           \
  errno = ENOENT;                                                             \
  return -1;

#define func(...) typefunc(int, __VA_ARGS__)

int access(const char *pathname, int mode) {
  func(access, mode);
}

int chdir(const char *pathname) {
  func(chdir);
}

int chmod(const char *pathname, mode_t mode) {
  func(chmod, mode);
}

int chown(const char *pathname, uid_t owner, gid_t group) {
  func(chown, owner, group);
}

int chroot(const char *pathname) {
  func(chroot);
}

int creat(const char *pathname, mode_t mode) {
  func(creat, mode);
}

ssize_t getxattr(const char *pathname, const char *name, void *value, size_t size) {
  typefunc(ssize_t, getxattr, name, value, size);
}

int lchown(const char *pathname, uid_t owner, gid_t group) {
  func(lchown, owner, group);
}

ssize_t lgetxattr(const char *pathname, const char *name, void *value, size_t size) {
  typefunc(ssize_t, lgetxattr, name, value, size);
}

ssize_t listxattr(const char *pathname, char *list, size_t size) {
  typefunc(ssize_t, listxattr, list, size);
}

ssize_t llistxattr(const char *pathname, char *list, size_t size) {
  typefunc(ssize_t, llistxattr, list, size);
}

int lremovexattr(const char *pathname, const char *name) {
  func(lremovexattr, name);
}

int lsetxattr(const char *pathname, const char *name, const void *value, size_t size, int flags) {
  func(lsetxattr, name, value, size, flags);
}

int lstat(const char *pathname, struct stat *buf) {
  func(lstat, buf);
}

int open(const char *pathname, int flags) {
  func(open, flags);
}

ssize_t readlink(const char *pathname, char *buf, size_t bufsiz) {
  typefunc(ssize_t, readlink, buf, bufsiz);
}

int removexattr(const char *pathname, const char *name) {
  func(removexattr, name);
}

int rmdir(const char *pathname) {
  func(rmdir);
}

int swapon(const char *pathname, int flags) {
  func(swapon, flags);
}

int swapoff(const char *pathname) {
  func(swapoff);
}

int stat(const char *pathname, struct stat *buf) {
  func(stat, buf);
}

int setxattr(const char *pathname, const char *name, const void *value, size_t size, int flags) {
  func(setxattr, name, value, size, flags);
}

int truncate(const char *pathname, off_t length) {
  func(truncate, length);
}

int unlink(const char *pathname) {
  func(unlink);
}

int umount(const char *pathname) {
  func(umount);
}

int umount2(const char *pathname, int flags) {
  func(umount2, flags);
}

int utime(const char *pathname, const struct utimbuf *times) {
  func(utime, times);
}

int utimes(const char *pathname, const struct timeval times[2]) {
  func(utimes, times);
}
