#define _GNU_SOURCE
#include <dlfcn.h>
#include <fnmatch.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>


int (*origopen)(const char *pathname, int flags);

__attribute__((constructor)) void foo() {
  origopen = dlsym(RTLD_NEXT, "open");
}

#define STR(x) #x
#define PATH(x) { STR(x), sizeof(STR(x)) },
static struct { const char* path; size_t len; } paths[] = {
#include "paths_gen.h"
};
static size_t pathnum = sizeof(paths) / sizeof(paths[0]);

int open(const char *pathname, int flags) {
  int fd = origopen(pathname, flags);
  if (fd > 0 || pathname[0] == '/') return fd;
  char tmp[PATH_MAX];
  size_t len = strlen(pathname);

  for (size_t i = 0; i < pathnum; i++) {
    if (len + paths[i].len + 2 > PATH_MAX) continue;
    memcpy(tmp, paths[i].path, paths[i].len);
    strcat(tmp, "/");
    strcat(tmp, pathname);

    fd = origopen(tmp, flags);
    if (fd > 0) return fd;
  }

  return -1;
}
