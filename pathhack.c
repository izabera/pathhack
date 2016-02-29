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

#define str(x) #x
#define STR(x) #x "/"
#define PATH(x) { STR(x), sizeof(STR(x)) },
static struct { const char* path; size_t len; } paths[] = {
#include "paths_gen.h"
};
static size_t pathnum = sizeof(paths) / sizeof(paths[0]);

char tmp[PATH_MAX];

#define proto(name, ...) int (*orig##name)(__VA_ARGS__)
proto(access, const char *pathname, int mode);
proto(chdir, const char *pathname);
proto(chmod, const char *pathname, mode_t mode);
proto(chown, const char *pathname, uid_t owner, gid_t group);
proto(chroot, const char *pathname);
proto(creat, const char *pathname, mode_t mode);
proto(lchown, const char *pathname, uid_t owner, gid_t group);
proto(lstat, const char *pathname, struct stat *buf);
proto(open, const char *pathname, int flags);
proto(rmdir, const char *pathname);
proto(stat, const char *pathname, struct stat *buf);
proto(swapon, const char *pathname, int flags);
proto(swapoff, const char *pathname);
proto(truncate, const char *pathname, off_t length);
proto(umount, const char *pathname);
proto(umount2, const char *pathname, int flags);
proto(unlink, const char *pathname);

#define construct(name) orig##name = dlsym(RTLD_NEXT, str(name));
__attribute__((constructor)) void foo() {
  construct(access);
  construct(chdir);
  construct(chmod);
  construct(chown);
  construct(chroot);
  construct(creat);
  construct(lchown);
  construct(lstat);
  construct(open);
  construct(rmdir);
  construct(stat);
  construct(swapon);
  construct(swapoff);
  construct(truncate);
  construct(umount);
  construct(umount2);
  construct(unlink);
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
 * realink (returns ssize_t)
 */

#define func(f, target, ...)                                                  \
  int ret, saverrno = errno;                                                  \
  errno = 0;                                                                  \
  ret = orig##f(target, ##__VA_ARGS__);                                       \
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

int access(const char *pathname, int mode) {
  func(access, pathname, mode);
}

int chdir(const char *pathname) {
  func(chdir, pathname);
}

int chmod(const char *pathname, mode_t mode) {
  func(chmod, pathname, mode);
}

int chown(const char *pathname, uid_t owner, gid_t group) {
  func(chown, pathname, owner, group);
}

int chroot(const char *pathname) {
  func(chroot, pathname);
}

int creat(const char *pathname, mode_t mode) {
  func(creat, pathname, mode);
}

int lchown(const char *pathname, uid_t owner, gid_t group) {
  func(lchown, pathname, owner, group);
}

int lstat(const char *pathname, struct stat *buf) {
  func(lstat, pathname, buf);
}

int open(const char *pathname, int flags) {
  func(open, pathname, flags);
}

int rmdir(const char *pathname) {
  func(rmdir, pathname);
}

int swapon(const char *pathname, int flags) {
  func(swapon, pathname, flags);
}

int swapoff(const char *pathname) {
  func(swapoff, pathname);
}

int stat(const char *pathname, struct stat *buf) {
  func(stat, pathname, buf);
}

int truncate (const char *pathname, off_t length) {
  func(truncate, pathname, length);
}

int umount(const char *pathname) {
  func(umount, pathname);
}

int umount2(const char *pathname, int flags) {
  func(umount2, pathname, flags);
}

int unlink(const char *pathname) {
  func(unlink, pathname);
}
