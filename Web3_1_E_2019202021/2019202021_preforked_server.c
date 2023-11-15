/////////////////////////////////////////////////////////////////////////
// File Name   : 2019202021_preforked_server.c							//
// Date        : 2023/5/15												//
// OS		: Ubuntu 16.04.5 LTS 64bits									//
// Author	: Jung Sung Yeob											//
// Student Id	: 2019202021											//
// ---------------------------------------------------------------------//
// Title : System Programming Assignment #3-1 (preforked Web Server)	//
// Description : Coding web server by c language						//
//				html document for ls_command							//
//			    activate socket to accept and bind client request       //
//              get client request and show html or send file           //
//              send response header and response message(file)         //
//              wait for next request                                   //
//              alarm 10 seconds for print connetion history  format    //
//              print connection history at child process               //
//              accessible.usr to get white list                        //
//              checking for new client and disconnect client           //
//              have to remove zombie process                           //
//              print simple log history                                //
//////////////////////////////////////////////////////////////////////////
#define _GNU_SOURCE
#define URL_LEN 256
#define BUFFSIZE 1024
#define PORTNO 40000
#define MAX_QUEUE 10
#define ACCESSIBLE_USR "accessible.usr"

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
#include <signal.h>
#include <sys/wait.h>
#include <sys/sendfile.h>

static int maxNchildren;
static pid_t *pids;

int NUM_Queue[MAX_QUEUE];           // number queue
struct in_addr IP_Queue[MAX_QUEUE]; // IP queue
int PID_Queue[MAX_QUEUE];           // PID queue
int PORT_Queue[MAX_QUEUE];          // PORT queue
time_t TIME_Queue[MAX_QUEUE];       // Time queue

int head=0 ;    // first queue
int tail=-1; // last queue
int queue_cnt=0;//queue count

//////////////////////////////////////////////////////////////////////////
// compare_WhiteList                           							//
// =====================================================================//
// Input:   char* IP_Address											//
// Ouput : 	if white list return 0                                      //
//          else return -1       										//
// Purpose: for handle alarm and restart alarm(10) 						//
//////////////////////////////////////////////////////////////////////////
int compare_WhiteList(char *IP_Address)
{
    FILE *file = fopen(ACCESSIBLE_USR, "r"); // open accessible.usr with read option
    if (file == NULL)
    {
        perror("failed to read accessible.usr"); // something wrong with open accessible.usr
        return -1;
    }
    char line[16]; // char for ip address
    while (fgets(line, sizeof(line), file) != NULL)
    { // get line of accessible.usr
        unsigned int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        { // change '\n' to '\0'
            line[len - 1] = '\0';
        }
        if (fnmatch(line, IP_Address, 0) == 0)
        { // fnmatch line and IP_Address
            return 0;
        }
        else
        {
            continue;
        }
    }
    fclose(file); // close file
    return -1;
}

//////////////////////////////////////////////////////////////////////////
// is_empty                                								//
// =====================================================================//
// Input:               												//
// Ouput : 	return tail<head											//
// Purpose: for checking queue is empty         						//
//////////////////////////////////////////////////////////////////////////
int is_empty()
{
    return (tail < head); // return true if tail<head
}
//////////////////////////////////////////////////////////////////////////
// is_full                                								//
// =====================================================================//
// Input:               												//
// Ouput : 	return tail == MAX_QUEUE-1									//
// Purpose: for checking queue if full           						//
//////////////////////////////////////////////////////////////////////////
int is_full()
{
    return (tail == MAX_QUEUE - 1); // return true if tail ==MAX_QUEUE-1
}

//////////////////////////////////////////////////////////////////////////
// sub_Queue                               								//
// =====================================================================//
// Input:               												//
// Ouput : 	no return        											//
// Purpose: delete fisrt value of queue         						//
//////////////////////////////////////////////////////////////////////////
void sub_Queue()
{
    if (is_empty())
    {                            // if queue is empty
        printf("queue empty\n"); // something wrong with counting
        return;
    }
    for (int i = 0; i < 9; i++)
    { // delete first queue
        NUM_Queue[i] = NUM_Queue[i + 1];
        IP_Queue[i] = IP_Queue[i + 1];
        PID_Queue[i] = PID_Queue[i + 1];
        PORT_Queue[i] = PORT_Queue[i + 1];
        TIME_Queue[i] = TIME_Queue[i + 1];
    }
    tail--; // tail = tail -1
}
//////////////////////////////////////////////////////////////////////////
// add_Queue                               								//
// =====================================================================//
// Input:    struct in_addr IP, int PID, int PORT, time_t TIME			//
// Ouput : 	no return	        										//
// Purpose: for adding queue                    						//
//////////////////////////////////////////////////////////////////////////
void add_Queue(struct in_addr IP, int PID, int PORT, time_t TIME)
{
    if (is_full())
    { // if full delete first queue
        sub_Queue();
    }
    // adding queue
    NUM_Queue[++tail] = ++queue_cnt;
    IP_Queue[tail] = IP;
    PID_Queue[tail] = PID;
    PORT_Queue[tail] = PORT;
    TIME_Queue[tail] = TIME;
}


