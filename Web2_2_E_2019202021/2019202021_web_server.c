/////////////////////////////////////////////////////////////////////////
// File Name   : 2019202021_web_server.c								//
// Date        : 2023/5/3												//
// OS		: Ubuntu 16.04.5 LTS 64bits									//
// Author	: Jung Sung Yeob											//
// Student Id	: 2019202021											//
// ---------------------------------------------------------------------//
// Title : System Programming Assignment #2-2 (Basic Web Server)		//
// Description : Coding web server by c language						//
//				html document for ls_command							//
//			    activate socket to accept and bind client request       //
//              get client request and show html or send file           //
//              send response header and response message(file)         //
//              wait for next request                                   //
/////////////////////////////////////////////////////////////////////////
#define _GNU_SOURCE
#define URL_LEN 256
#define BUFFSIZE 1024
#define PORTNO 40000

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>
#include <libgen.h>
#include <errno.h>
#include <fnmatch.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>




//////////////////////////////////////////////////////////////////////////
// print_permission                     								//
// =====================================================================//
// Input:   mode_t mode													//
// Ouput : print file's type & permission, 								//
//			no return													//
// Purpose: for print file's type & permission 							//
//////////////////////////////////////////////////////////////////////////
void print_permission(mode_t mode,FILE *file){ //option -l print permission
	fprintf(file,"<td>");
	if(S_ISLNK(mode))
		fprintf(file,"l");
	else
		fprintf(file,(S_ISDIR(mode)) ? "d" : "-"); //if dir print "e" else print "-"
	fprintf(file,(mode & S_IRUSR) ? "r" : "-"); //if have user read permission
    fprintf(file,(mode & S_IWUSR) ? "w" : "-"); //if have user write permission
	fprintf(file,(mode & S_IXUSR) ? "x" : "-"); //if have user execute permission
    fprintf(file,(mode & S_IRGRP) ? "r" : "-"); //if have group read permission
    fprintf(file,(mode & S_IWGRP) ? "w" : "-"); //if have group write permission
    fprintf(file,(mode & S_IXGRP) ? "x" : "-"); //if have group execute permission
    fprintf(file,(mode & S_IROTH) ? "r" : "-"); //if have other read permission
    fprintf(file,(mode & S_IWOTH) ? "w" : "-"); //if have other write permission
    fprintf(file,(mode & S_IXOTH) ? "x" : "-"); //if have other execute permission
	fprintf(file,"</td>");
}

//////////////////////////////////////////////////////////////////////////
// print_UID		                    								//
// =====================================================================//
// Input:   uid_t uid													//
// Ouput : print file's uid												//
//			no return													//
// Purpose: for print file's uid			 							//
//////////////////////////////////////////////////////////////////////////
void print_UID(uid_t uid,FILE *file){ //option -l print UID
	struct passwd *pw; //get struct passwd
	if((pw=getpwuid(uid))!=NULL){ //if get pwuid is not NULL
			fprintf(file,"<td>%s</td>", pw->pw_name); //print pw_name
		}else{
			fprintf(file,"<td>%d</td>",uid); //print uid num
		}
}

//////////////////////////////////////////////////////////////////////////
// print_GID		                    								//
// =====================================================================//
// Input:   gid_t gid													//
// Ouput : print file's gid												//
//			no return													//
// Purpose: for print file's gid			 							//
//////////////////////////////////////////////////////////////////////////
void print_GID(gid_t gid,FILE *file){ //option -l print GID
	struct group *gr; //get struct group
	if((gr=getgrgid(gid))!=NULL){ //if get grgid is not NULL
			fprintf(file,"<td>%s</td>", gr->gr_name); //print gr_name
		}else{
			fprintf(file,"<td>%d</td>",gid); //print gid num
		}
}


