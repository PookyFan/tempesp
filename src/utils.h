#ifndef __TEMPESP_UTILS_H__
#define __TEMPESP_UTILS_H__

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*a))

#define ALIGNAS(t) __attribute__((aligned(sizeof(t))))

#define S_IN_US *1000000

#endif