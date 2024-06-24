#pragma once

/*TODO: add guards to check if defines are within range of supported values
 *for each group
 */

#define KS_BACKEND_PLATFORM_LINUX 1
//TODO:
//#define KS_BACKEND_PLATFORM_WINDOWS 2
//#define KS_BACKEND_PLATFORM_MACOS 3
//#define KS_BACKEND_PLATFORM_FREEBSD 4
//#define KS_BACKEND_PLATFORM_NETBSD 5

//ensure `KS_BACKEND_PLATFORM` is defined
#ifndef KS_BACKEND_PLATFORM
 #error \
  "KS_BACKEND_PLATFORM is not defined, platform identification failed!"
#endif

//ensure `KS_BACKEND_PLATFORM` is set to a supported value
#if \
 KS_BACKEND_PLATFORM < KS_BACKEND_PLATFORM_LINUX \
 || KS_BACKEND_PLATFORM > KS_BACKEND_PLATFORM_NETBSD
 #error "KS_BACKEND_PLATFORM is set to an unknown value!"
#endif

#define KS_BACKEND_LIB_FREESTANDING 1
#define KS_BACKEND_LIB_LIBC 2
#define KS_BACKEND_LIB_NULIB 3
#define KS_BACKEND_LIB_BIONIC 4

//ensure `KS_BACKEND_LIB` is defined
#ifndef KS_BACKEND_LIB
 #error \
  "KS_BACKEND_LIB is not defined, runtime library identification failed!"
#endif

//ensure `KS_BACKEND_LIB` is set to a supported value
#if \
 KS_BACKEND_LIB < KS_BACKEND_LIB_FREESTANDING \
 || KS_BACKEND_LIB > KS_BACKEND_LIB_BIONIC
 #error \
  "KS_BACKEND_LIB is set to an unknown value!"
#endif
