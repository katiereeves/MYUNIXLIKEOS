/* Subject to change */
#include "stdio.h"
#include "stdarg.h"
#include <stdbool.h>
#include <stddef.h>
#include "stdint.h"
#include "unistd.h"

#define _PFB 4096

static int _print_i(int i, FILE *fp);
static int _print_u(unsigned int u, FILE *fp);
static int _print_f(float f, FILE *fp);
static int _print_s(char *s1, FILE *fp);
static int _print_c(char c, FILE *fp);
static int _print_x(int, FILE *fp);
static int _print_o(int, FILE *fp);
static int _print_p(void *, FILE *fp);
static int _print_e(double, FILE *fp);
static int _print_g(double, FILE *fp);
static int _print_li(long, FILE *fp);
static int _print_lu(unsigned long, FILE *fp);
static int _print_lli(long long, FILE *fp);
static int _print_llu(unsigned long long, FILE *fp);
static int _print_lf(double, FILE *fp);
static int _print_Lf(long double, FILE *fp);
static int _print_hi(short, FILE *fp);
static int _print_hhi(char, FILE *fp);
static int _print_zu(size_t, FILE *fp);

static void _flush(FILE *fp) {
    int len = (int)(fp->bptr - fp->buf);
    if (len > 0) {
        write(fp->fileno, (const char*)fp->buf, len);
        fp->bptr = fp->buf;
    }
}

__attribute__((weak)) FILE *stdout = NULL;

int vfprintf(FILE *fp, const char *restrict format, va_list args) {
    /* Write into fp's buffer, flush at end (or on \n for _IOLBF) */
    bool l  = false;
    bool ll = false;
    bool h  = false;
    bool hh = false;
    bool L  = false;
    bool z  = false;

    const char *restrict fmtptr = format;
    for (; *fmtptr; fmtptr++) {
        if (*fmtptr == '%') {
            fmtptr++;
            l = ll = h = hh = L = z = false;

            switch (*fmtptr) {
            case 'l':
                fmtptr++;
                if (*fmtptr == 'l') { ll = true; fmtptr++; }
                else                  l  = true;
                break;
            case 'h':
                fmtptr++;
                if (*fmtptr == 'h') { hh = true; fmtptr++; }
                else                  h  = true;
                break;
            case 'L': L = true; fmtptr++; break;
            case 'z': z = true; fmtptr++; break;
            default: break;
            }

            switch (*fmtptr) {
            case 'i':
            case 'd':
                if      (ll) _print_lli(va_arg(args, long long), fp);
                else if (l)  _print_li(va_arg(args, long), fp);
                else if (hh) _print_hhi((char)va_arg(args, int), fp);
                else if (h)  _print_hi((short)va_arg(args, int), fp);
                else         _print_i(va_arg(args, int), fp);
                break;
            case 'u':
                if      (ll) _print_llu(va_arg(args, unsigned long long), fp);
                else if (l)  _print_lu(va_arg(args, unsigned long), fp);
                else if (z)  _print_zu(va_arg(args, size_t), fp);
                else         _print_u(va_arg(args, unsigned int), fp);
                break;
            case 'f':
            case 'F':
                if      (L) _print_Lf(va_arg(args, long double), fp);
                else if (l) _print_lf(va_arg(args, double), fp);
                else        _print_f((float)va_arg(args, double), fp);
                break;
            case 'e':
            case 'E':
                _print_e(va_arg(args, double), fp);
                break;
            case 'g':
            case 'G':
                _print_g(va_arg(args, double), fp);
                break;
            case 's': _print_s(va_arg(args, char *), fp);  break;
            case 'c': _print_c((char)va_arg(args, int), fp); break;
            case 'x':
            case 'X': _print_x(va_arg(args, int), fp);    break;
            case 'o': _print_o(va_arg(args, int), fp);    break;
            case 'p': _print_p(va_arg(args, void *), fp); break;
            case 'n': break; /* not implemented */
            case '%': _print_c('%', fp); break;
            default:  break;
            }
            continue;
        }

        _print_c(*fmtptr, fp);

        /* line-buffered: flush on newline */
        if (*fmtptr == '\n' && fp->iobf == _IOLBF)
            _flush(fp);
    }

    /* flush whatever remains */
    if (fp->bptr > fp->buf)
        _flush(fp);

    return (int)(fp->bptr - fp->buf); /* 0 after flush, but already drained */
}

int fprintf(FILE *fp, const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(fp, format, args);
    va_end(args);
    return ret;
}

int printf(const char *restrict format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vfprintf(stdout, format, args);
    va_end(args);
    return ret;
}

