// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#include <stdio.h>

#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif