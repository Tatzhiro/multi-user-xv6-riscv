#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

/**
 * @brief compare p and q up to size letters
 *
 * @param p
 * @param q
 * @param size
 * @return 0 if p and q have same strings, otherwise 1
 */
int
strncmp(const char *p, const char *q, const int size)
{
  for(int i = 0; i < size; i++) {
    if (*p != *q) {
      return 1;
    }
    p++, q++;
  }
  return 0;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  char *cdst = (char *) dst;
  int i;
  for(i = 0; i < n; i++){
    cdst[i] = c;
  }
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  if (src > dst) {
    while(n-- > 0)
      *dst++ = *src++;
  } else {
    dst += n;
    src += n;
    while(n-- > 0)
      *--dst = *--src;
  }
  return vdst;
}

int
memcmp(const void *s1, const void *s2, uint n)
{
  const char *p1 = s1, *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

void *
memcpy(void *dst, const void *src, uint n)
{
  return memmove(dst, src, n);
}

int
isdigit(int c) {
  return '0' <= c && c <= '9';

}

int
isxdigit(int c)
{
  return isdigit(c) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'));
}

int
toupper(int c)
{
	//	英小文字の場合は英大文字へ変換
	if ((c >= 'a') && (c <= 'z'))
	{
		return c - 0x20;
	}

	return c;
}

void
myxtoa(unsigned long long val, char *str, int base, int negative)
{
  char *p;
  char *firstdig;
  char temp;
  unsigned long long digval;

  if (base != 0 && !(base >= 2 && base <= 36)) {
    write(1, "Error: invalid base\n", 21);
    return;
  }

  p = str;

  if (negative) {
    *p++ = '-';
    val = (unsigned long long)(-(long long) val);
  }

  if (!base) {
    base = 10;

    if (*str == '0') {
      base = 8;
      str++;

      if ((toupper(*str) == 'X') && isxdigit(str[1])) {
        str++;
        base = 16;
      }
    }
  } else if (base == 16) {
    if (str[0] == '0' && toupper(str[1]) == 'X') {
      str += 2;
    }
  }

  firstdig = p;

  do {
    digval = (unsigned long long)(val % base);
    val /= base;

    if (digval > 9) {
      *p++ = (char)(digval - 10 + 'a');
    } else {
      *p++ = (char)(digval + '0');
    }
  } while (val > 0);

  *p-- = '\0';

  do {
    temp = *p;
    *p = *firstdig;
    *firstdig = temp;
    p--;
    firstdig++;
  } while (firstdig < p);
}

/**
 * @brief convert int value to string
 *
 * @param val int value that is to be convereted
 * @param str converted val is stored here
 * @param base default should be 10, but base 2, 8, 16 probably should be possible
 * @return char*
 */
char*
myitoa(int val, char *str, int base)
{
  if (base == 10 && val < 0) {
    myxtoa(val, str, base, 1);
  } else {
    myxtoa(val, str, base, 0);
  }

  return str;
}

char* strstr(char *str, char *substr)
{
	  while (*str)
	  {
		    char *begin = str;
		    char *pattern = substr;

		    while (*str && *pattern && *str == *pattern)
			{
			      str++;
			      pattern++;
		    }
		    if (!*pattern)
		    	  return begin;

		    str = begin + 1;
	  }
	  return 0;
}