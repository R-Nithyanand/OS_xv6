// whoami.c -- print current username based on uid

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define MAX 128

// parse one /users line: user:pass:uid:home
int
parse_line(char *line, char *u, char *p, int *uid, char *home)
{
  int i = 0, j = 0, field = 0;
  char tmp[MAX];

  for(i = 0; line[i]; i++){
    if(line[i] == ':'){
      tmp[j] = 0;

      if(field == 0) strcpy(u, tmp);
      else if(field == 1) strcpy(p, tmp);
      else if(field == 2) *uid = atoi(tmp);

      j = 0;
      field++;
      continue;
    }

    tmp[j++] = line[i];
  }

  // last field (home)
  tmp[j] = 0;
  strcpy(home, tmp);

  // need at least user, pass, uid
  return field >= 3;
}

int
main(void)
{
  int fd = open("/users", O_RDONLY);
  if(fd < 0){
    printf(2, "whoami: cannot open /users\n");
    exit();
  }

  int myuid = getuid();
  char buf[MAX];
  char u[64], p[64], home[64];
  int uid;

  int n, pos = 0;
  while((n = read(fd, buf + pos, 1)) == 1){
    if(buf[pos] == '\n'){
      buf[pos] = 0;

      if(parse_line(buf, u, p, &uid, home)){
        if(uid == myuid){
          printf(1, "%s\n", u);
          close(fd);
          exit();
        }
      }

      pos = 0;
    } else {
      pos++;
      if(pos >= MAX-1) pos = MAX-2;
    }
  }

  close(fd);

  printf(1, "unknown\n");
  exit();
}