static int _print_c(char c, FILE *fp) {
    *fp->bptr++ = (unsigned char)c;
    return 1;
}

static int _print_s(char *s, FILE *fp) {
    int n = 0;
    for (char *c = s; *c; c++)
        n += _print_c(*c, fp);
    return n;
}

static int _print_i(int i, FILE *fp) {
    if (i < 0) { _print_c('-', fp); i = -i; }
    if (i >= 10) _print_i(i / 10, fp);
    _print_c('0' + (i % 10), fp);
    return 0;
}

static int _print_u(unsigned int u, FILE *fp) {
    if (u >= 10) _print_u(u / 10, fp);
    _print_c('0' + (u % 10), fp);
    return 0;
}

static int _print_li(long li, FILE *fp) {
    if (li < 0) { _print_c('-', fp); li = -li; }
    if (li >= 10) _print_li(li / 10, fp);
    _print_c('0' + (li % 10), fp);
    return 0;
}

static int _print_lu(unsigned long lu, FILE *fp) {
    if (lu >= 10) _print_lu(lu / 10, fp);
    _print_c('0' + (lu % 10), fp);
    return 0;
}

static int _print_lli(long long lli, FILE *fp) {
    if (lli < 0) { _print_c('-', fp); lli = -lli; }
    if (lli >= 10) _print_lli(lli / 10, fp);
    _print_c('0' + (lli % 10), fp);
    return 0;
}

static int _print_llu(unsigned long long llu, FILE *fp) {
    if (llu >= 10) _print_llu(llu / 10, fp);
    _print_c('0' + (llu % 10), fp);
    return 0;
}

static int _print_hi(short hi, FILE *fp)   { return _print_i((int)hi, fp); }
static int _print_hhi(char hhi, FILE *fp)  { return _print_i((int)hhi, fp); }
static int _print_zu(size_t zu, FILE *fp)  { return _print_llu((unsigned long long)zu, fp); }

static int _print_x(int x, FILE *fp) {
    static const char hex[] = "0123456789abcdef";
    unsigned int u = (unsigned int)x;
    if (u >= 16) _print_x((int)(u / 16), fp);
    _print_c(hex[u % 16], fp);
    return 0;
}

static int _print_o(int o, FILE *fp) {
    unsigned int u = (unsigned int)o;
    if (u >= 8) _print_o((int)(u / 8), fp);
    _print_c('0' + (u % 8), fp);
    return 0;
}

static int _print_p(void *p, FILE *fp) {
    _print_c('0', fp);
    _print_c('x', fp);
    _print_x((int)(uintptr_t)p, fp);
    return 0;
}

static int _print_f(float f, FILE *fp)     { return _print_lf((double)f, fp); }

static int _print_lf(double lf, FILE *fp) {
    if (lf < 0) { _print_c('-', fp); lf = -lf; }
    long long int_part = (long long)lf;
    double frac_part = lf - (double)int_part;
    _print_lli(int_part, fp);
    _print_c('.', fp);
    for (int i = 0; i < 6; i++) {
        frac_part *= 10;
        int digit = (int)frac_part;
        _print_c('0' + digit, fp);
        frac_part -= digit;
    }
    return 0;
}

static int _print_Lf(long double Lf, FILE *fp) {
    if (Lf < 0) { _print_c('-', fp); Lf = -Lf; }
    long long int_part = (long long)Lf;
    long double frac_part = Lf - (long double)int_part;
    _print_lli(int_part, fp);
    _print_c('.', fp);
    for (int i = 0; i < 6; i++) {
        frac_part *= 10;
        int digit = (int)frac_part;
        _print_c('0' + digit, fp);
        frac_part -= digit;
    }
    return 0;
}

static int _print_e(double e, FILE *fp) {
    int exp = 0;
    if (e < 0) { _print_c('-', fp); e = -e; }
    if (e != 0) {
        for (; e >= 10.0; e /= 10.0) exp++;
        for (; e <  1.0;  e *= 10.0) exp--;
    }
    _print_lf(e, fp);
    _print_c('e', fp);
    if (exp < 0) { _print_c('-', fp); exp = -exp; }
    else           _print_c('+', fp);
    if (exp < 10)  _print_c('0', fp);
    _print_i(exp, fp);
    return 0;
}

static int _print_g(double g, FILE *fp) {
    double abs_g = g < 0 ? -g : g;
    int exp = 0;
    if (abs_g != 0) {
        double tmp = abs_g;
        for (; tmp >= 10.0; tmp /= 10.0) exp++;
        for (; tmp <  1.0;  tmp *= 10.0) exp--;
    }
    return (exp < -4 || exp >= 6) ? _print_e(g, fp) : _print_lf(g, fp);
}