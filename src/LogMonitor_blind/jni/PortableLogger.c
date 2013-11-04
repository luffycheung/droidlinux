#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "PortableLogger.h"

char target[64][256];

int target_num = 0;

void add_targets(char *conf_file){
  // read targets here
  //strcpy(target[0], "com.example.android.apis");
  FILE *conf = fopen(conf_file, "r");
  int i = 0;
  while(!feof(conf) && i < 64){
	char line[256];
	fgets(line, sizeof(line), conf);
	if(line[0] == '#') continue;
	if(line[0] == '\n') continue;
	printf("%s, %d, %d\n", line, i, strlen(line));
	strncpy(target[i], line, strlen(line)-1);
	//printf("%s", target[i]);
	i ++;
  }
  target_num = i;
  return;  
}

int find_target(char *package_name){
  int i = 0;
  for(; i < target_num; i ++){
	if(strncmp(package_name, target[i], strlen(package_name)) == 0){
	  
	  return 1;
	}
  }
  return 0;

}

void start_strace(int pid){
  if(1){
	char command[512];
	int trace_pid;
	sprintf(command, "/system/xbin/strace -ttt -f -p %d -o /sdcard/trace.%d", pid, pid);
	int c_pid = fork();
	if(c_pid == 0){
	  printf("in child process\n %s:%d\n", command, getpid());
	  printf("trace started in child\n");
	  system(command);
	  printf("child exit\n");
	  exit(0);
	}else{
	  printf("trace child process %d\n", c_pid);
	  printf("continue in parent process\n");
	}
  }
  return;
}

int attach(){
  // scan through proc fs
  DIR *dirp;
  struct dirent entry;
  struct dirent *endp;
  
  if((dirp = opendir("/proc/")) == NULL)
	return -1;

  for(;;){
	endp = NULL;
	if(readdir_r(dirp, &entry, &endp) == -1){
	  closedir(dirp);
	  return -1;
	}
	if(endp == NULL){
	  break;
	}

	int pid;
	if(sscanf(entry.d_name, "%d", &pid) == 1){
	  //printf("%s, %d\n", entry.d_name, pid);	  
	}else{
	  continue;
	}
	
	// this is the cmdline 
	char file_name[256];
	snprintf(file_name, sizeof(file_name), "/proc/%d/cmdline", pid);
	//printf("%s\n", file_name);
	
	FILE *cmdline = fopen(file_name, "r");
	if(cmdline != NULL){
	  char package_name[512];
	  fgets(package_name, sizeof(package_name), cmdline);
	  printf("searching %s\n", package_name);
	  if(find_target(package_name)){
		printf("found %s\n", package_name);
		start_strace(pid);
	  }
	}
  }
}

int main(int argc, char *argv[]){
  char conf_file[128];
  
  int status = 0;
  
  if(argc <= 1){
	strcpy(conf_file, "targets.txt");
  }else{
	printf("%s, %s\n", argv[0], argv[1]);
	strcpy(conf_file, argv[1]);
  }
  add_targets(conf_file);
  attach();
  printf("%s, %d\n", target[0], strlen(target[0]));

  while ((wpid = wait(&status)) > 0)
  {
	printf("Exit status of %d was %d (%s)\n", (int)wpid, status,
			 (status > 0) ? "accept" : "reject");
  }
}
  
