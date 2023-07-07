#ifndef _SECUREC_H_
#define _SECUREC_H_

#include <stdarg.h>
#include <linux/kernel.h>
#include <linux/module.h>

#ifndef errno_t
typedef int errno_t;
#endif
/* new fix: */
/*
#ifndef size_t
typedef unsigned int size_t;
#endif
*/
#ifndef EOK
#define EOK             0
#endif

#ifndef NULL
#define NULL               0L
#endif

#ifndef EINVAL
#define EINVAL             22
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Description: The memset_s function copies the value of c (converted to an unsigned char) into each of
 * the first count characters of the object pointed to by dest.
 * Parameter: dest - destination address
 * Parameter: dest_max - The maximum length of destination buffer
 * Parameter: c - the value to be copied
 * Parameter: count - copies count bytes of value to dest
 * Return:    EOK if there was no runtime-constraint violation
 */
errno_t memset_s(void *dest, size_t dest_max, int c, size_t count);

/*
 * Description: The memmove_s function copies n characters from the object pointed to by src
 * into the object pointed to by dest.
 * Parameter: dest - destination  address
 * Parameter: dest_max - The maximum length of destination buffer
 * Parameter: src - source address
 * Parameter: count - copies count bytes from the src
 * Return:    EOK if there was no runtime-constraint violation
 */
errno_t memmove_s(void *dest, size_t dest_max, const void *src, size_t count);

/*
 * Description: The memcpy_s function copies n characters from the object pointed to
 * by src into the object pointed to by dest.
 * Parameter: dest - destination  address
 * Parameter: dest_max - The maximum length of destination buffer
 * Parameter: src - source address
 * Parameter: count - copies count bytes from the  src
 * Return:    EOK if there was no runtime-constraint violation
 */
errno_t memcpy_s(void *dest, size_t dest_max, const void *src, size_t count);

/*
 * Description: The strcpy_s function copies the string pointed to by str_src (including
 * the terminating null character) into the array pointed to by str_dest
 * Parameter: str_dest - destination  address
 * Parameter: dest_max - The maximum length of destination buffer(including the terminating null character)
 * Parameter: str_src - source address
 * Return:    EOK if there was no runtime-constraint violation
 */
errno_t strcpy_s(char *str_dest, size_t dest_max, const char *str_src);

/*
 * Description: The strncpy_s function copies not more than n successive characters (not including
 * the terminating null character) from the array pointed to by str_src to the array pointed to by str_dest.
 * Parameter: str_dest - destination  address
 * Parameter: dest_max - The maximum length of destination buffer(including the terminating null character)
 * Parameter: str_src - source  address
 * Parameter: count - copies count characters from the src
 * Return:    EOK if there was no runtime-constraint violation
 */
errno_t strncpy_s(char *str_dest, size_t dest_max, const char *str_src, size_t count);

/*
 * Description: The strcat_s function appends a copy of the string pointed to by str_src (including
 * the terminating null character) to the end of the string pointed to by str_dest.
 * Parameter: str_dest - destination  address
 * Parameter: dest_max - The maximum length of destination buffer(including the terminating null wide character)
 * Parameter: str_src - source address
 * Return:    EOK if there was no runtime-constraint violation
 */
errno_t strcat_s(char *str_dest, size_t dest_max, const char *str_src);

/*
 * Description: The vsprintf_s function is equivalent to the vsprintf function except for the parameter dest_max
 * and the explicit runtime-constraints violation
 * Parameter: str_dest -  produce output according to a format,write to the character string str_dest.
 * Parameter: dest_max - The maximum length of destination buffer(including the terminating null wide character)
 * Parameter: format - format string
 * Parameter: arg_list - instead of a variable number of arguments
 * Return:    the number of characters printed(not including the terminating null byte '\0'),
 * If an error occurred Return: -1.
 */
int vsprintf_s(char *str_dest, size_t dest_max, const char *format, va_list arg_list);


/*
 * Description: The sprintf_s function is equivalent to the sprintf function except for the parameter dest_max
 * and the explicit runtime-constraints violation
 * Parameter: str_dest -  produce output according to a format ,write to the character string str_dest.
 * Parameter: dest_max - The maximum length of destination buffer(including the terminating null byte '\0')
 * Parameter: format - format string
 * Return:    the number of characters printed(not including the terminating null byte '\0'),
 * If an error occurred Return: -1.
*/
int sprintf_s(char *str_dest, size_t dest_max, const char *format, ...);

/*
 * Description: The vsnprintf_s function is equivalent to the vsnprintf function except for
 * the parameter dest_max/count and the explicit runtime-constraints violation
 * Parameter: str_dest -  produce output according to a format ,write to the character string str_dest.
 * Parameter: dest_max - The maximum length of destination buffer(including the terminating null  byte '\0')
 * Parameter: count - do not write more than count bytes to str_dest(not including the terminating null  byte '\0')
 * Parameter: format - format string
 * Parameter: arg_list - instead of  a variable number of arguments
 * Return:    the number of characters printed(not including the terminating null byte '\0'),
 * If an error occurred Return: -1.Pay special attention to returning -1 when truncation occurs.
 */
int vsnprintf_s(char *str_dest, size_t dest_max, size_t count, const char *format, va_list arg_list);

/*
 * Description: The snprintf_s function is equivalent to the snprintf function except for
 * the parameter dest_max/count and the explicit runtime-constraints violation
 * Parameter: str_dest - produce output according to a format ,write to the character string str_dest.
 * Parameter: dest_max - The maximum length of destination buffer(including the terminating null  byte '\0')
 * Parameter: count - do not write more than count bytes to str_dest(not including the terminating null  byte '\0')
 * Parameter: format - format string
 * Return:    the number of characters printed(not including the terminating null byte '\0'),
 * If an error occurred Return: -1.Pay special attention to returning -1 when truncation occurs.
 */
int snprintf_s(char *str_dest, size_t dest_max, size_t count, const char *format, ...);

/*
 * Description: The sscanf_s function is equivalent to fscanf_s, except that input is obtained from a
 * string (specified by the argument buffer) rather than from a stream
 * Parameter: buffer - read character from buffer
 * Parameter: format - format string
 * Return:    the number of input items assigned, If an error occurred Return: -1.
 */
int sscanf_s(const char *buffer, const char *format, ...);

#ifdef __cplusplus
}
#endif
#endif

