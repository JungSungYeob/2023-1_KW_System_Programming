//////////////////////////////////////////////////////////////////////////
// File Name   : 2019202021_final_ls.c					//
// Date        : 2023/4/12						//
// OS		: Ubuntu 16.04.5 LTS 64bits				//
// Author	: Jung Sung Yeob					//
// Student Id	: 2019202021						//
// ---------------------------------------------------------------------//
// Title : System Programming Assignment #1-3 (final ls)		//
// Description : Coding linux's 'final ls' command in c language	//
//				+Wild card matching('*','?','[seq]')	//
//				+Passing arguments with quotes		//
//				(e.g., ./spls_final '*')		//
//				+implements -h, -r, -S option		//
//				+another options at assignment #1-1	//
// 				and #1-2				//
//////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////
// print_permission                     				//
// =====================================================================//
// Input:   mode_t mode							//
// Ouput : print file's type & permission, 				//
//			no return					//
// Purpose: for print file's type & permission 				//
//////////////////////////////////////////////////////////////////////////
void print_permission(mode_t mode){ //option -l print permission
	printf((S_ISDIR(mode)) ? "d" : "-"); //if dir print "e" else print "-"
	printf((mode & S_IRUSR) ? "r" : "-"); //if have user read permission
    printf((mode & S_IWUSR) ? "w" : "-"); //if have user write permission
	printf((mode & S_IXUSR) ? "x" : "-"); //if have user execute permission
    printf((mode & S_IRGRP) ? "r" : "-"); //if have group read permission
    printf((mode & S_IWGRP) ? "w" : "-"); //if have group write permission
    printf((mode & S_IXGRP) ? "x" : "-"); //if have group execute permission
    printf((mode & S_IROTH) ? "r" : "-"); //if have other read permission
    printf((mode & S_IWOTH) ? "w" : "-"); //if have other write permission
    printf((mode & S_IXOTH) ? "x" : "-"); //if have other execute permission
}

//////////////////////////////////////////////////////////////////////////
// print_UID		                    				//
// =====================================================================//
// Input:   uid_t uid							//
// Ouput : print file's uid						//
//			no return					//
// Purpose: for print file's uid			 		//
//////////////////////////////////////////////////////////////////////////
void print_UID(uid_t uid){ //option -l print UID
	struct passwd *pw; //get struct passwd
	if((pw=getpwuid(uid))!=NULL){ //if get pwuid is not NULL
			printf("%s\t", pw->pw_name); //print pw_name
		}else{
			printf("%d\t",uid); //print uid num
		}
}

//////////////////////////////////////////////////////////////////////////
// print_GID		                    				//
// =====================================================================//
// Input:   gid_t gid							//
// Ouput : print file's gid						//
//			no return					//
// Purpose: for print file's gid			 		//
//////////////////////////////////////////////////////////////////////////
void print_GID(gid_t gid){ //option -l print GID
	struct group *gr; //get struct group
	if((gr=getgrgid(gid))!=NULL){ //if get grgid is not NULL
			printf("%s\t", gr->gr_name); //print gr_name
		}else{
			printf("%d\t",gid); //print gid num
		}
}

//////////////////////////////////////////////////////////////////////////
// print_readableSIZE                    				//
// =====================================================================//
// Input:   size_t size							//
// Ouput : print file's readable size					//
//			no return					//
// Purpose: for print file's readable size	when option -h		//
//////////////////////////////////////////////////////////////////////////
void print_readableSIZE(size_t size){
	long unsigned int readable_size = size; //option -h print size as readable like G, M, K
			if(readable_size/1000000000>0){ //if Giga
				readable_size/=100000000; //divide 10^8
				if(readable_size%10>=5){ //if have to round number
					readable_size+=10; //round num
				}
				readable_size/=10; //divide 10
				printf("%luG\t",readable_size);
			}else if(readable_size/1000000>0){//if Mega
				readable_size/=100000;//divide 10^5
				if(readable_size%10>=5){//if have to round number
					readable_size+=10; //round num
				}
				readable_size/=10; //divide 10
				printf("%luM\t",readable_size);
			}else if(readable_size/1000>0){ //if Kilo
				readable_size/=100; //divide 10^2
				if(readable_size%10>=5){ //if have to round number
					readable_size+=10; //round num
				}
				readable_size/=10; //divide 10
				printf("%luK\t",readable_size); //print readable size
			}else{ //if cannot divide
				printf("%lu\t",readable_size); //print readable size
			}
}

