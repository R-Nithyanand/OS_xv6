//If the user forgets to pass an argument, sleep should print an error message.
//See kernel/sysproc.c for the xv6 kernel code that implements the sleep system call (look for sys_sleep), user/user.h for the C definition of sleep callable from a user program, and user/usys.S for the assembler code that jumps from user code into the kernel for sleep.
//sleep's main should call exit(0) when it is done.
#include "types.h"
#include "user.h"

int main(char argc, char *argv[]){
if (argc!=2){
printf(2,"USage: sleep <ticks>");
exit();
}
int ticks=atoi(argv[1]);
if (ticks<=0){
printf(2, "Error enter positive number of ticks");
exit();
}
sleep(ticks);

exit();
}