//////////////////////////////////////////////////////////////////////////
// alarm_handler                           								//
// =====================================================================//
// Input:   int sig        												//
// Ouput : 	no return       											//
// Purpose: for handle alarm and restart alarm(10) 						//
//////////////////////////////////////////////////////////////////////////
void alarm_handler(int sig)
{ // for parent T.T
    puts("===================Connection History=======================");
    printf("NO.\tIP\t\tPID\tPORT\tTIME\n");             // print format
    /*
    for(int i=tail;i>=0;i--){ //print all of queue
        char time_str[80];
        struct tm *time_tm;
        time_tm = localtime(&TIME_Queue[i]);
        strftime(time_str,sizeof(time_str),"%a %b %d %H:%M:%S %Y",time_tm);
        printf("%d\t%s\t%d\t%d\t%s\n",NUM_Queue[i],inet_ntoa(IP_Queue[i]),PID_Queue[i],PORT_Queue[i],time_str);
    }
    */
    for (int i = 0; i < 5; i++)
    {
        kill(pids[i], SIGUSR1);
    }

    alarm(10); // re-alarm 10 seconds
}
//////////////////////////////////////////////////////////////////////////
// exit_handler                           								//
// =====================================================================//
// Input:   int sig        												//
// Ouput : 	no return       											//
// Purpose: for handle SIGINT and printf exit process info  			//
//////////////////////////////////////////////////////////////////////////
void exit_handler(int sig){
    //exit child process
    time_t terminated_time = time(NULL);
    char time_str[80];
    struct tm *time_tm;
    time_tm = localtime(&terminated_time);
    strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);
    printf("[%s] %ld process is terminated.\n", time_str, (long)getpid());
    exit(0);
}
//////////////////////////////////////////////////////////////////////////
// parent_handler                           							//
// =====================================================================//
// Input:   int sig        												//
// Ouput : 	no return       											//
// Purpose: for handle exit by kill instruction                 		//
//////////////////////////////////////////////////////////////////////////
void parent_handler(int sig){
    for (int i = 0; i < 5; i++)
    {
        kill(pids[i], SIGUSR2);
    }
}
//////////////////////////////////////////////////////////////////////////
// child_handler                           								//
// =====================================================================//
// Input:   int sig        												//
// Ouput : 	no return       											//
// Purpose: for handle alarm's kill and printf connnection info  		//
//////////////////////////////////////////////////////////////////////////

void child_handler(int sig)
{
    for (int i = tail; i >= 0; i--)
    { // print all of queue
        char time_str[80];
        struct tm *time_tm;
        time_tm = localtime(&TIME_Queue[i]);
        strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);
        printf("%d\t%s\t%d\t%d\t%s\n", NUM_Queue[i], inet_ntoa(IP_Queue[i]), PID_Queue[i], PORT_Queue[i], time_str);
    }
}

//////////////////////////////////////////////////////////////////////////
// print_permission                     								//
// =====================================================================//
// Input:   mode_t mode													//
// Ouput : print file's type & permission, 								//
//			no return													//
// Purpose: for print file's type & permission 							//
//////////////////////////////////////////////////////////////////////////
void print_permission(mode_t mode, FILE *file)
{ // option -l print permission
    fprintf(file, "<td>");
    if (S_ISLNK(mode))
        fprintf(file, "l");
    else
        fprintf(file, (S_ISDIR(mode)) ? "d" : "-"); // if dir print "e" else print "-"
    fprintf(file, (mode & S_IRUSR) ? "r" : "-");    // if have user read permission
    fprintf(file, (mode & S_IWUSR) ? "w" : "-");    // if have user write permission
    fprintf(file, (mode & S_IXUSR) ? "x" : "-");    // if have user execute permission
    fprintf(file, (mode & S_IRGRP) ? "r" : "-");    // if have group read permission
    fprintf(file, (mode & S_IWGRP) ? "w" : "-");    // if have group write permission
    fprintf(file, (mode & S_IXGRP) ? "x" : "-");    // if have group execute permission
    fprintf(file, (mode & S_IROTH) ? "r" : "-");    // if have other read permission
    fprintf(file, (mode & S_IWOTH) ? "w" : "-");    // if have other write permission
    fprintf(file, (mode & S_IXOTH) ? "x" : "-");    // if have other execute permission
    fprintf(file, "</td>");
}

