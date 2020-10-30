#include <stdio.h>
#include <cstdarg>
#include "Log.h"
namespace utils {
sdl_log_t log_func = nullptr;

void set_log_fn(sdl_log_t fcnPtr)
{
    log_func = fcnPtr;
}

int sdl_log(int log_level...)
{
    if(log_func != nullptr)
    {
        va_list arglist;
        va_start(arglist, log_level);
        log_func(log_level, arglist);
        va_end( arglist );
    }
    return 1;
}
}
