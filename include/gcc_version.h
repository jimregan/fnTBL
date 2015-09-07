#ifndef __GCC_VERSION_H__

#ifndef HAVE_GCC_VERSION
#define HAVE_GCC_VERSION(MAJOR, MINOR) \
  (__GNUC__ > (MAJOR) || (__GNUC__ == (MAJOR) && __GNUC_MINOR__ >= (MINOR)))
#endif

#endif