//////////////////////////////////////////////////////////////////////////
// print_UID		                    								//
// =====================================================================//
// Input:   uid_t uid													//
// Ouput : print file's uid												//
//			no return													//
// Purpose: for print file's uid			 							//
//////////////////////////////////////////////////////////////////////////
void print_UID(uid_t uid, FILE *file)
{                      // option -l print UID
    struct passwd *pw; // get struct passwd
    if ((pw = getpwuid(uid)) != NULL)
    {                                              // if get pwuid is not NULL
        fprintf(file, "<td>%s</td>", pw->pw_name); // print pw_name
    }
    else
    {
        fprintf(file, "<td>%d</td>", uid); // print uid num
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
void print_GID(gid_t gid, FILE *file)
{                     // option -l print GID
    struct group *gr; // get struct group
    if ((gr = getgrgid(gid)) != NULL)
    {                                              // if get grgid is not NULL
        fprintf(file, "<td>%s</td>", gr->gr_name); // print gr_name
    }
    else
    {
        fprintf(file, "<td>%d</td>", gid); // print gid num
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
void print_LastModified_time(time_t time, FILE *file)
{                                                                 // option -l print lastmodified time
    char time_str[80];                                            // array 80
    struct tm *time_tm;                                           // get struct tm to pointer time_tm
    time_tm = localtime(&time);                                   // get localtime
    strftime(time_str, sizeof(time_str), "%b %d %H:%M", time_tm); // strftime as format
    fprintf(file, "<td>%s</td>", time_str);                       // print time
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
void print_long(struct stat file_stat, char *dir_name, char *abs_path, FILE *file)
{                           // option -l print long format
    char tmppath[255];      // to get relative path for hyper link
    char cwd[1024];         // to get current working directory
    getcwd(cwd, 1024);      // get current working directory
    int cwd_cnt = 0;        // current working directory's parent directory count number
    char *tok = NULL;       // for token
    tok = strtok(cwd, "/"); // cut by slash
    while (tok != NULL)
    {                            // until token == NULL
        cwd_cnt++;               // count up
        tok = strtok(NULL, "/"); // cut by slash
    }

    if (strcmp(dir_name, ".") == 0)
    {                              // if dir_name is ".", that special directory mean current directory
        strcpy(tmppath, abs_path); // tmppath = current directory
    }
    else
    {
        snprintf(tmppath, 255, "%s/%s", abs_path, dir_name); // tmppath = current directory + dir_name
    }
    // to goet relative path
    tok = strtok(tmppath, "/");
    for (int i = 0; i < cwd_cnt - 1; i++)
    {
        tok = strtok(NULL, "/");
    }
    // cut until current working directory
    tok = strtok(NULL, ""); // tok = relative path

    fprintf(file, "<td><a href=\"/%s\">%s</a></td>", tok, dir_name); // print file name
    print_permission(file_stat.st_mode, file);                       // print permission
    fprintf(file, "<td>\t%lu\t</td>", file_stat.st_nlink);            // print nlink num
    print_UID(file_stat.st_uid, file);                               // print UID
    print_GID(file_stat.st_gid, file);                               // print GID
    fprintf(file, "<td>%lu\t</td>", file_stat.st_size);              // print size
    print_LastModified_time(file_stat.st_mtime, file);               // print last modified time
    fprintf(file, "</tr>\n");
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

void dir_print(char *abs_Path, int flag_a, int flag_l, char url[], FILE *file, char *filename)
{
    DIR *dirp;             // DIR pointer dirp
    struct dirent *dir;    // struct dirent pointer dir
    struct stat file_stat; // struct stat for file_stat
    struct file_info
    { // create strcut about name and st
        char name[1024];
        struct stat st;
    };
    if ((dirp = opendir(abs_Path)) == NULL)
    { // if there is not exist directory
        fprintf(file, "<body>\n");
        fprintf(file, "<h1>Not Found</h1><br>");
        fprintf(file, "The request URL %s was not found on this server<br>HTTP 404 - Not Page Found </body>", url);
    }

    struct file_info files[1024]; // struct file_info files
    int num_cnt = 0;              // count for file count
    int total_size = 0;           // total block size
    while ((dir = readdir(dirp)) != NULL)
    {
        if (flag_a || dir->d_name[0] != '.')
        { // if flag_a or not hidden file
            char tmppath[265];
            snprintf(tmppath, 265, "%s/%s", abs_Path, dir->d_name);
            if (lstat(tmppath, &files[num_cnt].st) == 0)
            {                                                    // get information about tmppath
                strcpy(files[num_cnt].name, dir->d_name);        // get name
                total_size += (files[num_cnt].st.st_blocks / 2); // get block size
                num_cnt++;                                       // file count up
            }
        }
    }
    // sort files
    for (int i = 0; i < num_cnt - 1; i++)
    {
        for (int j = 0; j < num_cnt - i - 1; j++)
        {
            char cmp_tmp1[255];
            strcpy(cmp_tmp1, files[j].name);
            if (cmp_tmp1[0] == '.')
            { // if hidden file
                for (int k = 0; cmp_tmp1[k] != '\0'; k++)
                { // shift left
                    cmp_tmp1[k] = cmp_tmp1[k + 1];
                }
            }
            char cmp_tmp2[255];                  // tmp value
            strcpy(cmp_tmp2, files[j + 1].name); // get name value to cmp_tmp2
            if (cmp_tmp2[0] == '.')
            { // if hidden file
                for (int k = 0; cmp_tmp2[k] != '\0'; k++)
                { // shift left
                    cmp_tmp2[k] = cmp_tmp2[k + 1];
                }
            }
            if (strcasecmp(cmp_tmp1, cmp_tmp2) >
                0)
            { // compare name, if com_tmp1 is bigger than cmp_tmp2, change location
                struct file_info temp = files[j];
                files[j] = files[j + 1];
                files[j + 1] = temp;
            }
        }
    }
    // make html file
    fprintf(file, "<html>");
    fprintf(file, "<body>\n");
    fprintf(file, "<h1>System Programming Http</h1><br>");
    fprintf(file, "<B>Directory path : %s</B><br>", abs_Path);
    fprintf(file, "<B>total %d</B><br>", total_size);
    fprintf(file, "<table border = \"1\">\n");
    fprintf(file,
            "<tr><th>NAME</th><th>Permission</th><th>Link</th><th>Owner</th><th>Group</th><th>Size</th><th>Last Modified</th></tr>\n");
    for (int i = 0; i < num_cnt; i++)
    {
        struct stat file_stat = files[i].st;
        if (strcmp(filename, files[i].name) == 0)
            continue;
        if (S_ISLNK(file_stat.st_mode) != 0)
        { // if linkfile
            fprintf(file, "<tr style=\"color:green\">");
        }
        else if (S_ISDIR(file_stat.st_mode) != 0)
        { // if directory
            fprintf(file, "<tr style=\"color:blue\">");
        }
        else
        { // if others
            fprintf(file, "<tr style=\"color:red\">");
        }
        print_long(file_stat, files[i].name, abs_Path, file); // print long type
    }
    fprintf(file, "</table>\n</body>");
    fprintf(file, "</html>");
    // finish html file
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
void create_ls(int flag_a, int flag_l, char url[], int client_fd, FILE *file, char *filename)
{
    DIR *dirp; // DIR pointer dirp
    struct dirent *dir;
    // FILE *file = fopen("html_ls.html", "w");
    char path[1024]; // path element
    getcwd(path, 1024);
    if (flag_a != 0)
    {                      // if not root path
        strcat(path, url); // get absolute path
    }
    // we don't need <head> or <title>

    // check for not exist directory
    if ((dirp = opendir(path)) == NULL)
    {
        int checkFile;
        if ((checkFile = open(path, O_RDONLY)) == -1)
        { // if not directory and file == not exsit file
            fprintf(file, "<body>\n");
            fprintf(file, "<h1>Not Found</h1><br>");
            fprintf(file, "The request URL %s was not found on this server<br>HTTP 404 - Not Page Found </body>", url);
            return;
        }
        close(checkFile);
    }
    struct stat file_stat;
    lstat(path, &file_stat);
    if (S_ISDIR(file_stat.st_mode) != 0)
    { // if dirctory
        /*what to do?*/
        // nothing to do, I'll do it later
    }
    else if (S_ISLNK(file_stat.st_mode) != 0)
    { // if link file
        /*how to send link file?*/
        // just return;
        return;
    }
    else
    { // if regular file(html, c, txt, jpg, png, jpeg)
        /*how to send regular file?*/
        // if(fnmatch("*.jpg", path, FNM_CASEFOLD)==0||fnmatch("*.jpeg", path, FNM_CASEFOLD)==0||fnmatch("*.png", path, FNM_CASEFOLD)==0){
        // just return;
        // }
        return;
    }
    // if directory make html_ls file
    dir_print(path, flag_a, flag_l, url, file, filename);
    // fclose(file);//close file
    return;
}
//////////////////////////////////////////////////////////////////////////
// child_main			                 								//
// =====================================================================//
// Input:   int i, int socket_fd, int addrlen                           //
// Ouput : no return                                        			//
// Purpose: child main function                          				//
//	        child process execution code                                //
//          signal about SIGUSR1 & SIGINT                               //
//          main function at last assignment                            //
//////////////////////////////////////////////////////////////////////////
void child_main(int i, int socket_fd, int addrlen)
{
    signal(SIGUSR1, child_handler);
    signal(SIGINT, exit_handler);
    signal(SIGTERM, exit_handler);
    signal(SIGUSR2, exit_handler);

    int client_fd, len_out;
    socklen_t clilen;
    struct sockaddr_in *client_addr;

    time_t child_time = time(NULL); // get executed_time
    char time_str[80];
    struct tm *time_tm;
    time_tm = localtime(&child_time);
    strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);

    client_addr = (struct sockaddr_in *)malloc(addrlen);
    printf("[%s] %ld process is forked\n", time_str, (long)getpid());
    while (1)
    {
        struct in_addr inet_client_address; // to get client IP address
        // reset all element for client request and response
        char buf[BUFFSIZE] = {
            0,
        };
        char tmp[BUFFSIZE] = {
            0,
        };
        char response_header[BUFFSIZE] = {
            0,
        };
        char response_message[BUFFSIZE] = {
            0,
        };
        char url[URL_LEN] = {
            0,
        };
        char method[20] = {
            0,
        };
        char *tok = NULL;

        client_fd = accept(socket_fd, (struct sockaddr *)client_addr, &addrlen); // server accept client connection

        if (client_fd < 0)
        { // if error
            puts("===================Accept failed====================");
            puts("====================================================");
            close(client_fd);
            continue;
        }
        if (client_addr->sin_family != AF_INET)
        { // if client_addr.sin_family is not IPv4
            puts("==================invalid family====================");
            puts("====================================================");
            close(client_fd);
            continue;
        }
        if (addrlen < sizeof(struct sockaddr_in))
        { // if client_addr size is error
            puts("===================invalid length====================");
            puts("====================================================");
            close(client_fd);
            continue;
        }

        time_t executed_time = time(NULL); // get executed_time
        time_tm = localtime(&executed_time);
        strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);

        inet_client_address.s_addr = client_addr->sin_addr.s_addr; // get client IP address
        puts("=====================New Client====================");
        printf("[%s]\n", time_str);
        printf("IP : %s\nPort : %d\n", inet_ntoa(inet_client_address), client_addr->sin_port); // if connected
        puts("===================================================");
        printf("\n");

        // add queue about alarm handling
        add_Queue(inet_client_address, getpid(), client_addr->sin_port, executed_time); // add new client to queue

        // compare accessible user
        if (compare_WhiteList(inet_ntoa(client_addr->sin_addr)) != 0)
        { // not accessible
            sprintf(response_message,
                    "<h1>Access denied</h1>"
                    "<h3>Your IP: %s</h3>"
                    "You have no permission to access this web server.<br>"
                    "HTTP 403.6 - Forbidden: IP address reject",
                    inet_ntoa(inet_client_address));
            sprintf(response_header,
                    "HTTP/1.0 403 Forbidden \r\n"
                    "Server: 2019202021 advanced web server\r\n"
                    "Content-length:%lu\r\n"
                    "Content-type:text/html\r\n\r\n",
                    strlen(response_message));
            write(client_fd, response_header, strlen(response_header));
            write(client_fd, response_message, strlen(response_message));

            time_t disconnected_time = time(NULL);
            time_tm = localtime(&disconnected_time);
            strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);

            puts("===============Disconnected Client================");
            printf("[%s]\n", time_str);
            printf("IP : %s\nPort : %d\n", inet_ntoa(inet_client_address), client_addr->sin_port); // if connected
            puts("===================================================");
            continue;
        }
        ssize_t num_bytes = read(client_fd, buf, BUFFSIZE);

        if (num_bytes < 0)
        { // if read buf error
            time_t disconnected_time = time(NULL);
            time_tm = localtime(&disconnected_time);
            strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);
            puts("===============Disconnected Client================");
            printf("[%s]\n", time_str);
            printf("IP : %s\nPort : %d\n", inet_ntoa(inet_client_address), client_addr->sin_port); // if connected
            puts("===================================================");
            close(client_fd);
            continue;
        }
        else if (num_bytes == 0)
        { // if nothing in buf
            time_t disconnected_time = time(NULL);
            time_tm = localtime(&disconnected_time);
            strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);
            puts("===============Disconnected Client================");
            printf("[%s]\n", time_str);
            printf("IP : %s\nPort : %d\n", inet_ntoa(inet_client_address), client_addr->sin_port); // if connected
            puts("===================================================");
            close(client_fd); // close client socket
            continue;
        }
        else if (num_bytes > BUFFSIZE)
        { // if overflow in buf
            time_t disconnected_time = time(NULL);
            time_tm = localtime(&disconnected_time);
            strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);
            puts("===============Disconnected Client================");
            printf("[%s]\n", time_str);
            printf("IP : %s\nPort : %d\n", inet_ntoa(inet_client_address), client_addr->sin_port); // if connected
            puts("===================================================");
            close(client_fd); // close client socket
            continue;
        }

        strcpy(tmp, buf); // tmp = buf

        tok = strtok(tmp, " "); // cut by blank about tmp, get about GET
        strcpy(method, tok);    // requset method
        if (strcmp(method, "GET") == 0)
        {
            tok = strtok(NULL, " "); // cut by blank about tmp
            strcpy(url, tok);        // url = tok
        }
        else
        {
            time_t disconnected_time = time(NULL);
            time_tm = localtime(&disconnected_time);
            strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);
            puts("===============Disconnected Client================");
            printf("[%s]\n", time_str);
            printf("IP : %s\nPort : %d\n", inet_ntoa(inet_client_address), client_addr->sin_port); // if connected
            puts("==================================================");
            close(client_fd);
            continue;
        }
        // get unique name
        char filename[50];
        struct tm tm = *localtime(&executed_time);
        sprintf(filename, "%d-%02d-%02d_%02d-%02d-%02d_%03ld", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec, (long int)(executed_time % 1000));

        FILE *file = fopen(filename, "w");
        if (strcmp(url, "/") == 0)
        {
            create_ls(0, 1, url, client_fd, file, filename); // if directory it makes html file
        }
        else
        {
            while (url[strlen(url) - 1] == '/')
            { // delete last '/' for generalization
                url[strlen(url) - 1] = '\0';
            }
            if (url[0] == '\0')
            {
                create_ls(0, 1, url, client_fd, file, filename);
            }
            else
            {
                create_ls(1, 1, url, client_fd, file, filename); // if directory it makes html file
            }
        }
        fclose(file); // close file

        DIR *dirp;       // DIR pointer dirp
        char path[1024]; //
        getcwd(path, 1024);
        if (strcmp(url, "/") != 0)
        {                      // if not only port
            strcat(path, url); // add url after cwd
        }
        if ((dirp = opendir(path)) == NULL)
        { // if not directory
            int checkFile;
            if ((checkFile = open(path, O_RDONLY)) == -1)
            { // if not existing directory
                // char *filename = "html_ls.html";
                close(checkFile);
                // open html file
                int fd = open(filename, O_RDONLY);
                if (fd == -1)
                {
                    perror("open");
                    continue;
                }

                // get file size
                off_t offset = 0;
                struct stat file_stat;
                fstat(fd, &file_stat);              // get information about open file
                off_t filesize = file_stat.st_size; // get file size
                // error header
                sprintf(response_header, "HTTP/1.0 404 Not Found\r\n"
                                         "Server: 2019202021 advanced web server\r\n"
                                         "Content-length:%lu\r\n"
                                         "Content-type:text/html\r\n\r\n",
                        filesize);
                write(client_fd, response_header, strlen(response_header)); // send response header
                sendfile(client_fd, fd, &offset, filesize);                 // send file
                close(fd);
            }
            else
            { // if file
                if (fnmatch("*.jpg", path, FNM_CASEFOLD) == 0 || fnmatch("*.jpeg", path, FNM_CASEFOLD) == 0 ||
                    fnmatch("*.png", path, FNM_CASEFOLD) == 0)
                { // if image file
                    // get file size
                    off_t offset = 0;
                    struct stat file_stat;
                    fstat(checkFile, &file_stat);
                    off_t filesize = file_stat.st_size; // get file size
                    // image header
                    sprintf(response_header, "HTTP/1.0 200 OK\r\n"
                                             "Server: 2019202021 advanced web server\r\n"
                                             "Content-length:%lu\r\n"
                                             "Content-type:image/*\r\n\r\n",
                            filesize);
                    write(client_fd, response_header, strlen(response_header)); // send response header
                    sendfile(client_fd, checkFile, &offset, filesize);          // send file
                    close(checkFile);
                }
                else
                { // if plain file or symbolic link
                    off_t offset = 0;
                    struct stat file_stat;
                    lstat(path, &file_stat);
                    off_t filesize = file_stat.st_size;
                    if (S_ISLNK(file_stat.st_mode) != 0)
                    { // if link file
                        close(checkFile);
                        char linkpath[255];
                        ssize_t linklen = readlink(path, linkpath,
                                                   sizeof(linkpath)); // get about original of linkfile
                        linkpath[linklen] = '\0';                     // last char is NULL
                        char tmp_link[1024];
                        realpath(path, tmp_link);
                        printf("%s\n", tmp_link);
                        int linkFile;
                        if ((linkFile = open(tmp_link, O_RDONLY)) == -1)
                        { // if path is error
                            close(linkFile);
                            // make error html
                            FILE *file = fopen(filename, "w");
                            fprintf(file, "<body>\n");
                            fprintf(file, "<h1>Not Found</h1><br>");
                            fprintf(file,
                                    "The request URL %s was not found on this server<br>HTTP 404 - Not Page Found </body>",
                                    url);
                            fclose(file);
                            sprintf(response_header, "HTTP/1.0 404 Not Found\r\n"
                                                     "Server: 2019202021 advanced web server\r\n"
                                                     "Content-length:%lu\r\n"
                                                     "Content-type:text/html\r\n\r\n",
                                    filesize);
                            // open html file

                            int fd = open(filename, O_RDONLY);
                            if (fd == -1)
                            {
                                perror("open");
                                continue;
                            }
                            struct stat link_stat;
                            fstat(fd, &link_stat);
                            filesize = link_stat.st_size;
                            write(client_fd, response_header, strlen(response_header)); // send response header
                            sendfile(client_fd, fd, &offset, filesize);                 // send file
                            close(fd);
                        }
                        else
                        { // if there is link original file
                            struct stat link_stat;
                            fstat(linkFile, &link_stat);
                            filesize = link_stat.st_size;
                            if (fnmatch("*.jpg", tmp_link, FNM_CASEFOLD) == 0 ||
                                fnmatch("*.jpeg", tmp_link, FNM_CASEFOLD) == 0 ||
                                fnmatch("*.png", tmp_link, FNM_CASEFOLD) == 0)
                            { // if image file
                                sprintf(response_header, "HTTP/1.0 200 OK\r\n"
                                                         "Server: 2019202021 advanced web server\r\n"
                                                         "Content-length:%lu\r\n"
                                                         "Content-type:image/*\r\n\r\n",
                                        filesize);
                            }
                            else
                            { // if not image file
                                sprintf(response_header, "HTTP/1.0 200 OK\r\n"
                                                         "Server: 2019202021 advanced web server\r\n"
                                                         "Content-length:%lu\r\n"
                                                         "Content-type:text/plain\r\n\r\n",
                                        filesize);
                            }
                            write(client_fd, response_header, strlen(response_header)); // send response header
                            sendfile(client_fd, linkFile, &offset, filesize);           // send file
                            close(linkFile);
                        }
                    }
                    else
                    { // if not link file
                        // create text/plain header
                        sprintf(response_header, "HTTP/1.0 200 OK\r\n"
                                                 "Server: 2019202021 advanced web server\r\n"
                                                 "Content-length:%lu\r\n"
                                                 "Content-type:text/plain\r\n\r\n",
                                filesize);
                        write(client_fd, response_header, strlen(response_header)); // send response header
                        sendfile(client_fd, checkFile, &offset, filesize);          // send file
                        close(checkFile);
                    }
                }
            }
        }
        else
        { // if directory
            // if link folder file
            struct stat link_stat;
            lstat(path, &link_stat);
            if (S_ISLNK(link_stat.st_mode) != 0)
            {
                char tmp_link[1024];
                realpath(path, tmp_link);
                char cwd_link[1024];
                getcwd(cwd_link,1024);
                FILE *file = fopen(filename, "w");
                if(strcmp(tmp_link,cwd_link)==0){
                    dir_print(tmp_link,0,1,url,file,filename);
                }else{
                    dir_print(tmp_link,1,1,url,file,filename);
                }
                fclose(file);
            }

            // open html file
            int fd = open(filename, O_RDONLY);
            if (fd == -1)
            {
                perror("open");
                continue;
            }

            // get file size
            off_t offset = 0;
            struct stat file_stat;
            fstat(fd, &file_stat);
            off_t filesize = file_stat.st_size;
            // create text/plain header
            sprintf(response_header, "HTTP/1.0 200 OK\r\n"
                                     "Server: 2019202021 advanced web server\r\n"
                                     "Content-length:%lu\r\n"
                                     "Content-type:text/html\r\n\r\n",
                    filesize);
            write(client_fd, response_header, strlen(response_header)); // send response header
            sendfile(client_fd, fd, &offset, filesize);                 // send file
            close(fd);
        }
        time_t disconnected_time = time(NULL);
        time_tm = localtime(&disconnected_time);
        strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);
        puts("===============Disconnected Client================");
        printf("[%s]\n", time_str);
        printf("IP : %s\nPort : %d\n", inet_ntoa(inet_client_address), client_addr->sin_port); // if connected
        puts("===================================================");
        remove(filename);
        continue;
    }
}
//////////////////////////////////////////////////////////////////////////
// child_make                             								//
// =====================================================================//
// Input:  int i, int socketfd, int addrlen								//
// Ouput : pid_t pid when parent_process								//
// Purpose: for fork and making child           						//
//          for return child's pid                                      //
//////////////////////////////////////////////////////////////////////////
pid_t child_make(int i, int socketfd, int addrlen)
{
    pid_t pid;
    if ((pid = fork()) > 0)
    { // parent move out
        return (pid);
    }
    child_main(i, socketfd, addrlen);
}



