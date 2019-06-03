#include <u.h>
#include <libc.h>

//Vendored from BurnZeZ's libautism.

long
eread(int fd, void *v, long n)
{
	if((n = read(fd, v, n)) < 0)
		sysfatal("eread");
	return n;
}

void
ewrite(int fd, void *v, long n)
{
	if(pwrite(fd, v, n, -1ll) != n)
		sysfatal("ewrite");
}

long
estrtol(char *as, char **aas, int base)
{
	long n;
	char *p;

	n = strtol(as, &p, base);
	if(p == as || *p != '\0')
		sysfatal("estrtol: bad input '%s'", as);
	else if(aas != nil)
		*aas = p;

	return n;
}

int
eatoi(char *s)
{
	return estrtol(s, nil, 10);
}

void*
emalloc(ulong sz)
{
	void *v;

	if((v = malloc(sz)) == nil)
		sysfatal("emalloc");
	memset(v, 0, sz);

	setmalloctag(v, getcallerpc(&sz));
	return v;
}

int
eopen(char *s, int m)
{
	int fd;
	if((fd = open(s, m)) < 0)
		sysfatal("eopen");

	return fd;
}
