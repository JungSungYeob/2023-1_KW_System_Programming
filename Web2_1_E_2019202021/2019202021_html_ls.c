/////////////////////////////////////////////////////////////////////////
// File Name   : 2019202021_html_ls.c									//
// Date        : 2023/4/19												//
// OS		: Ubuntu 16.04.5 LTS 64bits									//
// Author	: Jung Sung Yeob											//
// Student Id	: 2019202021											//
// ---------------------------------------------------------------------//
// Title : System Programming Assignment #2-1 (html_ls)					//
// Description : Coding html file by c language							//
//				html document for ls_command							//
//				html document cotains									//
//					-current directory : title							//
//					-command : heading									//
//					-results of ls command : Table						//
//					-file names : hyperlink to the file					//
//					-output file name : html_ls.html					//
//				+)open & check this html page on browser				//
/////////////////////////////////////////////////////////////////////////
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
// print_readableSIZE                    								//
// =====================================================================//
// Input:   size_t size													//
// Ouput : print file's readable size									//
//			no return													//
// Purpose: for print file's readable size	when option -h				//
//////////////////////////////////////////////////////////////////////////
void print_readableSIZE(size_t size,FILE *file){
	long unsigned int readable_size = size; //option -h print size as readable like G, M, K
			if(readable_size/1000000000>0){ //if Giga
				readable_size/=100000000; //divide 10^8
				if(readable_size%10>=5){ //if have to round number
					readable_size+=10; //round num
				}
				readable_size/=10; //divide 10
				fprintf(file,"<td>%luG</td>",readable_size);
			}else if(readable_size/1000000>0){//if Mega
				readable_size/=100000;//divide 10^5
				if(readable_size%10>=5){//if have to round number
					readable_size+=10; //round num
				}
				readable_size/=10; //divide 10
				fprintf(file,"<td>%luM</td>",readable_size);
			}else if(readable_size/1000>0){ //if Kilo
				readable_size/=100; //divide 10^2
				if(readable_size%10>=5){ //if have to round number
					readable_size+=10; //round num
				}
				readable_size/=10; //divide 10
				fprintf(file,"<td>%luK</td>",readable_size); //print readable size
			}else{ //if cannot divide
				fprintf(file,"<td>%lu</td>",readable_size); //print readable size
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
// Input:   char* dir_name, int show_size								//
// Ouput : print file's info by long format								//
//			no return													//
// Purpose: for print file's info by long format						//
//			if -h option print readable size							//
//////////////////////////////////////////////////////////////////////////
void print_long(struct stat file_stat, char*dir_name, int show_size, char*abs_path, FILE *file){ //option -l print long format
	fprintf(file,"<td><a href=\"%s/%s\">%s</a></td>",abs_path,dir_name,dir_name);	//print file name
	print_permission(file_stat.st_mode,file); //print permission
	fprintf(file,"<td>\t%lu\t</td>",file_stat.st_nlink); //print nlink num
	print_UID(file_stat.st_uid,file); //print UID
	print_GID(file_stat.st_gid,file); //print GID
	if(show_size==1){ //if -h
		print_readableSIZE(file_stat.st_size,file); //print readable size
	}else{
		fprintf(file,"<td>%lu\t</td>",file_stat.st_size); //print size
	}
	print_LastModified_time(file_stat.st_mtime,file); //print last modified time
	fprintf(file,"</tr>\n");
}
//////////////////////////////////////////////////////////////////////////
// file_print			                 								//
// =====================================================================//
// Input:   char* dir_name, int flag_l, int flag_h, FILE *file			//
// Ouput : print file's info											//
//			no return													//
// Purpose: for print file's info										//
//			if -l option has activated, use function print_long			//
//			else print file name										//
//////////////////////////////////////////////////////////////////////////
void file_print(char *dir_name, int flag_l, int flag_h, FILE *file){
	struct stat file_stat; //struct file_stat
	char *cwdOK; //for get realpath checking
	char cwd[255]; //get realpath element
	char *realcwd; //file's realcwd
	char abs_path[PATH_MAX];//file's absolute path
	cwdOK=realpath(dir_name,cwd);//get realpath
	if(cwdOK==NULL){
		fprintf(file,"<h4>html_ls: cannot access realpath : No such directory</h4>\n"); //error message
		exit(1);
	}
	realcwd=dirname(cwd);//delete last dir
	realpath(realcwd,abs_path);//get absolute path
	lstat(dir_name,&file_stat);//stat about dir_name
	if(S_ISLNK(file_stat.st_mode)!=0){ //if link file
		fprintf(file,"<tr style=\"color:green\">");//color = green
	}else{ //else 
		fprintf(file,"<tr style=\"color:red\">");//color = red
	}
	int lastpath=-1;
	char file_name[255];
	for(int i=0;i<strlen(dir_name);i++){
		if(dir_name[i]=='/'){
			lastpath=i;
		}
	}
	if(lastpath!=-1){
		for(int i=lastpath;i<strlen(dir_name)-1;i++){
			file_name[i-lastpath]=dir_name[i+1];
		}
	}else{
		strcpy(file_name,dir_name);
	}

	if(flag_l==0){ //not -l option
		fprintf(file,"<td><a href=\"%s/%s\">%s</a></td><tr>\n",abs_path,file_name, file_name);//print info
	}else if(flag_l==1){//-l option
		print_long(file_stat,file_name,flag_h,abs_path,file);//print info
	}
	for(int i=0;i<255;i++){
		file_name[i]='\0';
	}
}
//////////////////////////////////////////////////////////////////////////
// dir_print			                 								//
// =====================================================================//
// Input:   char* dir_name, int flag_a, int flag_l, int flag_h,			//
//			int flag_S, int flag_r, int_wildcard, File *file			//
// Ouput : print dir's info												//
//			no return													//
// Purpose: for print dir's files info			 						//
//			if -a option has activated,	print all files					//
//			if -l option has activated, use function print_all			//
//			if -h option has activated, show readable size				//
//			if -S option has activated, sort by size descending order	//
//			if -r option has activated, reverse order					//
//			else print file name										//
//////////////////////////////////////////////////////////////////////////
void dir_print(char*dir_name, int flag_a, int flag_l, int flag_h, int flag_S, int flag_r, int flag_wildcard, FILE *file){
	DIR *dirp; //DIR pointer for dirp
	struct dirent *dir;//struct dirent pointer for dir
	struct stat file_stat;//struct stat
	struct file_info{//create file_info struct for name and stat
		char name[255];
		struct stat st;
	};

	if((dirp=opendir(dir_name))==NULL){//opendir about dir_name
		fprintf(file,"<h4>html_ls: cannot access realpath : No such directory</h4>\n");
		exit(1);
	}

	struct file_info files[1024]; //array for struct
	int num_cnt=0;//count contents in directory
	int total_size=0;//total block size
	while((dir = readdir(dirp))!=NULL){//read content in dirp
		if(flag_a||dir->d_name[0]!='.'){//-a option or hidden file
			char tmppath[255];//temp path 
			snprintf(tmppath,PATH_MAX,"%s/%s",dir_name,dir->d_name);//make path dir_name/dir->d_name
			if(lstat(tmppath,&files[num_cnt].st)==0){//get stat about tmppath
				strcpy(files[num_cnt].name, dir->d_name);//get name
				total_size+=(files[num_cnt].st.st_blocks/2);//sum size to total
				num_cnt++;//up count
			}
		}
	}

	if(flag_S==0){ // if -S option has not activated
		//bubble sort
    	for (int i = 0; i < num_cnt-1; i++) { 
        	for (int j = 0; j < num_cnt-i-1; j++) {
            	char cmp_tmp1[255];//tmp value
            	strcpy(cmp_tmp1,files[j].name); //get name value to cmp_tmp1
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
	}else{
		//sort by size descending order
		for(int i=0;i<num_cnt-1;i++){
			for(int j=0;j<num_cnt-i-1;j++){
				long unsigned int tmp1 = files[j].st.st_size; //tmp1 value about size of files[j]
				long unsigned int tmp2 = files[j+1].st.st_size; //tmp2 value about size of files[j+1]
				if(tmp1<tmp2){ //if tmp2 is bigger than tmp1, change location
					struct file_info temp = files[j];
					files[j] = files[j+1];
					files[j+1] = temp;
				}else if(tmp1==tmp2){ //if same size: string upside down
					char cmp_tmp1[255];//tmp value
            		strcpy(cmp_tmp1,files[j].name); //get name value to cmp_tmp1
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
            		if (strcasecmp(cmp_tmp1, cmp_tmp2) < 0) { //compare name, if com_tmp1 is bigger than cmp_tmp2, change location
                		struct file_info temp = files[j];
                		files[j] = files[j+1];
                		files[j+1] = temp;
            		}
				}
			}
		}
	}
	char *cwdOK; //get cwd ok
	char cwd[255]; // get cwd
	cwdOK=realpath(dir_name,cwd);//get realpath include last dir
	if(cwdOK==NULL){
		fprintf(file,"<h4>html_ls: cannot access realpath : No such directory</h4>\n"); //error message
		exit(6);
	}

	if(flag_wildcard==1&&flag_l==0){//no -l option but wildcard exist
		fprintf(file,"<b>Directory path: %s</b>\n",cwd); //print directory path
	}else if(flag_l==1){ // -l option
		fprintf(file,"<b>Directory path: %s</b>\n",cwd); //print directory path
		fprintf(file,"<BR><b>total: %ld</b>\n",(long)(total_size)); //print total size
	}
	fprintf(file,"<table border= \"1\">\n"); //table start
	if(flag_l==0){ //no -l option
		fprintf(file,"<tr><th>Name</th></tr>\n");
		if(flag_r==0){ //if -r option has not activated
			for(int i=0;i<num_cnt;i++){
				if(strcmp("html_ls.html",files[i].name)==0)//if filename is html_ls.html => skip
					continue;
				if(S_ISLNK(files[i].st.st_mode)!=0){//if link file
					fprintf(file,"<tr style=\"color:green\">");//color = green
				}else if(S_ISDIR(files[i].st.st_mode)!=0){//if dir
					fprintf(file,"<tr style=\"color:blue\">");//color = blue
				}else{//else
					fprintf(file,"<tr style=\"color:red\">");//color = red
				}
				fprintf(file,"<td><a href=\"%s/%s\">%s</a></td></tr>\n",cwd,files[i].name,files[i].name); //print files[i].name
			}
		}else{ //if -r option has activated
			for(int i=num_cnt-1;i>=0;i--){ //reverse
				if(strcmp("html_ls.html",files[i].name)==0)//if filename is html_ls.html => skip
					continue;
				if(S_ISLNK(files[i].st.st_mode)!=0){//if link file
					fprintf(file,"<tr style=\"color:green\">");//color = green
				}else if(S_ISDIR(files[i].st.st_mode)!=0){//if dir
					fprintf(file,"<tr style=\"color:blue\">");//color = blue
				}else{//else
					fprintf(file,"<tr style=\"color:red\">");//color =red
				}
				fprintf(file,"<td><a href=\"%s/%s\">%s</a></td></tr>\n",cwd,files[i].name ,files[i].name); //print files[i].name
			}
		}
	}else{ //-l option
		fprintf(file,"<tr><th>NAME</th><th>Permission</th><th>Link</th><th>Owner</th><th>Group</th><th>Size</th><th>Last Modified</th></tr>\n");
		if(flag_r==0){//if -r option has not activated
			for(int i=0;i<num_cnt;i++){ //print file list by long format
				struct stat file_stat = files[i].st;
				if(strcmp("html_ls.html",files[i].name)==0)//if filename is html_ls.html => skip
					continue;
				if(S_ISLNK(files[i].st.st_mode)!=0){//if link file
					fprintf(file,"<tr style=\"color:green\">");//color = green
				}else if(S_ISDIR(files[i].st.st_mode)!=0){//if dir
					fprintf(file,"<tr style=\"color:blue\">");//color = blue
				}else{//else
					fprintf(file,"<tr style=\"color:red\">");//color = red
				}
				print_long(file_stat,files[i].name,flag_h,cwd,file);//print file info by long format
			}
		}else{//if -r option has activated
			for(int i=num_cnt-1;i>=0;i--){//print file list by long format reverse
				struct stat file_stat = files[i].st;
				if(strcmp("html_ls.html",files[i].name)==0)//if filename is html_ls.html => skip
					continue;
				if(S_ISLNK(files[i].st.st_mode)!=0){//if link file
					fprintf(file,"<tr style=\"color:green\">");//color = green
				}else if(S_ISDIR(files[i].st.st_mode)!=0){//if dir
					fprintf(file,"<tr style=\"color:blue\">");//color = blue
				}else{//else
					fprintf(file,"<tr style=\"color:red\">");//color = red
				}
				print_long(file_stat,files[i].name,flag_h,cwd,file);//print file info by long format
			}
		}
		
	}
	fprintf(file,"</table>\n");//end table
	closedir(dirp); //close dirp;
	free(dir);
}

//////////////////////////////////////////////////////////////////////////
// manager				                   								//
// =====================================================================//
// Input: char* dir_name, int flag_a, int flag_l, int flag_h,			//
//				int flag_S, int flag_r,int flag_wildcard, FILE *file	//
// Ouput : print file's size											//
//			no return													//
// Purpose: for print file's size			 							//
//////////////////////////////////////////////////////////////////////////

void manager(char *dir_name, int flag_a, int flag_l, int flag_h, int flag_S, int flag_r, int flag_wildcard, FILE *file){
	DIR *dirp; //DIR pointer dirp
	struct dirent *tmp; //struct dirent pointer tmp
	int checkFile;//check for file
	int isFile=0;//is this file?
	
	if((dirp=opendir(dir_name))==NULL){//opendir dir_name
		if((checkFile=open(dir_name,O_RDONLY))==-1){//if not file
			fprintf(file,"<h4>html_ls: cannot access \'%s\' : No such directory</h4>\n",dir_name);//print error message
			exit(2);
		}else{
			isFile=1; //it's file!
		}
	}
	close(checkFile);//close file
	if(dirp!=NULL){//if dirp has opened
		closedir(dirp); //close dir
	}
	if(isFile==1){//if file
		file_print(dir_name,flag_l,flag_h,file);//print file_info
		return;
	}else{
		dir_print(dir_name,flag_a,flag_l,flag_h,flag_S,flag_r,flag_wildcard, file);//print dir contents_info
		return;
	}
}


int main(int argc, char *argv[]){
	DIR *dirp;//DIR pointer dirp
	struct dirent *dir;//struct dirent pointer dir
	int opt; //option element
	int flag_a = 0;//-a option
	int flag_l = 0;//-l option
	int flag_h = 0;//-h option
	int flag_S = 0;//-S option
	int flag_r = 0;//-r option
	int flag_wildcard=0;//if there is wildcard
	char *dir_name =".";//dir_name is current dir
	FILE *file=fopen("html_ls.html","w");//file open with write
	if(file==0){
		printf("error\n");//if cannot open print error message
		return -1;
	}
	fprintf(file,"<html>\n");
	fprintf(file,"<head>\n");
	char cwd[1024];
	getcwd(cwd,1024);//get current working directory
	fprintf(file,"<title>%s</title>\n",cwd);//set title
	fprintf(file,"<head>\n");
	fprintf(file,"<body>\n");
	//print command
	fprintf(file,"<h1>");//print command
	for(int i=0;i<argc;i++){
		fprintf(file,"%s ",argv[i]);
	}
	fprintf(file,"</h1>\n");//end heading
	//print command end
	while((opt=getopt(argc,argv,"alhSr"))!=-1){
		switch(opt)
		{
			case 'a': // -a option
				flag_a=1;
				break;
			case 'l': // -l option
				flag_l=1;
				break;
			case 'h': // -h option
				flag_h=1;
				break;
			case 'S': // -S option
				flag_S=1;
				break;
			case 'r': // -r option
				flag_r=1;
				break;
			case '?': //else option
				fprintf(file,"<h4>Unknow option character</h4>\n");
				exit(1);
		}
	}
	//tmp sort_argv
	char *sort_argv[255];
	int cnt=0;
	for(int i=optind;i<argc;i++){//get argv content to modify
		sort_argv[i-optind]=argv[i];
		cnt++;
	}

	int wild_cnt=0;
	//if wildcard is there, another argv have to print it's direction
	for(int i=0;i<cnt;i++){
		for(int j=0;j<strlen(sort_argv[i]);j++){
			if(sort_argv[i][j]=='*'){ //if there is '*'
				flag_wildcard=1;//set flag_wildcard
				int lastpath=-1;//not absolute path
				char path[255];
				char pattern[255];
				for(int l=0;l<strlen(sort_argv[i]);l++){
					if(sort_argv[i][l]=='/'){//if there is '/'
						lastpath=l; //savepoint
					}
				}
				if(lastpath==-1){//not absolute path
					strcpy(path,"."); //current working dir
					strcpy(pattern,sort_argv[i]);//argv is pattern
				}else{//absolute path
					strncpy(path,sort_argv[i],lastpath);//copy path
					for(int l=lastpath;l<strlen(sort_argv[i])-1;l++){
						pattern[l-lastpath]=sort_argv[i][l+1];//get pattern
					}
				}
				if((dirp=opendir(path))==NULL){//open dir path
					fprintf(file,"<h4>html_ls: cannot access \'%s\' : No such directory</h4>\n",path); //print error message
					exit(3);
				}
				while((dir=readdir(dirp))!=NULL){ //read dir content
					if(dir->d_name[0]=='.'&&pattern[0]!='.'){//if hidden file but pattern[0] is not '.'
						continue;
					}else if(pattern[0]=='.'&&dir->d_name[0]!='.'){//not hidden file but pattern[0] is '.'
						continue;
					}
					
					if(fnmatch(pattern,dir->d_name,0)==0){ //matching with pattern and dir->d_name with wildcard
						char tmp[255];
						strcpy(tmp,dir->d_name);
						sort_argv[cnt]=dir->d_name;
						strcpy(sort_argv[cnt],path);
						strcat(sort_argv[cnt],"/");
						strcat(sort_argv[cnt],tmp);
						cnt++;
					}
				}
				wild_cnt++;
				break;
			}else if(sort_argv[i][j]=='?'){//if there is '?'
				flag_wildcard=1;//set flag_wildcard
				int lastpath=-1;//not absolute path
				char path[255];
				char pattern[255];
				for(int l=0;l<strlen(sort_argv[i]);l++){
					if(sort_argv[i][l]=='/'){//if there is '/'
						lastpath=l;//savepoint
					}
				}
				if(lastpath==-1){//not absolute path
					strcpy(path,".");//current working dir
					strcpy(pattern,sort_argv[i]);//argv is pattern
				}else{//absolute path
					strncpy(path,sort_argv[i],lastpath);//copy path
					for(int l=lastpath;l<strlen(sort_argv[i])-1;l++){
						pattern[l-lastpath]=sort_argv[i][l+1];//get pattern
					}
				}
				char *cwdOK;
				char abs_path[255];
				cwdOK=realpath(path,abs_path);
				if((dirp=opendir(abs_path))==NULL){
					fprintf(file,"<h4>html_ls: cannot access \'%s\' : No such directory</h4>\n",path);
					exit(4);
				}
				while((dir=readdir(dirp))!=NULL){//read dir content
					if(dir->d_name[0]=='.'&&pattern[0]!='.'){//if hidden file but pattern[0] is not '.'
						continue;
					}else if(pattern[0]=='.'&&dir->d_name[0]!='.'){//not hidden file but pattern[0] is '.'
						continue;
					}
					if(fnmatch(pattern,dir->d_name,0)==0){//matching with pattern and dir->d_name with wildcard
						char tmp[255];
						strcpy(tmp,dir->d_name);
						sort_argv[cnt]=dir->d_name;
						strcpy(sort_argv[cnt],path);
						strcat(sort_argv[cnt],"/");
						strcat(sort_argv[cnt],tmp);
						cnt++;
					}
				}
				wild_cnt++;
				break;
			}else if(sort_argv[i][j]=='['){
				for(int k=j;k<strlen(sort_argv[i]);k++){
					if(sort_argv[i][k]==']'){
						flag_wildcard=1;
						int lastpath=-1;
						char path[255];
						char pattern[255];
						for(int l=0;l<strlen(sort_argv[i]);l++){
							if(sort_argv[i][l]=='/'){
								lastpath=l;
							}
						}
						if(lastpath==-1){//not absolute path
							strcpy(path,".");//current working dir
							strcpy(pattern,sort_argv[i]);//argv is pattern
						}else{//absolute path
							strncpy(path,sort_argv[i],lastpath);//copy path
							for(int l=lastpath;l<strlen(sort_argv[i])-1;l++){
								pattern[l-lastpath]=sort_argv[i][l+1];//get pattern
							}
						}
						char *cwdOK;
						char abs_path[255];
						cwdOK=realpath(path,abs_path);
						if((dirp=opendir(abs_path))==NULL){
							fprintf(file,"<h4>html_ls: cannot access \'%s\' : No such directory</h4>\n",path);
							exit(5);
						}
						while((dir=readdir(dirp))!=NULL){//read dir content
							if(dir->d_name[0]=='.'&&pattern[0]!='.'){//if hidden file but pattern[0] is not '.'
								continue;
							}else if(pattern[0]=='.'&&dir->d_name[0]!='.'){//not hidden file but pattern[0] is '.'
								continue;
							}
							
							if(fnmatch(pattern,dir->d_name,0)==0){//matching with pattern and dir->d_name with wildcard
								char tmp[255];
								strcpy(tmp,dir->d_name);
								sort_argv[cnt]=dir->d_name;
								strcpy(sort_argv[cnt],path);
								strcat(sort_argv[cnt],"/");
								strcat(sort_argv[cnt],tmp);
								cnt++;
							}
						}
						wild_cnt++;
						break;
					}
				}
			}
		}
	}



	int file_cnt=0; //file count 
	char *sort_file[255];//sorting list of file
	int dir_cnt=0; //dir count 
	char *sort_dir[255];//sorting list of dir
	for(int i=0;i<cnt;i++){
		char tmp[255];
		char tmp2[255];
		strcpy(tmp2,sort_argv[i]);
		realpath(sort_argv[i],tmp);
		struct stat tmp_stat;
		lstat(tmp,&tmp_stat);
		//fprintf(file,"%s\t",sort_argv[i]); 
		char *ptr=strtok(tmp2,"/");
		int html=0;
		while(ptr!=NULL){
			if(strcmp(ptr,"html_ls.html")==0){
				html=1;
			}
			ptr=strtok(NULL,"/");
		}
		if(html==1){
			continue;
		}
		if(S_ISDIR(tmp_stat.st_mode)==0){ //if file
			if(open(sort_argv[i],O_RDONLY)==-1)
				continue;
			sort_file[file_cnt++]=sort_argv[i];
		}else{
			if(opendir(sort_argv[i])==NULL){
				continue;
			}
			sort_dir[dir_cnt++]=sort_argv[i];
		}
	}
	
	
	if(flag_S==0){// -S option
		for(int i=0;i<file_cnt-1;i++){
			for(int j=0;j<file_cnt-i-1;j++){
				char cmp_tmp1[255];//tmp file
				strcpy(cmp_tmp1,sort_file[j]);//get name

				int lastpath1=-1;
				char pattern1[255];
				for(int k=0;k<strlen(cmp_tmp1);k++){
					if(cmp_tmp1[k]=='/'){
						lastpath1=k;
					}
				}
				if(lastpath1!=-1){
					for(int k=lastpath1;k<strlen(cmp_tmp1)-1;k++){
						pattern1[k-lastpath1]=cmp_tmp1[k+1];
					}
				}else{
					strcpy(pattern1,cmp_tmp1);
				}

				if(pattern1[0]=='.'){//if hidden file
            		for(int k=0;pattern1[k]!='\0';k++){ //shift left
                		pattern1[k]=pattern1[k+1]; 
           			}
       			}
				char cmp_tmp2[255];//tmp file
				strcpy(cmp_tmp2,sort_file[j+1]);//get name

				int lastpath2=-1;
				char pattern2[255];
				for(int k=0;k<strlen(cmp_tmp2);k++){
					if(cmp_tmp2[k]=='/'){
						lastpath2=k;
					}
				}
				if(lastpath2!=-1){
					for(int k=lastpath2;k<strlen(cmp_tmp2)-1;k++){
						pattern2[k-lastpath2]=cmp_tmp2[k+1];
					}
				}else{
					strcpy(pattern2,cmp_tmp2);
				}

				if(pattern2[0]=='.'){//if hidden file
        	    	for(int k=0;pattern2[k]!='\0';k++){ //shift left
            	    	pattern2[k]=pattern2[k+1]; 
           			}
       			}
				//fprintf(file,"%s   %s <BR>",pattern1,pattern2);
				if(strcasecmp(pattern1,pattern2)>0){ //if cmp_tmp1>cmp_tmp2 swap location
					char temp[255]; //temp
					strcpy(temp,sort_file[j]);
					strcpy(sort_file[j],sort_file[j+1]);
					strcpy(sort_file[j+1],temp);
				}
				for(int l=0;l<255;l++){
					pattern1[l]='\0';
					pattern2[l]='\0';
				}
			}
		}
		for(int i=0;i<dir_cnt-1;i++){
			for(int j=0;j<dir_cnt-i-1;j++){
				char cmp_tmp1[255];//tmp file
				strcpy(cmp_tmp1,sort_dir[j]);//get name

				int lastpath1=-1;
				char pattern1[255];
				for(int k=0;k<strlen(cmp_tmp1);k++){
					if(cmp_tmp1[k]=='/'){
						lastpath1=k;
					}
				}
				if(lastpath1!=-1){
					for(int k=lastpath1;k<strlen(cmp_tmp1)-1;k++){
						pattern1[k-lastpath1]=cmp_tmp1[k+1];
					}
				}else{
					strcpy(pattern1,cmp_tmp1);
				}

				if(pattern1[0]=='.'){//if hidden file
            		for(int k=0;pattern1[k]!='\0';k++){ //shift left
                		pattern1[k]=pattern1[k+1]; 
           			}
       			}
				char cmp_tmp2[255];//tmp file
				strcpy(cmp_tmp2,sort_dir[j+1]);//get name

				int lastpath2=-1;
				char pattern2[255];
				for(int k=0;k<strlen(cmp_tmp2);k++){
					if(cmp_tmp2[k]=='/'){
						lastpath2=k;
					}
				}
				if(lastpath2!=-1){
					for(int k=lastpath2;k<strlen(cmp_tmp2)-1;k++){
						pattern2[k-lastpath2]=cmp_tmp2[k+1];
					}
				}else{
					strcpy(pattern2,cmp_tmp2);
				}

				if(pattern2[0]=='.'){//if hidden file
        	    	for(int k=0;pattern2[k]!='\0';k++){ //shift left
            	    	pattern2[k]=pattern2[k+1]; 
           			}
       			}
				//fprintf(file,"%s   %s <BR>",pattern1,pattern2);
				if(strcasecmp(pattern1,pattern2)>0){ //if cmp_tmp1>cmp_tmp2 swap location
					char temp[255]; //temp
					strcpy(temp,sort_dir[j]);
					strcpy(sort_dir[j],sort_dir[j+1]);
					strcpy(sort_dir[j+1],temp);
				}
				for(int l=0;l<255;l++){
					pattern1[l]='\0';
					pattern2[l]='\0';
				}
			}
		}
	}else if(flag_S==1){
		for(int i=0;i<file_cnt-1;i++){
			for(int j=0;j<file_cnt-i-1;j++){
				long unsigned int tmp1_size=0;
				long unsigned int tmp2_size=0;
				char tmp1[255];//tmp1
				char *check;
				check=realpath(sort_file[j],tmp1);//get realpath
				char tmp2[255];//tmp2
				check=realpath(sort_file[j+1],tmp2);//get realpath
				struct stat file_stat1;//struct stat
				struct stat file_stat2;//struct stat
				if(lstat(tmp1,&file_stat1)==0){//get stat of tmp1
					tmp1_size=file_stat1.st_size;//size
				}
				if(lstat(tmp2,&file_stat2)==0){//get stat of tmp2
					tmp2_size=file_stat2.st_size;//size
				}
				if((int)tmp1_size<(int)tmp2_size){//if bigger swap
					char temp2[255];
					strcpy(temp2,sort_file[j]);
					strcpy(sort_file[j],sort_file[j+1]);
					strcpy(sort_file[j+1],temp2);
				}else if((int)tmp1_size==(int)tmp2_size){//if file size is same compare with name
					char cmp_tmp1[255];//tmp file
					strcpy(cmp_tmp1,sort_file[j]);//get name

					int lastpath1=-1;
					char pattern1[255];
					for(int k=0;k<strlen(cmp_tmp1);k++){
						if(cmp_tmp1[k]=='/'){
							lastpath1=k;
						}
					}
					if(lastpath1!=-1){
						for(int k=lastpath1;k<strlen(cmp_tmp1)-1;k++){
							pattern1[k-lastpath1]=cmp_tmp1[k+1];
						}
					}else{
						strcpy(pattern1,cmp_tmp1);
					}

					if(pattern1[0]=='.'){//if hidden file
            			for(int k=0;pattern1[k]!='\0';k++){ //shift left
                			pattern1[k]=pattern1[k+1]; 
           				}
       				}
					char cmp_tmp2[255];//tmp file
					strcpy(cmp_tmp2,sort_file[j+1]);//get name

					int lastpath2=-1;
					char pattern2[255];
					for(int k=0;k<strlen(cmp_tmp2);k++){
						if(cmp_tmp2[k]=='/'){
							lastpath2=k;
						}
					}
					if(lastpath2!=-1){
						for(int k=lastpath2;k<strlen(cmp_tmp2)-1;k++){
							pattern2[k-lastpath2]=cmp_tmp2[k+1];
						}
					}else{
						strcpy(pattern2,cmp_tmp2);
					}

					if(pattern2[0]=='.'){//if hidden file
        		    	for(int k=0;pattern2[k]!='\0';k++){ //shift left
            		    	pattern2[k]=pattern2[k+1]; 
           				}
	       			}
					//fprintf(file,"%s   %s <BR>",pattern1,pattern2);
					if(strcasecmp(pattern1,pattern2)<0){ //if cmp_tmp1>cmp_tmp2 swap location
						char temp[255]; //temp
						strcpy(temp,sort_file[j]);
						strcpy(sort_file[j],sort_file[j+1]);
						strcpy(sort_file[j+1],temp);
					}
					//fprintf(file,"%s // %s <BR>",pattern1,pattern2);
					for(int l=0;l<255;l++){
						pattern1[l]='\0';
						pattern2[l]='\0';
					}
				}
				for(int l=0;l<255;l++){
					tmp1[l]='\0';
					tmp2[l]='\0';
				}
			}
		}
		for(int i=0;i<dir_cnt-1;i++){
			for(int j=0;j<dir_cnt-i-1;j++){
				long unsigned int tmp1_size=0;
				long unsigned int tmp2_size=0;
				char tmp1[255];//tmp1
				char *check;
				check=realpath(sort_dir[j],tmp1);//get realpath
				char tmp2[255];//tmp2
				check=realpath(sort_dir[j+1],tmp2);//get realpath
				struct stat file_stat1;//struct stat
				struct stat file_stat2;//struct stat
				if(lstat(tmp1,&file_stat1)==0){//get stat of tmp1
					tmp1_size=file_stat1.st_size;//size
				}
				if(lstat(tmp2,&file_stat2)==0){//get stat of tmp2
					tmp2_size=file_stat2.st_size;//size
				}
				if((int)tmp1_size<(int)tmp2_size){//if bigger swap
					char temp2[255];
					strcpy(temp2,sort_dir[j]);
					strcpy(sort_dir[j],sort_dir[j+1]);
					strcpy(sort_dir[j+1],temp2);
				}else if((int)tmp1_size==(int)tmp2_size){//if file size is same compare with name
					char cmp_tmp1[255];//tmp file
					strcpy(cmp_tmp1,sort_dir[j]);//get name

					int lastpath1=-1;
					char pattern1[255];
					for(int k=0;k<strlen(cmp_tmp1);k++){
						if(cmp_tmp1[k]=='/'){
							lastpath1=k;
						}
					}
					if(lastpath1!=-1){
						for(int k=lastpath1;k<strlen(cmp_tmp1)-1;k++){
							pattern1[k-lastpath1]=cmp_tmp1[k+1];
						}
					}else{
						strcpy(pattern1,cmp_tmp1);
					}

					if(pattern1[0]=='.'){//if hidden file
       		     		for(int k=0;pattern1[k]!='\0';k++){ //shift left
       	 	        		pattern1[k]=pattern1[k+1]; 
    	       			}
	       			}
					char cmp_tmp2[255];//tmp file
					strcpy(cmp_tmp2,sort_dir[j+1]);//get name	

					int lastpath2=-1;
					char pattern2[255];
					for(int k=0;k<strlen(cmp_tmp2);k++){
						if(cmp_tmp2[k]=='/'){
							lastpath2=k;
						}
					}
					if(lastpath2!=-1){
						for(int k=lastpath2;k<strlen(cmp_tmp2)-1;k++){
							pattern2[k-lastpath2]=cmp_tmp2[k+1];
						}
					}else{
						strcpy(pattern2,cmp_tmp2);
					}	

					if(pattern2[0]=='.'){//if hidden file
   	     	    		for(int k=0;pattern2[k]!='\0';k++){ //shift left
   	         	 	   	pattern2[k]=pattern2[k+1]; 
   	        			}
   	    			}
					//fprintf(file,"%s   %s <BR>",pattern1,pattern2);
					if(strcasecmp(pattern1,pattern2)<0){ //if cmp_tmp1>cmp_tmp2 swap location
						char temp[255]; //temp
						strcpy(temp,sort_dir[j]);
						strcpy(sort_dir[j],sort_dir[j+1]);
						strcpy(sort_dir[j+1],temp);
					}
					for(int l=0;l<255;l++){
						pattern1[l]='\0';
						pattern2[l]='\0';
					}
				}
			}
		}
	}



	

	if(optind<argc){
		if(flag_r==0){
			if(flag_l&&file_cnt>0&&dirp){
				fprintf(file,"<table border= \"1\">\n");
				fprintf(file,"<tr><th>NAME</th><th>Permission</th><th>Link</th><th>Owner</th><th>Group</th><th>Size</th><th>Last Modified</th></tr>\n");
			}else if(flag_l==0&&file_cnt>0){
				fprintf(file,"<table border= \"1\">\n");
				fprintf(file,"<tr><th>Name</th></tr>\n");
			}

			for(int i=0;i<file_cnt;i++){
				dir_name=sort_file[i];
				int checkFILE;
				if((checkFILE=open(dir_name,O_RDONLY))==-1){//if not file
					continue;
				}

				manager(dir_name,flag_a,flag_l,flag_h,flag_S,flag_r,flag_wildcard,file);
			}
			if(file_cnt>0)
				fprintf(file,"</table>\n");
			for(int i=0;i<dir_cnt;i++){
				dir_name=sort_dir[i];
				manager(dir_name,flag_a,flag_l,flag_h,flag_S,flag_r,flag_wildcard,file);
			}
		}else{
			fprintf(file,"<table border= \"1\">\n");
			if(flag_l&&file_cnt>0){
				fprintf(file,"<table border= \"1\">\n");
				fprintf(file,"<tr><th>NAME</th><th>Permission</th><th>Link</th><th>Owner</th><th>Group</th><th>Size</th><th>Last Modified</th></tr>\n");
			}else if(flag_l==0&&file_cnt>0){
				fprintf(file,"<table border= \"1\">\n");
				fprintf(file,"<tr><th>Name</th></tr>\n");
			}
			for(int i=file_cnt-1;i>=0;i--){
				dir_name=sort_file[i];
				int checkFILE;
				if((checkFILE=open(dir_name,O_RDONLY))==-1){//if not file
					continue;
				}
				char *str = strtok(dir_name,"/");
				while(str!=NULL){
					str = strtok(NULL,"/");
				}
				manager(dir_name,flag_a,flag_l,flag_h,flag_S,flag_r,flag_wildcard,file);
			}
			fprintf(file,"</table>\n");
			for(int i=dir_cnt-1;i>=0;i--){
				dir_name=sort_dir[i];
				manager(dir_name,flag_a,flag_l,flag_h,flag_S,flag_r,flag_wildcard,file);
			}
		}
	}else{
		manager(dir_name,flag_a,flag_l,flag_h,flag_S,flag_r,flag_wildcard,file);
	}
	fprintf(file,"</body>\n");
	fprintf(file,"</html>\n");
	if(dirp!=NULL){
		closedir(dirp);
	}
	fclose(file);
	return 0;
}