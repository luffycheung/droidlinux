#include <stdio.h>
#include <stdlib.h>

//This is a portable version of strace that simply get the pid of services and 
// attach to them. 
// maxium number of process that we want to trace. 
int target_pid[128];
int n_pid = 0;
int in_parent = 1;

static volatile unsigned int printk_cpu;
// get current time:
unsigned long long t;
unsigned long nanosec_rem;
// change to the package 
// com.example.android.apis
//char test_case[] = {"com.example.android"};
//char test_case[] = {"edu.seclab"};
char test_case[] = {"com.example.android.apis"};

FILE * kmsg_fd = 0;

// we can read all the files from 

void add_pid(int pid){
  int i = 0;
  for(; i < n_pid; i ++){
	if(target_pid[i] == pid)
	  return;
  }
  
  // not found:
  target_pid[n_pid++] = pid;
}

int find_pid(int pid){
  // search for pid: 
  printf("find pid %d\n", pid);
  int i = 0;
  for(; i < n_pid; i ++){
	if(target_pid[i] == pid)
	  return i;
  }

  printf("%d not found \n", pid);
  return -1;
}

void start_strace(int pid){
  if(find_pid(pid) < 0){
	char command[512];
	int trace_pid;
	sprintf(command, "/system/xbin/strace -ttt -f -p %d -o /sdcard/trace.%d", pid, pid);
	// add the process for monitoring
	add_pid(pid);
	int c_pid = fork();
	in_parent = c_pid;
	if(c_pid == 0){
	  printf("in child process\n %s:%d", command, getpid());
	  //fclose(kmsg_fd);
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


int parse_binder_strace(char * str){
  //char test_str[256] = {"<4>[  500.980009] ###edu.seclab.eventinjector 1828 /system/bin/surfaceflinger 160###"};
  //str = test_str;
  //char test_str2[256] = {"###com.example 162 mediaservice 162###"};
  
  int src_pid = -1;
  int dest_pid = -1;
  char src_name[128];
  char dest_name[128];

  if(strstr(str, "###")){
	sscanf(str, "<%*d>[%*d.%*d] ###%s %d %s %d###", src_name, &src_pid, dest_name, &dest_pid);
	//printf("%s, %d, %s, %d\n", src_name, src_pid, dest_name, dest_pid);
	if(strncmp(src_name, test_case, sizeof(test_case)-1) == 0){
	  printf("%s, %d, %s, %d\n", src_name, src_pid, dest_name, dest_pid);
	  printf("match\n");
	  start_strace(dest_pid);
	  start_strace(src_pid);
	}	
  }
  
  /*
  if(strstr(test_str2, "###")){
	sscanf(test_str2, "###%s %d %s %d###", src_name, &src_pid, dest_name, &dest_pid);
	printf("%s, %d, %s, %d\n", src_name, src_pid, dest_name, dest_pid);
	start_strace(dest_pid);
  }
  */

  return 0;
}


int main(){
  // read log files from /proc/kmsg
  /*
  parse_binder_strace(NULL);
  return 0;
  */
  
  char *mode = "r";
  
  kmsg_fd = fopen("/proc/kmsg", mode);
  
  if(kmsg_fd == NULL){
	printf("Can not open /proc/kmsg\n");
	exit(1);
  }
  while(1){
	if(in_parent == 0){
	  break;
	}
	  char line[256];
	  while(fgets(line, sizeof(line), kmsg_fd) != NULL){
		parse_binder_strace(line);
	  }
  }

  fclose(kmsg_fd);

  return 0;
}