//////////////////////////////////////////////////////////////////////////
// print_LastModified_time                								//
// =====================================================================//
// Input:   time_t time													//
// Ouput : print file's last modified time								//
//			no return													//
// Purpose: for print file's last modified time							//
//////////////////////////////////////////////////////////////////////////
void print_LastModified_time(time_t time, FILE *file){ //option -l print lastmodified time
	char time_str[80]; //array 80
	struct tm *time_tm; //get struct tm to pointer time_tm
	time_tm = localtime(&time); //get localtime
		strftime(time_str,sizeof(time_str),"%b %d %H:%M",time_tm);//strftime as format
		fprintf(file,"<td>%s</td>",time_str); //print time
}

//////////////////////////////////////////////////////////////////////////
// print_long			                 								//
// =====================================================================//
// Input:   char* dir_name, struct stat file_stat, char*abspath,        //
//          FILE*file                                                   //         
// Ouput : print file's info by long format								//
//			no return													//
// Purpose: for print file's info by long format						//
//			if -h option print readable size							//
//////////////////////////////////////////////////////////////////////////
void print_long(struct stat file_stat, char*dir_name, char*abs_path, FILE *file){ //option -l print long format
	char tmppath[255]; //to get relative path for hyper link
    char cwd[1024];// to get current working directory
    getcwd(cwd,1024);//get current working directory
    int cwd_cnt=0;//current working directory's parent directory count number
    char * tok=NULL; //for token
    tok=strtok(cwd,"/"); //cut by slash
    while(tok!=NULL){ //until token == NULL
        cwd_cnt++; //count up
        tok = strtok(NULL,"/");//cut by slash
    }

    if(strcmp(dir_name,".")==0){ //if dir_name is ".", that special directory mean current directory
        strcpy(tmppath,abs_path); //tmppath = current directory
    }else{
        snprintf(tmppath,255,"%s/%s",abs_path,dir_name);//tmppath = current directory + dir_name
    }
    //to goet relative path
    tok = strtok(tmppath,"/");
    for(int i=0;i<cwd_cnt-1;i++){
        tok = strtok(NULL,"/");
    }
    //cut until current working directory
    tok = strtok(NULL,""); // tok = relative path
    
    fprintf(file,"<td><a href=\"/%s\">%s</a></td>",tok,dir_name);	//print file name
	print_permission(file_stat.st_mode,file); //print permission
	fprintf(file,"<td>\t%lu\t</td>",file_stat.st_nlink); //print nlink num
	print_UID(file_stat.st_uid,file); //print UID
	print_GID(file_stat.st_gid,file); //print GID
	fprintf(file,"<td>%lu\t</td>",file_stat.st_size); //print size
	print_LastModified_time(file_stat.st_mtime,file); //print last modified time
	fprintf(file,"</tr>\n");
}

//////////////////////////////////////////////////////////////////////////
// dir_print			                 								//
// =====================================================================//
// Input:   char* abs_Path, int flag_a, int flag_l, char url[],         //
//          FILE* file                                                  //    
// Ouput : create html file for ls result   							//
//			no return													//
// Purpose: for create html file for ls result or 404 error				//
//			symbolic link file: green       							//
//          directory : blue                                            //
//          others: red                                                 //
//////////////////////////////////////////////////////////////////////////

