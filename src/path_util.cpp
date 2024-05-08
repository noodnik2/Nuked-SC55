#include "path_util.h"
#include <string_view>

#ifdef _WIN32
#include <Windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

std::filesystem::path P_GetProcessPath()
{
#ifdef _WIN32
    char path[MAX_PATH];
    DWORD actual_size = GetModuleFileNameA(NULL, path, sizeof(path));
    if (actual_size == 0)
    {
        // TODO: handle error
        exit(1);
    }
#else
    char path[PATH_MAX];
    ssize_t actual_size = readlink("/proc/self/exe", result, sizeof(path));
    if (actual_size == -1)
    {
        // TODO: handle error
        exit(1);
    }
#endif
    return std::filesystem::path(std::string_view(path, (size_t)actual_size));
}
