#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <crypt.h>
#include <time.h>

#define MAX_PASSWD      40
#define MAX_COMMENT     100
#define MAX_USERNAME    17
#define MAX_USER        10
#define MAX_ID          10
#define BUFSIZE         256

#define GET_NAME        0
#define GET_PASSWD      1
#define GET_ID          2
#define GET_COMMENT     3



static struct {
        long offset;        // 해당 문자열 시작위치
        int len;            // 문자열 길이
} table [MAX_USER] = {};
int entry; 
int open_shadow();
int table_update(int fd);
int parser(int fd, char *username, char *get_string, int flags);
int userdel(int fd, char *inData);
int main(int argc, char **argv){

	int fd = open_shadow();

	if(argc < 2){
		printf("\nPlease insert user name!\n Example : userdel keti\n");
	}
	else
		userdel(fd, argv[1]);
	

}

int userdel(int fd, char *inData){

	int linenumb = 0;
	char *name = malloc(sizeof(char) * MAX_USERNAME);
	char line_str[3];

	char dummy_fstring[100] = "sed -i '";
	char dummy_bstring[25] =  "d' /etc/keti_shadow";
	linenumb = parser(fd, inData, NULL, 0);

	if(linenumb != -1){
		printf("you try to delete %s\n", inData);
		printf("line number is %d\n", linenumb);
		sprintf(line_str, "%d", linenumb);
		strcat(dummy_fstring, line_str);
		strcat(dummy_fstring, dummy_bstring);
		printf("dummy string : %s\n", dummy_fstring);
		system(dummy_fstring);
	}
	else{
		printf("%s user is not exist in file\n", inData);
	}	
			

}
int parser(int fd, char *username, char *get_string, int flags)
{       
        int i, j;
        char buf[BUFSIZE];
        char *ptr, *buf_ptr;
        
        table_update(fd);
                
                // dprintf("Entry : %d\n", entry);
        for(i = 0; i < entry; i++) {
                lseek(fd, table[i].offset, 0); 
                if(read(fd, buf, table[i].len) <= 0)
                        continue;
                if(table[i].len != 0 && buf[table[i].len-1] == '\n')
                        buf[table[i].len-1] = '\0';
                else    
                        buf[table[i].len] = '\0';
                buf_ptr = strdup(buf);
                // dprintf("buf_ptr : %s\n", buf_ptr);
                ptr = strtok(buf_ptr, ":");
                //  dprintf("parsed : %s\n", ptr);
                if(username != NULL) {
                        if(strcmp(username, ptr) == 0){
                                for(j = 0; j < flags; j++) {
                                        ptr = strtok( NULL, ":");
                                }
                                //printf("parse fin : %s\n", ptr);
                                //          dprintf("%s", buf);
                                if(get_string != NULL){
                   //                     printf("get_string : %s, ptr : %s", get_string, ptr);
                                        strcpy(get_string, ptr);
                                }
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
int open_shadow()
{
	int fd;
	
	char *file_name = "/etc/keti_shadow";
	
	if (access(file_name, F_OK) == 0){
		//dprintf("%s File exist.\n", file_name);
		if(( fd = open(file_name, O_RDWR)) < 0)
		{
			perror("open");
			return -1;
		}
		table_update(fd);
	}
	else{
		if ((fd = open(file_name, O_RDWR | O_CREAT, 0644)) < 0){
			perror("open");
			return -1;
		}
	}
	return fd;
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