void dir_print(char*abs_Path, int flag_a, int flag_l, char url[], FILE *file){
    DIR *dirp; //DIR pointer dirp
    struct dirent *dir; //struct dirent pointer dir
    struct stat file_stat; //struct stat for file_stat
    struct file_info{ //create strcut about name and st
        char name[1024];
        struct stat st;
    };
    if((dirp=opendir(abs_Path))==NULL){ //if there is not exist directory
        fprintf(file,"<body>\n");
        fprintf(file,"<h1>Not Found</h1><br>");
        fprintf(file,"The request URL %s was not found on this server<br>HTTP 404 - Not Page Found </body>",url);
    }

    struct file_info files[1024]; //struct file_info files
    int num_cnt=0; //count for file count
    int total_size=0;//total block size
    while((dir = readdir(dirp))!=NULL){
        if(flag_a||dir->d_name[0]!='.'){//if flag_a or not hidden file
            char tmppath[255];
            snprintf(tmppath,255,"%s/%s",abs_Path,dir->d_name);
            if(lstat(tmppath,&files[num_cnt].st)==0){//get information about tmppath
                strcpy(files[num_cnt].name, dir->d_name);//get name
                total_size+=(files[num_cnt].st.st_blocks/2);//get block size
                num_cnt++;//file count up
            }
        }
    }
    //sort files
    for(int i=0;i<num_cnt-1;i++){
        for(int j=0;j<num_cnt-i-1;j++){
            char cmp_tmp1[255];
            strcpy(cmp_tmp1,files[j].name);
            if(cmp_tmp1[0]=='.'){//if hidden file
                for(int k=0;cmp_tmp1[k]!='\0';k++){ //shift left
                	cmp_tmp1[k]=cmp_tmp1[k+1]; 
           		}
       		}
        	char cmp_tmp2[255];//tmp value
        	strcpy(cmp_tmp2,files[j+1].name); //get name value to cmp_tmp2
        	if(cmp_tmp2[0]=='.'){ //if hidden file
            	for(int k=0;cmp_tmp2[k]!='\0';k++){//shift left
                	cmp_tmp2[k]=cmp_tmp2[k+1];
            	}
        	}
        	if (strcasecmp(cmp_tmp1, cmp_tmp2) > 0) { //compare name, if com_tmp1 is bigger than cmp_tmp2, change location
            	struct file_info temp = files[j];
            	files[j] = files[j+1];
            	files[j+1] = temp;
        	}
        }
    }
    //make html file
    fprintf(file,"<html>");
    fprintf(file,"<body>\n");
    fprintf(file,"<h1>System Programming Http</h1><br>");
    fprintf(file,"<B>Directory path : %s</B><br>",abs_Path);
    fprintf(file,"<B>total %d</B><br>",total_size);
    fprintf(file,"<table border = \"1\">\n");
    fprintf(file,"<tr><th>NAME</th><th>Permission</th><th>Link</th><th>Owner</th><th>Group</th><th>Size</th><th>Last Modified</th></tr>\n");
    for(int i=0;i<num_cnt;i++){
        struct stat file_stat = files[i].st;
        if(strcmp("html_ls.html",files[i].name)==0)
            continue;
        if(S_ISLNK(file_stat.st_mode)!=0){ //if linkfile
            fprintf(file,"<tr style=\"color:green\">");
        }else if(S_ISDIR(file_stat.st_mode)!=0){// if directory
            fprintf(file,"<tr style=\"color:blue\">");
        }else{//if others
            fprintf(file,"<tr style=\"color:red\">");
        }
        print_long(file_stat,files[i].name,abs_Path,file); //print long type
    }
    fprintf(file,"</table>\n</body>");
    fprintf(file,"</html>");
    //finish html file
}

