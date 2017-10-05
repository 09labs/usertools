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
#define MAX_COMMENT     100
#define MAX_USERNAME    17
#define MAX_USER        10
#define MAX_ID			10
#define BUFSIZE         256

#define GET_NAME        0
#define GET_PASSWD      1
#define GET_ID          2
#define GET_COMMENT     3

#define dprintf(fmt, args...) fprintf(stderr, "\x1b[31m""[%s:%d]: " "\x1b[0m" fmt, \
		__FILE__, __LINE__, ##args)

static struct {
	long offset;        // 해당 문자열 시작위치
	int len;            // 문자열 길이
} table [MAX_USER] = {}; 
int entry = 0; 

int open_shadow();
int usermod(int fd, int line, char *new);
int table_update(int fd);
int parser(int fd, char *username, char *get_string, char *full_line, int flags);
int usermod_parser(char *full_string, char *edit_string, int flags);
int make_passwd(char *inData);
int check_passwd(int fd, char *username, char *main_argv, char *password);

void usage(void);

int main(int argc, char *argv[])
{
	int i, j;
	int fd, menu, ix;
	char *name = malloc(sizeof(char) * MAX_USERNAME);
	char *name2 = malloc(sizeof(char) * MAX_USERNAME);
	char *old_comment = malloc(sizeof(char) * MAX_COMMENT);
	char *new_comment = malloc(sizeof(char) * MAX_COMMENT);
	char *old_password = malloc(sizeof(char) * MAX_PASSWD);
	char *new_password = malloc(sizeof(char) * MAX_PASSWD);
	char *old_id = malloc(sizeof(char) * MAX_ID); 
	char *new_id = malloc(sizeof(char) * MAX_ID); 
	char *full_sh = malloc(sizeof(char) * BUFSIZE);

	if(argc < 3 ) {
		usage();
	}

	// check option
	if(argv[2][0] == '-') {    

		if ((fd = open_shadow()) < 0) {
			perror("No shadow file!");
			exit(1);
		}   

		if((strcmp(argv[2], "-n") == 0) && (argc == 4)) {
			if((ix  = parser(fd, argv[1], NULL, full_sh, GET_NAME)) == -1) {
				dprintf("ERROR : There is no username [%s]!\n", argv[1]);
				exit(-1);
			}
			else
				usermod_parser(full_sh, argv[3], 1);
				printf("edited full line is %s\n", full_sh);
				fd = usermod(fd, ix, full_sh);
		}   
		else if((strcmp(argv[2], "-p") == 0)) {
			if((ix  = parser(fd, argv[1], old_password, full_sh, GET_PASSWD)) == -1) {
				dprintf("ERROR : There is no username [%s]!\n", argv[1]);
				exit(-1);
			}
			else {
				if(check_passwd(fd, argv[1], argv[3], old_password) != -1){
					make_passwd(argv[4]);
					strcpy(new_password, argv[4]);
					usermod_parser(full_sh, new_password,2);
					printf("edited full line is %s\n", full_sh);
					fd = usermod(fd, ix, full_sh);
				}
				else {
					dprintf("Wrong password!\n");
					exit(-3);
				}
			}
		}   
		else if((strcmp(argv[2], "-i") == 0) && (argc == 4)) {
			if((ix  = parser(fd, argv[1], old_id, full_sh, GET_ID)) == -1) {
				dprintf("ERROR : There is no username [%s]!\n", argv[1]);
				exit(-1);
			}
			else
				usermod_parser(full_sh, argv[3],3);
                                printf("edited full line is %s\n", full_sh);
				fd = usermod(fd, ix, full_sh);
		}   
		else if((strcmp(argv[2], "-c") == 0) && (argc == 4)) {
			if((ix  = parser(fd, argv[1], old_comment, full_sh, GET_COMMENT)) == -1) {
				dprintf("ERROR : There is no username [%s]!\n", argv[1]);
				exit(-1);
			}
			else
				printf("before full line is %s\n", full_sh);
				usermod_parser(full_sh, argv[3], 4);
				printf("edited full line is %s\n", full_sh);
				fd = usermod(fd, ix, full_sh);
		}   
		else {
			fprintf(stderr, "unknwon option %s\n", argv[2]);
			usage();
		}   
	}   
	else 
		usage();
	close(fd);
	exit(0);
}