//////////////////////////////////////////////////////////////////////////
// print_LastModified_time                				//
// =====================================================================//
// Input:   time_t time							//
// Ouput : print file's last modified time				//
//			no return					//
// Purpose: for print file's last modified time				//
//////////////////////////////////////////////////////////////////////////
void print_LastModified_time(time_t time){ //option -l print lastmodified time
	char time_str[80]; //array 80
	struct tm *time_tm; //get struct tm to pointer time_tm
	time_tm = localtime(&time); //get localtime
		strftime(time_str,sizeof(time_str),"%b %d %H:%M",time_tm);//strftime as format
		printf("%s\t",time_str); //print time
}

//////////////////////////////////////////////////////////////////////////
// print_long			                 			//
// =====================================================================//
// Input:   char* dir_name, int show_size				//
// Ouput : print file's info by long format				//
//			no return					//
// Purpose: for print file's info by long format			//
//			if -h option print readable size		//
//////////////////////////////////////////////////////////////////////////
void print_long(struct stat file_stat,char*name, int show_size){ //option -l print long format
	print_permission(file_stat.st_mode); //print permission
	printf("\t%lu\t",file_stat.st_nlink); //print nlink num
	print_UID(file_stat.st_uid); //print UID
	print_GID(file_stat.st_gid); //print GID
	if(show_size==1){ //if -h
		print_readableSIZE(file_stat.st_size); //print readable size
	}else{
		printf("%lu\t",file_stat.st_size); //print size
	}
	print_LastModified_time(file_stat.st_mtime); //print last modified time
	printf("%s\n", name);	//print file name
}

//////////////////////////////////////////////////////////////////////////
// file_print			                  			//
// =====================================================================//
// Input:   char*dir_name, int show_long, int show_size			//
// Ouput : print file info						//
//			no return					//
// Purpose: for print file's info			 		//
//			if -l option has activated,			//
//			use function print_long				//
//			else print file name				//
//////////////////////////////////////////////////////////////////////////
void file_print(char *dir_name, int show_long, int show_size){ //if argument is file
	struct stat file_stat; // get struct stat
	if(show_long==0){ //if not -l option
		printf("%s\n",dir_name); //print file name
		return;
	}
	if(show_long==1){ //if -l option
		char *cwdOK; //get cwd Ok sign
		char cwd[PATH_MAX]; //get cwd include filename
		char *realcwd; //get cwd except filename
		char abs_path[PATH_MAX]; //absolute path
		cwdOK=realpath(dir_name,cwd);//get cwd
		if(cwdOK==NULL){ // if error
			printf("get realpath error"); //get realpath error
			return; // end function
		}
		realcwd=dirname(cwd); //except file name
		realpath(realcwd,abs_path); //get absolute path
		printf("Directory path: %s\n",abs_path); //print directory path
		if(stat(dir_name,&file_stat)==0){ //stat to file_stat about dir_name
			printf("total: %ld\n",(long)((file_stat.st_blocks+1)/2)); //print total block = file block
		}
		print_long(file_stat,dir_name,show_size); //print file name by long format
		return;
	}
}