//////////////////////////////////////////////////////////////////////////
// create_ls			                 								//
// =====================================================================//
// Input:   int argc, char *argv[], int flag_a, int flag_l, char url[]  //
//          int client_fd                                               //    
// Ouput : if there is not exist directory make error html file			//
//			if there is exist directory make ls_html file				//
// Purpose: distinguish exist file or not exist file       				//
//	                                                                    //
//////////////////////////////////////////////////////////////////////////
void create_ls(int argc, char * argv[],int flag_a, int flag_l, char url[],int client_fd){
    DIR *dirp;//DIR pointer dirp
    struct dirent *dir;
    FILE *file=fopen("html_ls.html","w");
    char path[1024];//path element
    getcwd(path,1024);
    if(flag_a!=0){ //if not root path
        strcat(path,url);//get absolute path
    }
    //we don't need <head> or <title>

    //check for not exist directory
    if((dirp=opendir(path))==NULL){
        int checkFile;
        if((checkFile=open(path,O_RDONLY))==-1){//if not directory and file == not exsit file
            fprintf(file,"<body>\n");
            fprintf(file,"<h1>Not Found</h1><br>");
            fprintf(file,"The request URL %s was not found on this server<br>HTTP 404 - Not Page Found </body>",url);
            fclose(file);
            return;
        }
    }
    struct stat file_stat;
    lstat(path,&file_stat);
    if(S_ISDIR(file_stat.st_mode)!=0){//if dirctory
        /*what to do?*/
        //nothing to do, I'll do it later
    }else if(S_ISLNK(file_stat.st_mode)!=0){//if link file
        /*how to send link file?*/
        //just return;
        return;
    }else{ //if regular file(html, c, txt, jpg, png, jpeg)
        /*how to send regular file?*/
        //if(fnmatch("*.jpg", path, FNM_CASEFOLD)==0||fnmatch("*.jpeg", path, FNM_CASEFOLD)==0||fnmatch("*.png", path, FNM_CASEFOLD)==0){
        //just return;
        //}
        return;
    }
    //if directory make html_ls file
    dir_print(path,flag_a,flag_l,url,file);
    fclose(file);//close file
    return;
}


