// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#include <stdio.h>
#include "raylib.h"

#ifdef _DEBUG
    #define LOG(...) printf(__VA_ARGS__)
    #define LOG_DEBUG(...) printf(__VA_ARGS__)
#elif defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
    #define LOG_DEBUG(...)
#else 
    #define LOG(...)
    #define LOG_DEBUG(...)
#endif

