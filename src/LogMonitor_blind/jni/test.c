#include <stdio.h>


int main(){
  char a[] = {"edu.seclab.eventinjector"};
  char b[] = {"edu.seclab"};
  printf("size of b %d\n", (int)sizeof(b));

  int equal = strncmp(a, b, sizeof(b)-1);
   
  printf("euqal %d\n", equal);

  if(strncmp(a, b, sizeof(b)-1) == 0){
	printf("hello");
  }
}
