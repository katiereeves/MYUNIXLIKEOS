/* Only supports IA-32, x86_64 passes args in registers, not the stack. */

#ifndef _STDARG_H_
#define _STDARG_H_

typedef char* va_list;

#define __va_align(type) \
    ((sizeof(type) + __alignof__(type) - 1) & ~(__alignof__(type) - 1))

#define va_start(ap, argN) \
    ((ap) = (char*)&(argN) + __va_align(argN))

#define va_copy(dest, src) \
    ((dest) = (src))

#define va_arg(ap, type) \
    (*(type *)(((ap) += __va_align(type)) - __va_align(type)))

#define va_end(ap) \
    ((ap) = (char *)0)

#endif /* _STDARG_H_ */