#ifndef __LIBS_STRING_H__
#define __LIBS_STRING_H__
#include <u.h>

void *memcpy(void *d, void *s, uint n) {
  asm (LL,8); asm (LBL, 16); asm (LCL,24); asm (MCPY); asm (LL,8);
}

void *memset(void *d, uint c,  uint n) {
  asm (LL,8); asm (LBLB,16); asm (LCL,24); asm (MSET); asm (LL,8);
}

void *memchr(void *s, uint c,  uint n) {
  asm (LL,8); asm (LBLB,16); asm (LCL,24); asm (MCHR);
}

xstrncpy(char *s, char *t, int n) {       // no return value unlike strncpy XXX remove me only called once
  while (n-- > 0 && (*s++ = *t++));
  while (n-- > 0) *s++ = 0;
}

safestrcpy(char *s, char *t, int n) {     // like strncpy but guaranteed to null-terminate.
  if (n <= 0) return;
  while (--n > 0 && (*s++ = *t++));
  *s = 0;
}

/* *
 * strlen - calculate the length of the string @s, not including
 * the terminating '\0' character.
 * @s:      the input string
 *
 * The strlen() function returns the length of string @s.
 * */
size_t
strlen(char *s) {
  size_t cnt = 0;
  while (*s != '\0') {
    s++;
    cnt++;
  }
  return cnt;
}

/* *
 * strnlen - calculate the length of the string @s, not including
 * the terminating '\0' char acter, but at most @len.
 * @s:      the input string
 * @len:    the max-length that function will scan
 *
 * Note that, this function looks only at the first @len characters
 * at @s, and never beyond @s + @len.
 *
 * The return value is strlen(s), if that is less than @len, or
 * @len if there is no '\0' character among the first @len characters
 * pointed by @s.
 * */
size_t
strnlen(char *s, size_t len) {
  size_t cnt = 0;
  while (cnt < len && *s++ != '\0') {
    cnt++;
  }
  return cnt;
}

/* *
 * strcpy - copies the string pointed by @src into the array pointed by @dst,
 * including the terminating null character.
 * @dst:    pointer to the destination array where the content is to be copied
 * @src:    string to be copied
 *
 * The return value is @dst.
 *
 * To avoid overflows, the size of array pointed by @dst should be long enough to
 * contain the same string as @src (including the terminating null character), and
 * should not overlap in memory with @src.
 * */
char *
strcpy(char *dst, char *src) {
#ifdef __HAVE_ARCH_STRCPY
  return __strcpy(dst, src);
#else
  char *p = dst;
  while ((*p++ = *src++) != '\0')
    /* nothing */;
  return dst;
#endif /* __HAVE_ARCH_STRCPY */
}

/* *
 * strncpy - copies the first @len characters of @src to @dst. If the end of string @src
 * if found before @len characters have been copied, @dst is padded with '\0' until a
 * total of @len characters have been written to it.
 * @dst:    pointer to the destination array where the content is to be copied
 * @src:    string to be copied
 * @len:    maximum number of characters to be copied from @src
 *
 * The return value is @dst
 * */
char *
strncpy(char *dst, char *src, size_t len) {
  char *p = dst;
  while (len > 0) {
    if ((*p = *src) != '\0') {
      src++;
    }
    p++, len--;
  }
  return dst;
}

/* *
 * strcmp - compares the string @s1 and @s2
 * @s1:     string to be compared
 * @s2:     string to be compared
 *
 * This function starts comparing the first character of each string. If
 * they are equal to each other, it continues with the following pairs until
 * the characters differ or until a terminanting null-character is reached.
 *
 * Returns an integral value indicating the relationship between the strings:
 * - A zero value indicates that both strings are equal;
 * - A value greater than zero indicates that the first character that does
 *   not match has a greater value in @s1 than in @s2;
 * - And a value less than zero indicates the opposite.
 * */
int
strcmp(char *s1, char *s2) {
#ifdef __HAVE_ARCH_STRCMP
  return __strcmp(s1, s2);
#else
  while (*s1 != '\0' && *s1 == *s2) {
    s1++, s2++;
  }
  return (int)((unsigned char)*s1 - (unsigned char)*s2);
#endif /* __HAVE_ARCH_STRCMP */
}

/* *
 * strncmp - compares up to @n characters of the string @s1 to those of the string @s2
 * @s1:     string to be compared
 * @s2:     string to be compared
 * @n:      maximum number of characters to compare
 *
 * This function starts comparing the first character of each string. If
 * they are equal to each other, it continues with the following pairs until
 * the characters differ, until a terminating null-character is reached, or
 * until @n characters match in both strings, whichever happens first.
 * */
int
strncmp(char *s1, char *s2, size_t n) {
  while (n > 0 && *s1 != '\0' && *s1 == *s2) {
    n--, s1++, s2++;
  }
  return (n == 0) ? 0 : (int)((unsigned char)*s1 - (unsigned char)*s2);
}

/* *
 * strchr - locates first occurrence of character in string
 * @s:      the input string
 * @c:      character to be located
 *
 * The strchr() function returns a pointer to the first occurrence of
 * character in @s. If the value is not found, the function returns 'NULL'.
 * */
char *
strchr(char *s, char c) {
  while (*s != '\0') {
    if (*s == c) {
      return (char *)s;
    }
    s++;
  }
  return NULL;
}

/* *
 * strfind - locates first occurrence of character in string
 * @s:      the input string
 * @c:      character to be located
 *
 * The strfind() function is like strchr() except that if @c is
 * not found in @s, then it returns a pointer to the null byte at the
 * end of @s, rather than 'NULL'.
 * */
char *
strfind(char *s, char c) {
  while (*s != '\0') {
    if (*s == c) {
      break;
    }
    s++;
  }
  return (char *)s;
}

#endif /* !__LIBS_STRING_H__ */
