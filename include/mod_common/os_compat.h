#ifndef CPP_TOOLKIT_OS_COMPAT_H
#define CPP_TOOLKIT_OS_COMPAT_H

#if defined(_MSC_VER)
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#endif

#endif //CPP_TOOLKIT_OS_COMPAT_H
