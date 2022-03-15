


typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)


//
// stdio.h
//

struct _IO_FILE;
typedef struct _IO_FILE FILE;
extern FILE *stdin;  /* Standard input stream.  */
extern FILE *stdout; /* Standard output stream.  */
extern FILE *stderr; /* Standard error output stream.  */
extern int fclose(FILE *__stream);
extern int fflush (FILE *__stream);
extern FILE *fopen(const char *__filename, const char *__modes);
extern FILE *open_memstream (char **__bufloc, long *__sizeloc);
extern int fprintf(FILE *__stream, const char *__format, ...);
extern int sprintf (char *__s,const char *__format, ...);
extern int vfprintf(FILE *__s, const char *__format, __gnuc_va_list __arg);
extern long fread(void *__ptr, long __size, long __n, FILE *__stream);
extern long fwrite (const void *__ptr, long __size, long __n, FILE *__s);

// extern int fclose (FILE *__stream);
// extern int fflush (FILE *__stream);
// extern FILE *fopen (const char *__restrict __filename,
//       const char *__restrict __modes) ;
// extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) __attribute__ ((__nothrow__ , __leaf__)) ;
// extern int fprintf (FILE *__restrict __stream,
//       const char *__restrict __format, ...);
// extern int sprintf (char *__restrict __s,
//       const char *__restrict __format, ...) __attribute__ ((__nothrow__));
// extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
//        __gnuc_va_list __arg);
// extern size_t fread (void *__restrict __ptr, size_t __size,
//        size_t __n, FILE *__restrict __stream) ;
// extern size_t fwrite (const void *__restrict __ptr, size_t __size,
//         size_t __n, FILE *__restrict __s);


//
// string.h
//

extern char *strncpy (char *__dest, const char *__src, long __n);
extern int memcmp(const void *__s1, const void *__s2, long __n);
extern int strcmp(const char *__s1, const char *__s2);
extern int strncmp(const char *__s1, const char *__s2, long __n);
extern long strlen(const char *__s);

// extern char *strncpy (char *__restrict __dest,
//         const char *__restrict __src, size_t __n)
//      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1, 2)));
// extern int memcmp (const void *__s1, const void *__s2, size_t __n)
//      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
// extern int strcmp (const char *__s1, const char *__s2)
//      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
// extern int strncmp (const char *__s1, const char *__s2, size_t __n)
//      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1, 2)));
// extern size_t strlen (const char *__s)
//      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));


//
// stdlib.h
//

extern long strtol(const char *__nptr, char **__endptr, int __base);
extern void *calloc(long __nmemb, long __size);
extern void exit(int __status);

// extern long int strtol (const char *__restrict __nptr,
//    char **__restrict __endptr, int __base)
//      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__nonnull__ (1)));
// extern void *calloc (size_t __nmemb, size_t __size)
//      __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__malloc__)) __attribute__ ((__alloc_size__ (1, 2))) ;
// extern void exit (int __status) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));


//
// ctype.h
//

extern int isdigit(int x);
extern int ispunct (int x);
extern int isspace(int x);

// extern int isdigit (int) __attribute__ ((__nothrow__ , __leaf__));
// extern int ispunct (int) __attribute__ ((__nothrow__ , __leaf__));
// extern int isspace (int) __attribute__ ((__nothrow__ , __leaf__));



#define NULL 0

// bool
#define bool int
#define true 1
#define false 0