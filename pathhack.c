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

#define functop(func)                                                         \
  int ret, saverrno = errno;                                                  \
  errno = 0;                                                                  \
  ret = orig##func;                                                           \
  if (ret != -1) {                                                            \
    errno = saverrno;                                                         \
    return ret;                                                               \
  }                                                                           \
  if (errno != ENOENT || pathname[0] == '/') return ret;                      \
  size_t len = strlen(pathname);

#define funcbody(func)                                                        \
  for (size_t i = 0; i < pathnum; i++) {                                      \
    if (len + paths[i].len > PATH_MAX) continue;                              \
    memcpy(tmp, paths[i].path, paths[i].len);                                 \
    strcat(tmp, pathname);                                                    \
    errno = saverrno;                                                         \
    ret = orig##func;                                                         \
    if (ret != -1) return ret;                                                \
  }                                                                           \
  errno = ENOENT;                                                             \
  return -1;

int access(const char *pathname, int mode) {
  functop(access(pathname, mode));
  funcbody(access(tmp, mode));
}

int chdir(const char *pathname) {
  functop(chdir(pathname));
  funcbody(chdir(tmp));
}

int chmod(const char *pathname, mode_t mode) {
  functop(chmod(pathname, mode));
  funcbody(chmod(tmp, mode));
}

int chown(const char *pathname, uid_t owner, gid_t group) {
  functop(chown(pathname, owner, group));
  funcbody(chown(tmp, owner, group));
}

int chroot(const char *pathname) {
  functop(chroot(pathname));
  funcbody(chroot(tmp));
}

int creat(const char *pathname, mode_t mode) {
  functop(creat(pathname, mode));
  funcbody(creat(tmp, mode));
}

int lchown(const char *pathname, uid_t owner, gid_t group) {
  functop(lchown(pathname, owner, group));
  funcbody(lchown(tmp, owner, group));
}

int lstat(const char *pathname, struct stat *buf) {
  functop(lstat(pathname, buf));
  funcbody(lstat(tmp, buf));
}

int open(const char *pathname, int flags) {
  functop(open(pathname, flags));
  funcbody(open(tmp, flags));
}

int rmdir(const char *pathname) {
  functop(rmdir(pathname));
  funcbody(rmdir(tmp));
}

int swapon(const char *pathname, int flags) {
  functop(swapon(pathname, flags));
  funcbody(swapon(tmp, flags));
}

int swapoff(const char *pathname) {
  functop(swapoff(pathname));
  funcbody(swapoff(tmp));
}

int stat(const char *pathname, struct stat *buf) {
  functop(stat(pathname, buf));
  funcbody(stat(tmp, buf));
}

int truncate (const char *pathname, off_t length) {
  functop(truncate(pathname, length));
  funcbody(truncate(tmp, length));
}

int umount(const char *pathname) {
  functop(umount(pathname));
  funcbody(umount(tmp));
}

int umount2(const char *pathname, int flags) {
  functop(umount2(pathname, flags));
  funcbody(umount2(tmp, flags));
}

int unlink(const char *pathname) {
  functop(unlink(pathname));
  funcbody(unlink(tmp));
}
