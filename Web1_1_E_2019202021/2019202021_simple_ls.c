//////////////////////////////////////////////////////////////////////////
// File Name	: 2019202021_simple_ls.c				//
// Date		: 2023/3/29						//
// OS		: Ubuntu 16.04.5 LTS 64bits				//
// Author	: Jung Sung Yeob					//
// Student Id	: 2019202021						//
// ---------------------------------------------------------------------//
// Title : System Programming Assignment #1-1 (Simple ls)		//
// Description : Coding linux's 'ls' command in c language.		//
//		 Sort case-sensitive					/
// 		 Exception:						//
//		 	1) If enter a file instead of a directory	//
//			2) If enter a directory that does not exist	//
//			3) If enter more than one file path		//
//////////////////////////////////////////////////////////////////////////


#include <stdio.h> //standard input output library
#include <dirent.h> //directory library
#include <unistd.h> //C complier headerfile in UNIX
#include <stdlib.h> //standard utility function headerfile
#include <string.h> //string library


//////////////////////////////////////////////////////////////////////////
// Ascending_sort case-sensitive					//
// =====================================================================//
// Input : 	char *arr[255] 	: directory content list		//
// 		int size	: count of directory content list	//
// output: 								//
//									//
// Purpose: Ascending_sort directory content list			//
//////////////////////////////////////////////////////////////////////////
void ascending_sort(char *arr[255], int size){
	int i, j;
	char tmp[255];
	for(i = 0; i<size-1;i++){
		for(j=i+1;j<size;j++){
			if(strcasecmp(arr[i],arr[j]) >0){
				strcpy(tmp,arr[i]);
				strcpy(arr[i], arr[j]);
				strcpy(arr[j], tmp);
			}
		}
	}
}

int main(int argc, char* argv[])
{
	char *workDIR = (char*)malloc(sizeof(char)*1024); //Dynamic allocation of 1024 bytes to workDIR
	DIR *dirp=NULL; //directory pointer = NULL
	struct dirent *dir=NULL;//dirent struct pointer = NULL

	//if there is no argument(current working directory)
	if(argc==1){
		getcwd(workDIR,1024); //get current working directory to workDIR
		//if cannot open workDIR
		if((dirp=opendir(workDIR))==NULL){
			printf("simple_ls: cannot access current directory \n"); //print error message
			exit(1); // exit execute file
		}
	}
	//if there is one argument
	else if(argc==2){
		//if enter a directory that does not exist or enter a file instead of directory
		if((dirp=opendir(argv[1]))==NULL){
			printf("simple_ls: cannot access \'%s\' : No such directory \n", argv[1]);//print error message
			exit(1); // exit execute file
		}
		
	}
	//if enter more than one file path
	else if(argc>2){
		printf("sipmle_ls: only one directory path can be processed \n"); //print error message
		exit(1); //exit execute file
	}

	char *list[255]; //file name list(Maximum = 255byte)
	int size=0;

	while((dir = readdir(dirp))!=NULL){
		char*tmp = dir->d_name; // tmp = each dir->d_name
		//if hidden file
		if(tmp[0]=='.'){
			continue;
		}
		list[size++]=tmp; //save
	}
	//ascending_sort case-sensitive
	ascending_sort(list, size);

	//print list
	for(int j =0; j<size;j++){
		printf("%s\n",list[j]);
	}
	
	free(workDIR);//free sorkDIR
	closedir(dirp);//closedir dirp
	return 0;


}
