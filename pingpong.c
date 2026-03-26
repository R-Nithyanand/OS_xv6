#include"types.h"
#include"user.h"

int main(){
int p1[2],p2[2];
char byte='x';
int exchanges=1000; //# pingpongs

if((pipe(p1)<0) | (pipe(p2)<0)){
printf(2, "Pipe creation failed.");
exit();
}

int pid=fork();
if(pid<0){
printf(2,"Fork failed");
exit();}

if (pid==0){
close(p1[1]);
close(p2[0]);
for (int i=0;i<exchanges;i++){
read(p1[0],&byte,1);
write(p2[1],&byte,1);
}
exit();
}

else
{
close(p1[0]);
close(p2[1]);
int start=uptime();
for(int i=0;i<exchanges;i++){
read(p2[0],&byte,1);
write(p1[1],&byte,1);
}
int end=uptime();
int duration = end - start;  // ticks taken
printf(1,"performed the pingpong in %d exchanges successfully in %d ticks",exchanges,duration);
exit();
}
}
