#include <u.h>
#include <libc.h>
#include <thread.h>
#include "fncs.h"

int aflag;
int randfd, nullfd;
int numprocs;
int *children, ascii[2];
Channel *cpid;

typedef
struct Args{
	char *name;
	char **args;
} Args;

void
usage(void)
{
	fprint(2, "Usage: %s [-a] [-t procs] program\n", argv0);
	exits("usage");
}

void
kill(int pid)
{
	int nfd;
	char *note = smprint( "/proc/%d/note", pid);
	nfd = open(note, OWRITE);
	//If the file does not exist, it is probably already dead
	if(nfd<0)
		return;
	write(nfd, "kill", 4);
	close(nfd);
	free(note);
}

int
sudoku(void*, char*)
{
	int i;
	for(i=0;i<numprocs;i++)
		kill(children[i]);
	return 0;
}

#define BUFFSIZE 512
void
asciiproc(void*)
{
	int i;
	char buf[BUFFSIZE];
	for(;;){
		eread(randfd, buf, BUFFSIZE);
		for(i=0;i<BUFFSIZE;i++)
			buf[i] = buf[i] % 127;
		ewrite(ascii[0], buf, BUFFSIZE);
	}
}

void
execproc(void *args)
{
	Args *a = args;
	if(aflag)
		dup(ascii[1], 0);
	else
		dup(randfd, 0);
	dup(nullfd, 1);
	dup(nullfd, 2);
	procexec(cpid, a->name, a->args);
}

void
threadmain(int argc, char **argv)
{
	int i;
	Args *a;
	Waitmsg *m;
	Channel *waitchan;
	numprocs = 0;
	aflag = 0;

	ARGBEGIN{
	case 't': numprocs = eatoi(EARGF(usage)); break;
	case 'a': aflag++; break;
	default:
		usage();
	}ARGEND
	if(numprocs == 0)
		numprocs = 1;
	if(argc == 0)
		usage();

	threadnotify(sudoku, 1);
	waitchan = threadwaitchan();
	cpid = chancreate(sizeof(int), numprocs);

	randfd = eopen("/dev/random", OREAD);
	nullfd = eopen("/dev/null", OWRITE);

	a = emalloc(sizeof(Args));
	a->name = smprint("/bin/%s", argv[0]);
	a->args = argv;

	if(aflag){
		pipe(ascii);
		proccreate(asciiproc, nil, 8192);
	}
	
	children = emalloc(sizeof(int) * numprocs);
	for(i=0;i<numprocs;i++){
		procrfork(execproc, a, 8192, RFFDG);
		recv(cpid, children+i);
	}

	for(i=0;i<numprocs;i++)
		if(recv(waitchan, &m) == 1)
			print("Pid %d exited with msg: %s\n", m->pid, m->msg);
}