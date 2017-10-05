/* 
 *  usermod 2017/09/18
 */
#include <stdio.h>          // printf()
#include <stdlib.h>          // exit()
#include <string.h>         // strlen()
#include <fcntl.h>          // O_WRONLY
#include <unistd.h>         // write(), close()
#include <errno.h>          // perror
#include <crypt.h>
#include <time.h>

#define MAX_PASSWD      40
#define MAX_COMMENT     20
#define MAX_USERNAME    17
#define MAX_USER        10
#define MAX_ID			10
#define BUFSIZE         256

#define GET_NAME        0
#define GET_PASSWD      1
#define GET_ID          2
#define GET_COMMENT     3

#define dprintf(fmt, args...) fprintf(stderr, "\x1b[33m""[%s:%d]: " "\x1b[0m" fmt, \
		__FILE__, __LINE__, ##args)

static struct {
	long offset;        // 해당 문자열 시작위치
	int len;            // 문자열 길이
} table [MAX_USER] = {}; 
int entry = 0; 
int open_shadow();
int usage();
int table_update(int fd);
int parser(int fd, char *username, char *get_string, int flags);
int make_passwd(char *inData);
int useradd(int fd, char *inname, char *incomment, char *inuid, char *inpasswd);
int main(int argc, char **argv){
	int fd; 
	int flag_n, flag_i, flag_c, flag_p, flag_cc=0;
	int c;

	char *opt_n = NULL;
	char *opt_i = NULL;
	char *opt_c = NULL;
	char *opt_p = NULL;

	while((c = getopt(argc, argv, "n:i:c:p:")) != -1){
		switch(c){
			case 'n':
				flag_n = 1;
				opt_n = optarg;	

				break;
			case 'i':
				flag_i = 1;
				opt_i = optarg;
				if(atoi(opt_i) <= 10)
					flag_cc = 1;
				else
					printf("error! please insert uid under 10");
				break;
			case 'c':
				flag_c = 1;
				opt_c = optarg;
				break;
			case 'p':
				flag_p = 1;
				opt_p = optarg;
				break;
			case '?':
				printf("\nUnknown flag\n");
				break;
		}
	}
	
	printf("n is %s\n", opt_n);
	printf("i is %s\n", opt_i);
	printf("c is %s\n", opt_c);
	printf("p is %s\n", opt_p);

	if(flag_n == 1)printf("n is on\n");
	if(flag_i == 1)printf("i is on\n");
	if(flag_c == 1)printf("c is on\n");
	if(flag_p == 1)printf("p is on\n");
	if(flag_cc == 1)printf("cc is on\n");

	if(flag_n == 1 && flag_i == 1 && flag_c == 1 && flag_p == 1 && flag_cc == 1){
		if ((fd = open_shadow()) < 0) {
			perror("open");
			exit(1);
		}   
		dprintf("file open : %d\n", fd);

		if(entry < 10){
			printf("\n------- Useradd Section --------\n");
		
			useradd(fd, opt_n, opt_c, opt_i, opt_p);
		}
		else{
			printf("\n------- Full user list ---------\n");
			printf("\n------- End Program ---------\n");

		}
	}
	else{
		usage();
	}
}
int usage(){
	printf(" ------ Usage ------- \n");
	printf(" -n : User name \n");
	printf(" -i : User ID\n");
	printf(" -c : Comment (Acccess / privildege \n");
	printf(" -p : Password\n");
}
int open_shadow()
{
	int fd;

	char *file_name  = "/etc/keti_shadow";
	if (access(file_name, F_OK) == 0) {
		/* Check there is already shadow file */
		dprintf( "%s 파일이 있습니다.\n", file_name);
		if (( fd = open(file_name, O_RDWR)) < 0)
		{
			perror("open");
			return -1;
		}
		table_update(fd);
		/* start write */
	}
	else {
		/* Make new shadow file */
		if ((fd = open(file_name, O_RDWR | O_CREAT , 0644)) < 0)
		{
			perror("open");
			return -1;
		}
		/* start write */
		//write(fd, temp, strlen( temp));      
	}
	return fd;
}

