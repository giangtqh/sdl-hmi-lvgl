#ifndef _SDL2W_UTIL_LOG_H
#define _SDL2W_UTIL_LOG_H

#include <stdio.h>

namespace utils {
typedef int (*sdl_log_t)(int log_level, va_list args);
void set_log_fn(sdl_log_t fcnPtr);

int sdl_log(int log_level...);

// #define LOGE(...) sdl_log(1, __VA_ARGS__)
// #define LOGW(...) sdl_log(2, __VA_ARGS__)
// #define LOG(...) sdl_log(3, __VA_ARGS__)
// #define LOGD(...) sdl_log(4, __VA_ARGS__)

#define LOGE(...) fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n")
#define LOGW(...) fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n")
#define LOG(...) fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n")
#define LOGD(...) fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n")
}
#endif  // _SDL2W_UTIL_LOG_H