/* Open shadow file and return shadow file fd */
int open_shadow()
{
	int fd;

	char *file_name  = "/etc/keti_shadow";
	if (access(file_name, F_OK) == 0) {
		/* Check there is already shadow file */
		if (( fd = open(file_name, O_RDWR)) < 0) {
			perror("open");
			return -1;
		}
		table_update(fd);
		return fd;
	}
	else {
		return -1;
	}
}

int usermod(int fd, int line, char *new)
{
	char *cmd = malloc(sizeof(char) * 512);
	char *sed       = "sed -i '";
	char line_num[2]= "\0";
	char *ch_start  = "s#.*#";   // line number 추가 필요
	char *ch_end    = "#g' /etc/keti_shadow";
	printf("line : %d\n", line);
	snprintf(line_num, 2, "%d", line);
	sprintf(cmd, "%s%s%s%s%s", sed, line_num, ch_start, new, ch_end);
	dprintf("%s\n", cmd);
	//system("mv new_shadow_example new_shadow_example.tmp");
	system(cmd);	
	close(fd);
	return open_shadow();
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
* usermod_parser(full_string, edit_string, flags)
* full_string : Full string for edit
* edit_string : To edit string (ex : comment : 1/1/1/4, name : keti)
* flags : 1 is name / 2 is passwd / 3 is userid / 4 is comment
*
*
*/
int usermod_parser(char *full_string, char *edit_string, int flags)
{
	int i = 0;
	char *buf_ptr;
	char **parse_string;
	//Memory 동적할당
	parse_string = (char **)malloc(4*sizeof(char *));
	parse_string[0] = (char *)malloc(MAX_USERNAME*sizeof(char));
	parse_string[1] = (char *)malloc(MAX_PASSWD*sizeof(char));
	parse_string[2] = (char *)malloc(MAX_ID*sizeof(char));
	parse_string[3] = (char *)malloc(MAX_COMMENT*sizeof(char));

	//라인 한 줄 파싱
	buf_ptr = strtok(full_string, ":");

	while(buf_ptr != NULL){
		strcpy(parse_string[i], buf_ptr); // 각 위치에 파싱한 데이터 저장
		printf("buf is %s\n", buf_ptr);
		buf_ptr = strtok(NULL, ":");
		i++;
	}
/*	Debug print
	for(i = 0 ; i < 4 ; i++){
		printf("parse %d = %s\n",i,  parse_string[i]);
	}
*/
	if(edit_string != NULL){
		strcpy(parse_string[flags-1],"");
		//strset(parse_string[flags-1], '\0');
		strcpy(parse_string[flags-1], edit_string);
	}
	strcpy(full_string, "");
	//strset(full_string);
	for(i = 0 ; i < 4 ; i++){
		strcat(full_string, parse_string[i]);
		if(i != 3)
			strcat(full_string, ":");
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
int parser(int fd, char *username, char *get_string, char *full_line, int flags)
{
	int i, j;
	char buf[BUFSIZE];
	char *ptr, *buf_ptr, full_string;

	table_update(fd);

	for(i = 0; i < entry; i++) {
		lseek(fd, table[i].offset, 0);
		if(read(fd, buf, table[i].len) <= 0)
			continue;

		buf[table[i].len-1] = '\0';
		buf_ptr = strdup(buf);
		if(full_line != NULL){
			printf("buf ptr : %s\n", buf_ptr);
			strcpy(full_line, buf_ptr);
		}
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

int check_passwd(int fd, char *username, char *main_argv, char *password) {
	char *p, *supplied, *salt;
	char *correct = malloc(sizeof(char) * MAX_PASSWD);

	p = password;
	/* Read the correct hash from the shadow entry */
	parser(fd, username, correct, NULL, GET_PASSWD);
	/* Extract the salt. Remember to free the memory. */
	salt = strdup(correct);
	if (salt == NULL)
		return 2;
	p = strchr(salt + 1, '$');
	if (p == NULL)
		return 2;
	p = strchr(p + 1, '$');
	if (p == NULL)
		return 2;
	p[1] = 0;

	/*Encrypt the supplied password with the salt and compare the results*/
	supplied = crypt(main_argv, salt);
	if (supplied == NULL) return 2;
	if(strcmp(supplied, correct) == 0) 
		return 0;
	else 
		return -1;
}

void usage(void)
{
	//manual for usermod
	printf("Usage : usermod USERNAME [OPTION] [NEW_DATA]\
			\n\t-n : username을 NEW_DATA로 변경\
			\n\t-p : password 	확인 및 변경\
			\n\t-i : UID를      NEW_DATA로 변경\
			\n\t-c : comment를	NEW_DATA로 변경\n");
	exit(1);
}


