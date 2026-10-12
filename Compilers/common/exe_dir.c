// exe_dir -- the folders a tool finds next to its own executable. See exe_dir.h.

#include "exe_dir.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
// declared here instead of pulling in <windows.h>, whose macros (IN, OUT,
// ERROR, ...) collide with names in the compilers (as console_utf8.c does)
__declspec(dllimport) unsigned long __stdcall GetModuleFileNameA(void *module, char *name, unsigned long size);
#else
#include <unistd.h>
#endif

static int make_one(const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0) return (st.st_mode & S_IFDIR) ? 0 : -1;
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

int make_dir(const char *path)       // as mkdir -p: every missing level
{
    char p[4096];
    snprintf(p, sizeof(p), "%s", path);
    for (char *c = p + 1; *c; c++)
        if ((*c == '/' || *c == '\\') && c[-1] != ':')
        {
            char keep = *c; *c = 0;
            make_one(p);
            *c = keep;
        }
    return make_one(p);
}

char *abs_path(const char *path)
{
    char buf[4096];
#ifdef _WIN32
    if (!_fullpath(buf, path, sizeof(buf))) snprintf(buf, sizeof(buf), "%s", path);
#else
    if (!realpath(path, buf)) snprintf(buf, sizeof(buf), "%s", path);
#endif
    for (char *c = buf; *c; c++) if (*c == '\\') *c = '/';
    char *r = malloc(strlen(buf) + 1);
    if (r) strcpy(r, buf);
    return r;
}

char *dir_of(const char *path)
{
    char *d = abs_path(path);
    if (!d) return NULL;
    char *s = strrchr(d, '/');
    if (s && s != d && s[-1] != ':') *s = 0;   // keep "/" and "C:/" whole
    else if (s) s[1] = 0;
    return d;
}

char *path_stem(const char *path)
{
    const char *b = path;
    for (const char *c = path; *c; c++) if (*c == '/' || *c == '\\') b = c + 1;
    const char *dot = strrchr(b, '.');
    size_t n = dot ? (size_t)(dot - b) : strlen(b);
    char *r = malloc(n + 1);
    if (r) {memcpy(r, b, n); r[n] = 0;}
    return r;
}

const char *exe_dir(void)
{
    static char dir[4096];
    static int  done = 0;
    if (done) return dir;
    done = 1;

#ifdef _WIN32
    unsigned long n = GetModuleFileNameA(NULL, dir, sizeof(dir) - 1);
    if (n == 0 || n >= sizeof(dir) - 1) {dir[0] = 0; return dir;}
    dir[n] = 0;
#else
    // /proc/self/exe names the real file, through any symbolic link on the PATH
    ssize_t n = readlink("/proc/self/exe", dir, sizeof(dir) - 1);
    if (n <= 0) {dir[0] = 0; return dir;}
    dir[n] = 0;
#endif
    for (char *c = dir; *c; c++) if (*c == '\\') *c = '/';
    char *slash = strrchr(dir, '/');
    if (slash) *slash = 0; else dir[0] = 0;
    return dir;
}

char *exe_find_dir(const char *const *candidates)
{
    const char *base = exe_dir();
    if (!base[0]) return NULL;
    for (int i = 0; candidates[i]; i++)
    {
        char path[4096];
        snprintf(path, sizeof(path), "%s/%s", base, candidates[i]);
        struct stat st;
        if (stat(path, &st) == 0 && (st.st_mode & S_IFDIR))
        {
            char *r = malloc(strlen(path) + 1);
            if (r) strcpy(r, path);
            return r;
        }
    }
    return NULL;
}