int useradd(int fd, char *inname, char *incomment, char *inuid, char *inpasswd){
	char *name = malloc(sizeof(char) * MAX_USERNAME);		// Username variable
	char *comment = malloc(sizeof(char) * MAX_COMMENT);		// User comment variable
	char *password = malloc(sizeof(char) * MAX_PASSWD);		// User password variable
	char *uid = malloc(sizeof(char) * MAX_ID);				// User specific id variable
	//char *checker = malloc(sizeof(char) * 255);				// To check username that in new_shadow_example files
	char *dummy = malloc(sizeof(char) * 100);				// finally append string (username:password:uid:comment)
	int flag_n, flag_c = 0;
	//strdump(name, 


	while(1) {
		printf("%d", parser(fd, inname, NULL, 0));
		if(parser(fd, inname, NULL, 0) == -1) {			// To check username in new_shadow_example file. If name do not exist in new_shadow_example file, return -1.
		dprintf("Name Checked!\n");
			while(1) {										// if uid exist, run loop
				if(parser(fd, NULL, inuid, 2) == 0) {   		// uid check  
					printf("Exist same id!\n");
					exit(-1);	
				}   
				else 
					break;
			}   
			make_passwd(inpasswd);							// To encrypt user password.
			printf("returned password : %s\n", inpasswd);

			sprintf(dummy, "%s:%s:%s:%s\n", inname, inpasswd, inuid, incomment);	//Insert full string in dummy variable.
			printf("final = %s", dummy);
			if(entry == 0)
				lseek(fd, 0, SEEK_SET);
			else
				lseek(fd, 0, SEEK_END);											// Append dummy variable to end of new_shadow_example file
			if(write(fd, dummy, strlen(dummy)) == -1) 
				printf("write error!");
			break;
		}   
		else{
			printf("Exist Username! please retype name");
			exit(-1);
			break;
		}   
	}   
}

int table_update(int fd) {
	int n, i, len;
	long offset;
	char buf[BUFSIZE];
	offset = 0; entry = 0; len = 0;
	lseek(fd, 0, SEEK_SET);
	while((n = read(fd, buf, BUFSIZE)) > 0) {
		for(i = 0 ; i < n ; i++) {
			len++; offset++;
			if(len > MAX_PASSWD && buf[i] == '\n') {
				table[entry].len = len;
				len = 0;
				table[++entry].offset = offset;
			}
		}
	}
}
/* usage 
 * parser(fd, username, get_string, flags) 
 * return > 0	: return line of existing information in shadow file 
 & store requested string in get_string 
 (When you give get_string NULL, just return line number)
 * return -1	: there is no username
 *
 * parser(fd, NULL, check_id, flags)
 * return 0		: there is check_id 
 * return -1	: there is no check_id
 */


int parser(int fd, char *username, char *get_string, int flags)
{       
        int i, j;
        char buf[BUFSIZE];
        char *ptr, *buf_ptr;
        
        table_update(fd);
        
        for(i = 0; i < entry; i++) {
                lseek(fd, table[i].offset, 0); 
                if(read(fd, buf, table[i].len) <= 0)
                        continue;
                
                buf[table[i].len-1] = '\0';
                buf_ptr = strdup(buf);
                ptr = strtok(buf_ptr, ":");
                
                if(username != NULL) {
                        if(strcmp(username, ptr) == 0){
                                for(j = 0; j < flags; j++) {
                                        ptr = strtok( NULL, ":");
                                }
                                if(get_string != NULL)
                                        strcpy(get_string, ptr);
                                return i+1;
                        }
                }
                else{   
                        /* To check id */
                        if(get_string != NULL) {
                                for(j = 0; j < GET_ID; j++) {
                                        ptr = strtok( NULL, ":");
                                }
                                if(strcmp(get_string, ptr) == 0){
                                        return 0;
                                }
                        }
                }
        }
        /* return -1 when there is no username or id */
        return -1;
}

int make_passwd(char *inData)
{
	unsigned long seed[2];
	char salt[] = "$1$........";
	const char *const seedchars =
		"./0123456789ABCDEFGHIJKLMNOPQRST"
		"UVWXYZabcdefghijklmnopqrstuvwxyz";
	char *password;
	int i;

	/* Generate a (not very) random seed.
	   You should do it better than this... */
	seed[0] = time(NULL);
	seed[1] = getpid() ^ (seed[0] >> 14 & 0x30000);

	/* Turn it into printable characters from ‘seedchars’. */
	for (i = 0; i < 8; i++)
		salt[3+i] = seedchars[(seed[i/5] >> (i%5)*6) & 0x3f];

	password = crypt(inData, salt);

	/* Print the results. */
	memset(inData, '\0', MAX_PASSWD);
	strncpy(inData, password, strlen(password));
	return 0;
} 