int main(int argc, char *argv[]){//for server operation, not for create html file.

    struct sockaddr_in server_addr, client_addr; //struct for server and client(browser)
    int socket_fd, client_fd; //server and client(browser) nonnegative descriptor
    unsigned int len, len_out;

    if((socket_fd=socket(PF_INET, SOCK_STREAM,0))<0){ //IPv4 internet protocol, TCP method
        printf("Server: Can't open stream socket."); //if cannot socket
        return 0;
    }

    int opt =1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT|SO_REUSEADDR, &opt, sizeof(opt));// set socket's option

    memset(&server_addr, 0, sizeof(server_addr)); //reset server_addr
    server_addr.sin_family = AF_INET; // IPv4 Internet Protocol
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//host to network IP address
    server_addr.sin_port = htons(PORTNO);//host to network port number

    if (bind(socket_fd, (struct sockaddr *)&server_addr,sizeof(server_addr))<0){ //binding socket and struct
        printf("Server: can't bind local address.\n"); //if cannot bind
        return 0;
    }

    listen(socket_fd, 5);//waiting for client connection, backlog = 5, if connected goto waiting queue
    while(1){
        struct in_addr inet_client_address; //to get client IP address
        //reset all element for client request and response
        char buf[BUFFSIZE]={0, };
        char tmp[BUFFSIZE]={0, };
        char response_header[BUFFSIZE]={0, };
        char response_message[BUFFSIZE]={0, };
        char url[URL_LEN]={0, };
        char method[20] = {0, };
        char *tok =NULL;
        len = sizeof(client_addr); //size of struct client_addr
        client_fd = accept(socket_fd, (struct sockaddr*)&client_addr, &len); //server accept client connection
        if(client_fd<0){ //if error
            printf("Server : accept failed\n");//print error message
            return 0;
        }
        if(client_addr.sin_family!=AF_INET){ //if client_addr.sin_family is not IPv4
            printf("invalid family\n");
            close(client_fd);
            continue;
        }
        if(len<sizeof(struct sockaddr_in)){ //if client_addr size is error
            printf("invalid length\n");
            close(client_fd);
            continue;
        }
        inet_client_address.s_addr = client_addr.sin_addr.s_addr; //get client IP address
        printf("[%s : %d] client was connected\n", inet_ntoa(inet_client_address), client_addr.sin_port); //if connected
        ssize_t num_bytes = read(client_fd, buf, BUFFSIZE);

        if (num_bytes < 0) { //if read buf error
            printf("Server: read failed\n");//close client socket
            continue;
        } else if (num_bytes == 0) {//if nothing in buf
            printf("Server: client closed connection\n");
            close(client_fd); //close client socket
            continue;
        } else if (num_bytes > BUFFSIZE) { //if overflow in buf
            printf("Server: input buffer overflow\n");
            close(client_fd); //close client socket
            continue;
        }

        strcpy(tmp, buf);//tmp = buf
        puts("=========================================");
        printf("Request form [%s : %d]\n",inet_ntoa(inet_client_address),client_addr.sin_port); //request form
        puts(buf);
        puts("=========================================");

        tok = strtok(tmp," "); //cut by blank about tmp, get about GET
        strcpy(method, tok); //requset method
        if(strcmp(method, "GET")==0){
            tok = strtok(NULL, " ");//cut by blank about tmp
            strcpy(url, tok); //url = tok
        }else{
            printf("there is no GET\n");
            close(client_fd);
            continue;
        }


        if(strcmp(url,"/")==0){ 
            create_ls(argc,argv,0,1,url,client_fd); //if directory it makes html file
        }else{
            while(url[strlen(url)-1]=='/'){//delete last '/' for generalization
                url[strlen(url)-1]='\0';
            }
            if(url[0]=='\0'){
                create_ls(argc,argv,0,1,url,client_fd);
            }else{
                create_ls(argc,argv,1,1,url,client_fd);//if directory it makes html file
            }
        }

        
        DIR * dirp; //DIR pointer dirp
        char path[1024];//
        getcwd(path,1024);
        if(strcmp(url,"/")!=0){ //if not only port
            strcat(path,url);//add url after cwd
        }
        if((dirp=opendir(path))==NULL){//if not directory
            int checkFile;
            if((checkFile=open(path,O_RDONLY))==-1){//if not existing directory
                char *filename = "html_ls.html";

                // open html file
                int fd = open(filename, O_RDONLY);
                if (fd == -1) {
                    perror("open");
                    exit(EXIT_FAILURE);
                }

                // get file size
                off_t offset = 0;
                struct stat file_stat;
                fstat(fd, &file_stat);//get information about open file
                off_t filesize = file_stat.st_size;//get file size
                //error header
                sprintf(response_header,"HTTP/1.0 404 Error\r\n"
                            "Server: 2019202021 simple web server\r\n"
                            "Content-length:%lu\r\n"
                            "Content-type:text/html\r\n\r\n",filesize);
                write(client_fd,response_header,strlen(response_header));//send response header
                sendfile(client_fd, fd, &offset, filesize);//send file
                close(fd);
            }else{//if file
                if(fnmatch("*.jpg", path, FNM_CASEFOLD)==0||fnmatch("*.jpeg", path, FNM_CASEFOLD)==0||fnmatch("*.png", path, FNM_CASEFOLD)==0){//if image file
                    // get file size
                    off_t offset = 0;
                    struct stat file_stat;
                    fstat(checkFile, &file_stat);
                    off_t filesize = file_stat.st_size;//get file size
                    //image header
                    sprintf(response_header,"HTTP/1.0 200 OK\r\n"
                                "Server: 2019202021 simple web server\r\n"
                                "Content-length:%lu\r\n"
                                "Content-type:image/*\r\n\r\n",filesize);
                    write(client_fd,response_header,strlen(response_header)); //send response header
                    sendfile(client_fd,checkFile,&offset,filesize);//send file
                    close(checkFile);
                }else{//if plain file or symbolic link
                    off_t offset = 0;
                    struct stat file_stat;
                    lstat(path,&file_stat);
                    off_t filesize=file_stat.st_size;
                    if(S_ISLNK(file_stat.st_mode)!=0){ //if link file
                        close(checkFile);
                        char linkpath[255];
                        ssize_t linklen = readlink(path,linkpath, sizeof(linkpath)); //get about original of linkfile
                        linkpath[linklen]='\0'; //last char is NULL
                        int linkFile;
                        if((linkFile=open(linkpath,O_RDONLY))==-1){//if path is error
                            close(linkFile);
                            //make error html
                            FILE *file =fopen("html_ls.html","w");
                            fprintf(file,"<body>\n");
                            fprintf(file,"<h1>Not Found</h1><br>");
                            fprintf(file,"The request URL %s was not found on this server<br>HTTP 404 - Not Page Found </body>",url);
                            fclose(file);
                            sprintf(response_header,"HTTP/1.0 404 Error\r\n"
                                        "Server: 2019202021 simple web server\r\n"
                                        "Content-length:%lu\r\n"
                                        "Content-type:text/html\r\n\r\n",filesize);
                            //open html file
                            char *filename = "html_ls.html";

                            int fd = open(filename,O_RDONLY);
                            if(fd ==-1){
                                perror("open");
                                exit(EXIT_FAILURE);
                            }
                            struct stat link_stat;
                            fstat(fd, &link_stat);
                            filesize=link_stat.st_size;
                            write(client_fd,response_header,strlen(response_header)); //send response header
                            sendfile(client_fd,fd,&offset,filesize);//send file
                            close(fd);
                        }else{//if there is link original file
                            struct stat link_stat;
                            fstat(linkFile,&link_stat);
                            filesize=link_stat.st_size;
                            if(fnmatch("*.jpg", linkpath, FNM_CASEFOLD)==0||fnmatch("*.jpeg", linkpath, FNM_CASEFOLD)==0||fnmatch("*.png", linkpath, FNM_CASEFOLD)==0){//if image file
                                sprintf(response_header,"HTTP/1.0 200 OK\r\n"
                                            "Server: 2019202021 simple web server\r\n"
                                            "Content-length:%lu\r\n"
                                            "Content-type:image/*\r\n\r\n",filesize);
                            }else{//if not image file
                                sprintf(response_header,"HTTP/1.0 200 OK\r\n"
                                            "Server: 2019202021 simple web server\r\n"
                                            "Content-length:%lu\r\n"
                                            "Content-type:text/plain\r\n\r\n",filesize);
                            }
                            write(client_fd,response_header,strlen(response_header));//send response header
                            sendfile(client_fd,linkFile,&offset,filesize);//send file
                            close(linkFile);
                        }
                    }else{ //if not link file
                        //create text/plain header
                        sprintf(response_header,"HTTP/1.0 200 OK\r\n"
                                    "Server: 2019202021 simple web server\r\n"
                                    "Content-length:%lu\r\n"
                                    "Content-type:text/plain\r\n\r\n",filesize);
                        write(client_fd,response_header,strlen(response_header));//send response header
                        sendfile(client_fd,checkFile,&offset,filesize);//send file
                        close(checkFile);
                    }
                }
            }
        }else{//if directory
            char *filename = "html_ls.html";

            // open html file
            int fd = open(filename, O_RDONLY);
            if (fd == -1) {
                perror("open");
                exit(EXIT_FAILURE);
            }

            // get file size
            off_t offset = 0;
            struct stat file_stat;
            fstat(fd, &file_stat);
            off_t filesize = file_stat.st_size;
            //create text/plain header
            sprintf(response_header,"HTTP/1.0 200 OK\r\n"
                        "Server: 2019202021 simple web server\r\n"
                        "Content-length:%lu\r\n"
                        "Content-type:text/html\r\n\r\n",filesize);
            write(client_fd,response_header,strlen(response_header));//send response header
            sendfile(client_fd, fd, &offset, filesize);//send file
            close(fd);
        }
        
       
        //send response header and file;


        printf("[%s : %d] client was disconnected\n", inet_ntoa(inet_client_address), client_addr.sin_port); //printf disconnected message
        close(client_fd);//close client_fd
    }

    close(socket_fd); //close socket_fd
    return 0;
}