//////////////////////////////////////////////////////////////////////////
// dir_print			                  			//
// =====================================================================//
// Input:   char*dir_name, int show_all, int show_long, int show_size	//
//			 int sort_size, int show_rev			//
// Ouput : print dir's files info					//
//			no return					//
// Purpose: for print dir's files info			 		//
//			if -a option has activated,	print all files	//
//			if -l option has activated, use function	//
//			print_all//					//
//			if -h option has activated, show readable size	//
//			if -S option has activated, sort by size	//
//			 descending order				//
//			if -r option has activated, reverse order	//
//			else print file name				//
//////////////////////////////////////////////////////////////////////////
void dir_print(char*dir_name, int show_all, int show_long, int show_size, int sort_size, int show_rev){ //if argument is path
	DIR *dirp; //DIR pointer dirp
	struct dirent *dir; //get struct dirent to pointer dir
	struct stat file_stat; //get strcut stat
	struct file_info{//set struct file_info
		char name[255]; //file name
		struct stat st; //get struct stat
	};

	
	if((dirp = opendir(dir_name))==NULL){ //if opendir about dir_name is NULL
		printf("spls_final: Opendir Error"); //print error
		exit(1);//exit program
	}

	struct file_info files[1024]; //struct file_info to array 1024
	int num_cnt=0;//file count
	int total_size; // total size
	while((dir = readdir(dirp))!=NULL){ //read each content
		if(show_all||dir->d_name[0]!='.'){ //if -a option has actived or hidden file
			char tmppath[PATH_MAX]; //set tmppath to get stat
			snprintf(tmppath,PATH_MAX,"%s/%s",dir_name,dir->d_name);//make path dir_name/dir->d_name
			if(stat(tmppath,&files[num_cnt].st)==0){//get stat about tmppath
				strcpy(files[num_cnt].name, dir->d_name); //get name
				total_size+=(files[num_cnt].st.st_blocks+1)/2; //sum size to total size
				num_cnt++; //up count
			}
		}
	}

	//basically bubble sort
	if(sort_size==0){ // if -S option has not activated
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

	if (show_long==0){ //if -l option has not activated
		if(show_rev==0){ //if -r option has not activated
			for(int i=0;i<num_cnt;i++){
				printf("%s\n",files[i].name); //print files[i].name
			}
		}else{ //if -r option has activated
			for(int i=num_cnt-1;i>=0;i--){ //reverse
				printf("%s\n",files[i].name); //print files[i].name
			}
		}
	}else{ //if -l option has activated
		char *cwdOK; //get cwd ok
		char cwd[PATH_MAX]; // get cwd
		cwdOK=realpath(dir_name,cwd);//get realpath include last dir
		if(cwdOK==NULL){
			printf("get realpath error"); //print realpath error
			return;
		}
		printf("Directory path: %s\n",cwd); //print directory path
		printf("total: %ld\n",(long)(total_size)); //print total size
		if(show_rev==0){//if -r option has not activated
			for(int i=0;i<num_cnt;i++){ //print file list by long format
				struct stat file_stat = files[i].st;
				print_long(file_stat,files[i].name,show_size);
			}
		}else{//if -r option has activated
			for(int i=num_cnt-1;i>=0;i--){//print file list by long format reverse
				struct stat file_stat = files[i].st;
				print_long(file_stat,files[i].name,show_size);
			}
		}
	}
	closedir(dirp); //close dirp;
	free(dir);

}

//////////////////////////////////////////////////////////////////////////
// wildcard_print			              			//
// =====================================================================//
// Input: char* dir_name, int show_all, int show_long, int show_size,	//
//				int sort_size, int show_rev		//
// Ouput : print file's size						//
//			no return					//
// Purpose: for print wildcard format		 			//
//			don't need to consider about option!!		//
//////////////////////////////////////////////////////////////////////////
void wildcard_print(char* dir_name,int isAsterisk, int isSequence, int isQuestion){
	struct stat file_stat;
	struct file_info{//set struct file_info
		char name[255]; //file name
		struct stat st; //get struct stat
	};
	DIR *dirp; // DIR pointer dirp
	struct dirent *dir; //get struct dirent pointer dir
	int lastpath=-1; //lastpath location
	char path[PATH_MAX]; //path
	char pattern[PATH_MAX]; //pattern
	for(int i=0;i<strlen(dir_name)-1;i++){ //ignore and last '/'
		if(dir_name[i]=='/'){
			lastpath=i;//get lastpath
		}
	}

	if(lastpath==-1){
		strcpy(path,".");//get current dir
		strcpy(pattern,dir_name);//get dir_name as pattern
	}
	else{
		strncpy(path,dir_name,lastpath); //get path
		for(int i=lastpath;i<strlen(dir_name)-1;i++){//get pattern
			pattern[i-lastpath]=dir_name[i+1];
		}
	}


	if((dirp=opendir(path))==NULL){//opendir path
		printf("spls_final: cannot access \'%s\' : No such directory \n", path);
		exit(1);
	}
	struct file_info files[1024]; //struct file_info to array 1024 
	int num_cnt=0; //count
	while((dir=readdir(dirp))!=NULL){//readir(dirp)
		if(dir->d_name[0]=='.'&&pattern[0]!='.'){//if hidden file when pattern[0] is not '.'
			continue;
		}else if(pattern[0]=='.'&&dir->d_name[0]!='.'){//if pattern[0] is '.' when file[0] is not '.'
			continue;
		}
		if(strcmp(dir->d_name,".")==0||strcmp(dir->d_name,"..")==0){ //except current dir and previous dir
			continue;
		}
		if (fnmatch(pattern,dir->d_name,0)==0){ //if find pattern
			char tmppath[PATH_MAX]; //set tmppath to get stat
			snprintf(tmppath,PATH_MAX,"%s/%s",path,dir->d_name);//make path dir_name/dir->d_name
			if(stat(tmppath,&files[num_cnt].st)==0){
				strcpy(files[num_cnt].name, dir->d_name); //get name
				num_cnt++; //up count
			}
		}
	}
	
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

	for(int i=0;i<num_cnt;i++){
		if(S_ISDIR(files[i].st.st_mode)!=0){//if directory
			char tmppath[PATH_MAX]; //set tmppath to get stat
			snprintf(tmppath,PATH_MAX,"%s/%s",path,files[i].name);//make path dir_name/dir->d_name
			char abs_path[PATH_MAX];
			realpath(tmppath,abs_path);
			printf("Directory path: %s\n",abs_path);
			dir_print(abs_path,0,0,0,0,0);
			printf("\n");
		}else{
			printf("%s\n",files[i].name);
		}
	}

	closedir(dirp);
}


//////////////////////////////////////////////////////////////////////////
// list_dir				                   		//
// =====================================================================//
// Input: char* dir_name, int show_all, int show_long, int show_size,	//
//				int sort_size, int show_rev		//
// Ouput : print file's size						//
//			no return					//
// Purpose: for print file's size			 		//
//////////////////////////////////////////////////////////////////////////

void list_dir(char *dir_name, int show_all, int show_long, int show_size, int sort_size, int show_rev, int isWildcard){ //manager about print
	DIR *dirp; //DIR *dirp
	struct dirent *tmp; //get struct dirent *tmp
	int file; //file open element
	int isFile=0; //is file?
	int isAsterisk=0; //if *
	int isSequence=0; //if []
	int isQuestion=0; //if ?

	for(int i=0;i<strlen(dir_name);i++){
		if(dir_name[i]=='*'){//get *
			isAsterisk=1;//set
		}else if(dir_name[i]=='['){//get [
			isSequence=1;//set
		}else if(dir_name[i]=='?'){//get ?
			isQuestion=1;//set
		}
	}
	if(isAsterisk||isSequence||isQuestion){//if wildcard
		wildcard_print(dir_name,isAsterisk,isSequence,isQuestion);//function wildcard_print
		return;
	}

	
	//detect opendir or open error
	if((dirp=opendir(dir_name))==NULL){//not dir
		if((file=open(dir_name,O_RDONLY))==-1){//not file
			printf("spls_final: cannot access \'%s\' : No such directory \n", dir_name);
			exit(1);
		}else{//it's file!
			isFile=1;//set
		}
	}
	closedir(dirp);//close dirp
	close(file);//close file
	if(isFile==1){//if file
		file_print(dir_name,show_long,show_size);//just need -l option or -h option
		return;
	}
	else{//if path
		if(isWildcard){
			char abs_path[PATH_MAX];
			realpath(dir_name,abs_path);
			printf("Directory path: %s\n",abs_path);
		}
		dir_print(dir_name,show_all,show_long,show_size,sort_size,show_rev); //need all option
		return;
	}

	
}


int main(int argc, char *argv[]){
	DIR *dirp;
	struct dirent *dir;
	int opt; //option num
	int show_all=0; // -a option
	int show_long=0; //-l optoin
	int show_size=0; //-h option
	int sort_size=0; //-S option
	int show_rev =0; //-r option
	char *dir_name = "."; //current dir

	while((opt=getopt(argc,argv,"alhSr"))!=-1){ //get option 
		switch (opt)
		{
		case 'a':// -a option
			show_all =1; //set
			break;
		case 'l': //-l option
			show_long =1; //set
			break;
		case 'h': //-h option
			show_size=1; //set
			break;
		case 'S': //-S option
			sort_size=1; //set
			break;
		case 'r': //-r option
			show_rev =1; //set
			break;
		case '?': //another option
			printf("Unknow option character\n"); //print error
			exit(1); //exit program
		}
	}
	
	//if wildcard is there another argv have print it's direction
	int isWildcard=0; //if wildcard exist
	char abs_path[PATH_MAX]; //get abs_path
	for(int i=optind; i<argc; i++){ //find wildcard
		for(int j=0;j<strlen(argv[i]);j++){
			if(argv[i][j]=='*'){
				isWildcard=1;
			}else if(argv[i][j]=='['){
				isWildcard=1;
			}else if(argv[i][j]=='?'){
				isWildcard=1;
			}
		}
	}

	if(sort_size==0){
		for(int i=optind; i<argc-1; i++){
			for(int j=optind;j<argc-i;j++){
				char cmp_tmp1[255];//tmp value
        		strcpy(cmp_tmp1,argv[j]); //get name value to cmp_tmp1
        		if(cmp_tmp1[0]=='.'){//if hidden file
            		for(int k=0;cmp_tmp1[k]!='\0';k++){ //shift left
                		cmp_tmp1[k]=cmp_tmp1[k+1]; 
           			}
       			}
				char cmp_tmp2[255];//tmp value
	        	strcpy(cmp_tmp2,argv[j+1]); //get name value to cmp_tmp1
    	    	if(cmp_tmp2[0]=='.'){//if hidden file
        	    	for(int k=0;cmp_tmp2[k]!='\0';k++){ //shift left
            	    	cmp_tmp2[k]=cmp_tmp2[k+1]; 
           			}
       			}

				if(strcasecmp(cmp_tmp1,cmp_tmp2)>0){ //if cmp_tmp1>cmp_tmp2 swap location
					char temp[PATH_MAX]; //temp
					strcpy(temp,argv[j]);
					strcpy(argv[j],argv[j+1]);
					strcpy(argv[j+1],temp);
				}
			}
		}
	}else if(sort_size==1){
		for(int i=optind;i<argc-1;i++){
			for(int j=optind;j<argc-i-1;j++){
				struct stat file_stat1;
				struct stat file_stat2;
				stat(argv[j],&file_stat1);
				stat(argv[j+1],&file_stat2);
				long unsigned int tmp1=file_stat1.st_size;
				long unsigned int tmp2=file_stat2.st_size;
				printf("%lu , %lu\n\n",tmp1,tmp2);
				if(tmp1<tmp2){
					char temp[PATH_MAX];
					strcpy(temp,argv[j]);
					strcpy(argv[j],argv[j+1]);
					strcpy(argv[j+1],temp);
				}else if(tmp1==tmp2){
					char cmp_tmp1[255];//tmp value
        			strcpy(cmp_tmp1,argv[j]); //get name value to cmp_tmp1
        			if(cmp_tmp1[0]=='.'){//if hidden file
            			for(int k=0;cmp_tmp1[k]!='\0';k++){ //shift left
            	    		cmp_tmp1[k]=cmp_tmp1[k+1]; 
           				}
       				}
					char cmp_tmp2[255];//tmp value
        			strcpy(cmp_tmp2,argv[j+1]); //get name value to cmp_tmp1
        			if(cmp_tmp2[0]=='.'){//if hidden file
            			for(int k=0;cmp_tmp2[k]!='\0';k++){ //shift left
                			cmp_tmp2[k]=cmp_tmp2[k+1]; 
           				}
       				}

					if(strcasecmp(cmp_tmp1,cmp_tmp2)<0){ //if cmp_tmp1>cmp_tmp2 swap location
						char temp[PATH_MAX]; //temp
						strcpy(temp,argv[j]);
						strcpy(argv[j],argv[j+1]);
						strcpy(argv[j+1],temp);
					}
				}
			}
		}
	}
	if(optind<argc){ //if there is path or filename
		if(show_rev){
			for(int i=argc-1;i>=optind;i--){
				dir_name = argv[i]; //get filename or path
				list_dir(dir_name,show_all,show_long,show_size,sort_size,show_rev,isWildcard); //use list_dir function to print
				
			}
		}else{
			for(int i=optind; i<argc; i++){ //get argument
				dir_name = argv[i]; //get filename or path
				list_dir(dir_name,show_all,show_long,show_size,sort_size,show_rev,isWildcard); //use list_dir function to print
				
			}
		}
	}else{ //if there is no path or filename
		list_dir(dir_name,show_all,show_long,show_size,sort_size,show_rev,isWildcard); //use list_dir function to print
	}

	return 0;
}