int main(int argc, char *argv[])
{                                      // for server operation, not for create html file.
    time_t executed_time = time(NULL); // get executed_time
    char time_str[80];
    struct tm *time_tm;
    time_tm = localtime(&executed_time);
    strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);
    signal(SIGALRM, alarm_handler); // set signal alarm
    alarm(10);                      // alarm 10 second

    struct sockaddr_in server_addr, client_addr; // struct for server and client(browser)
    int socket_fd, client_fd;                    // server and client(browser) nonnegative descriptor
    unsigned int len, len_out;

    if ((socket_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {                                                // IPv4 internet protocol, TCP method
        printf("Server: Can't open stream socket."); // if cannot socket
        return 0;
    }

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &opt, sizeof(opt)); // set socket's option

    memset(&server_addr, 0, sizeof(server_addr));    // reset server_addr
    server_addr.sin_family = AF_INET;                // IPv4 Internet Protocol
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // host to network IP address
    server_addr.sin_port = htons(PORTNO);            // host to network port number

    if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {                                                  // binding socket and struct
        printf("Server: can't bind local address.\n"); // if cannot bind
        return 0;
    }
    printf("[%s] Server is started\n", time_str);
    listen(socket_fd, 5); // waiting for client connection, backlog = 5, if connected goto waiting queue

    maxNchildren = 5;
    pids = (pid_t *)malloc(maxNchildren * sizeof(pid_t));
    len = sizeof(client_addr);
    // preforking routine
    for (int i = 0; i < maxNchildren; i++)
    {
        pids[i] = child_make(i, socket_fd, len); // pids = child processes's pid
    }
    //child never come here!
    signal(SIGINT, parent_handler);
    signal(SIGTERM, parent_handler);
    int status;
    time_t terminated_time;
    
    //wait for 5 child process
    wait(&status);
    wait(&status);
    wait(&status);
    wait(&status);
    wait(&status);
    terminated_time = time(NULL);
    time_tm = localtime(&terminated_time);
    strftime(time_str, sizeof(time_str), "%a %b %d %H:%M:%S %Y", time_tm);
    printf("[%s] Server is terminated\n", time_str);

    close(socket_fd); // close socket_fd
    return 0;
}
