#ifndef QORM_GLOBAL
#define QORM_GLOBAL

#include <qglobal.h>

#if defined(QORM_LIBRARY)
#  define QORM_EXPORT Q_DECL_EXPORT
#else
#  define QORM_EXPORT Q_DECL_IMPORT
#endif

#endif // QORM_GLOBAL

