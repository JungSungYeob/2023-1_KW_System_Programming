//////////////////////////////////////////////////////////////////////////
// File Name   : 2019202021_advanced_ls.c				//
// Date        : 2023/4/5						//
// OS		: Ubuntu 16.04.5 LTS 64bits				//
// Author	: Jung Sung Yeob					//
// Student Id	: 2019202021						//
// ---------------------------------------------------------------------//
// Title : System Programming Assignment #1-2 (Advanced ls)		//
// Description : Coding linux's 'ls' command in c language with option.	//
//		        +option implementation(-a -l -al)		//
//              +print directory path when option -l			//
//              +print the number of 1k blocks when option -l		//
//              							//
//              ascending order all of file(include hidden file)	//
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

//////////////////////////////////////////////////////////////////////////
// list_dir                     					//
// =====================================================================//
// Input:   char *dir_name : directory name				//
//          int show_all : -a option					//
//          int show_long: -l option					//
// Ouput : X    							//
// Purpose: for print list of directory of file by option 		//
//////////////////////////////////////////////////////////////////////////

void list_dir(char *dir_name, int show_all, int show_long) {
    DIR *dirp; //Directory pointer dirp
    struct dirent *content; //struct dirent pointer content
    struct stat file_stat; //struct stat file_stat
    struct passwd *pw; // struct passwd pointer pw to get uid
    struct group *gr; //struct group pointer gr to get gid
    char path[PATH_MAX]; //size of path = maximum of path
    char time_str[80]; //set time _str
    struct tm *time_tm;//struct tm pointer to convert time
    struct file_info{   //struct file_info to save name and stat
        char name[256];
        struct stat st;
    };

    if((dirp = opendir(dir_name)) == NULL) {
        printf("advanced_ls: cannot access \'%s\' : No such directory \n", dir_name);//print error message
        exit(1);
    }
    long total_size = 0; //directory total size
    char cwd[PATH_MAX]; //current directory
    if(dir_name!="."){ //if argv is exist
        chdir(dir_name); //change dir to dir_name
    }
    getcwd(cwd,sizeof(cwd)); //get current working directory
    if(show_long){
        printf("Directory path: %s\n",cwd); //print
    }
    
    getcwd(cwd,sizeof(cwd));
    while((content=readdir(dirp))!=NULL){ //read until dirp is null
        if (content->d_name[0]=='.' && !show_all){ //if not -a option and hidden file
            continue;
        }
        if(stat(content->d_name,&file_stat)==0){ //stat name and file stat
            total_size+= (file_stat.st_blocks+1)/2; // sum all of stat blocks+1 and devide 2
        }
    }
    if(show_long){
        printf("total: %ld\n",(long)(total_size)); //print total
    }
    rewinddir(dirp); //rewind dir to reread

    struct file_info files[1024]; //struct file info
    int num_cnt=0; //count
    while((content = readdir(dirp))!=NULL){//read until dirp is null
        if(show_all||content->d_name[0]!='.'){//if -a option or hidden file
        if(stat(content->d_name,&files[num_cnt].st)==0){ //stat name and files[num_cnt].st
            strcpy(files[num_cnt].name, content->d_name);//copy name to files
            num_cnt++;
        }
        }
    }
    
    //bubble sort
    for (int i = 0; i < num_cnt-1; i++) {
        for (int j = 0; j < num_cnt-i-1; j++) {
            char cmp_tmp1[255];//tmp value
            strcpy(cmp_tmp1,files[j].name);
            if(cmp_tmp1[0]=='.'){//if hidden file
                for(int k=0;cmp_tmp1[k]!='\0';k++){ //shift left
                    cmp_tmp1[k]=cmp_tmp1[k+1];
                }
            }
            char cmp_tmp2[255];//tmp value
            strcpy(cmp_tmp2,files[j+1].name);
            if(cmp_tmp2[0]=='.'){ //if hidden file
                for(int k=0;cmp_tmp2[k]!='\0';k++){//shift left
                    cmp_tmp2[k]=cmp_tmp2[k+1];
                }
            }
            if (strcasecmp(cmp_tmp1, cmp_tmp2) > 0) {
                struct file_info temp = files[j];
                files[j] = files[j+1];
                files[j+1] = temp;
            }
        }
    }
    
    if(show_long){//if -l option
    int cnt=0;
    for(int i=0;i<num_cnt;i++){
        struct stat st = files[i].st; //struct stat st to files[i].st
        printf((S_ISDIR(st.st_mode)) ? "d" : "-");//if dir print d else -
        //permission of directory or files
            printf((st.st_mode & S_IRUSR) ? "r" : "-");
            printf((st.st_mode & S_IWUSR) ? "w" : "-");
            printf((st.st_mode & S_IXUSR) ? "x" : "-");
            printf((st.st_mode & S_IRGRP) ? "r" : "-");
            printf((st.st_mode & S_IWGRP) ? "w" : "-");
            printf((st.st_mode & S_IXGRP) ? "x" : "-");
            printf((st.st_mode & S_IROTH) ? "r" : "-");
            printf((st.st_mode & S_IWOTH) ? "w" : "-");
            printf((st.st_mode & S_IXOTH) ? "x" : "-");
            printf("\t%lu \t", st.st_nlink);
            if ((pw = getpwuid(st.st_uid)) != NULL) {//get uid or pw_name
                printf("%s\t", pw->pw_name);
            } else {
                printf("%d\t", st.st_uid);
            }
            if ((gr = getgrgid(st.st_gid)) != NULL) {//get gid or gr_name
                printf("%s\t", gr->gr_name);
            } else {
                printf("%d\t", st.st_gid);
            }
            printf("%lu\t", st.st_size); //get size of dir or file
            time_tm = localtime(&st.st_mtime);//set time_tm to localtime
            strftime(time_str, sizeof(time_str), "%b %d %H:%M", time_tm);
            printf("%s\t", time_str);//print time
            printf("%s\n", files[i].name);//print name
        }
    }else{ // if no option -l
        for(int i=0;i<num_cnt;i++){
            printf("%s\n",files[i].name);
        }
    }
    if(dir_name!="."){//change dir
        chdir("..");
    }
        closedir(dirp);//close dir
        free(content);//free content
        
    }

    



int main(int argc, char *argv[]) {
    int opt; //option num
    int show_all = 0; //-a option
    int show_long = 0; //-l option
    char *dir_name = "."; //current directory

    while ((opt = getopt(argc, argv, "al")) != -1) { //get option
        switch (opt) {
            case 'a':
                show_all = 1; // if -a show_all =1
                break;
            case 'l':
                show_long = 1;// if -l show_long =1
                break;
            case '?': //else exit
                printf("Unkown option character\n");
                exit(1); //
        }
    }

    if (optind < argc) { //if optind < argc  => not only option
        for (int i=optind; i< argc; i++){ //print each argv
            dir_name = argv[i];
            list_dir(dir_name, show_all, show_long);
        }
    }else{ //only option
        list_dir(dir_name, show_all, show_long);
    }

    

    return 0;

    
}
