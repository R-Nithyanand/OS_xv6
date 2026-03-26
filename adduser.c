// adduser.c -- secure useradd for xv6 with password hashing
// usage: adduser username password uid
// Only root can add users

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define LINE_MAX 128

// Simple DJB2 hash (same as hash.c)
unsigned int
djb2_hash(const char *str)
{
  unsigned int hash = 5381;
  int c;
  
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c;
  
  return hash;
}

// Convert hash to hex string
void
hash_to_hex(unsigned int hash, char *out)
{
  const char *hex = "0123456789abcdef";
  int i;
  
  for(i = 7; i >= 0; i--) {
    out[i] = hex[hash & 0xF];
    hash >>= 4;
  }
  out[8] = '\0';
}

// Hash a password
void
hash_password(const char *password, char *hash_out)
{
  unsigned int hash = djb2_hash(password);
  hash_to_hex(hash, hash_out);
}

int
main(int argc, char *argv[])
{
  // Security check: only root can add users
  if(getuid() != 0){
    printf(2, "adduser: permission denied (must be root)\n");
    exit();
  }

  if(argc < 4){
    printf(2, "usage: adduser username password uid\n");
    exit();
  }

  // Hash the password
  char hashed[9];
  hash_password(argv[2], hashed);

  // open /users for read+write, create if missing
  int fd = open("/users", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(2, "adduser: cannot open /users\n");
    exit();
  }

  // read until EOF to move file offset to end (so write will append)
  char tmp;
  while(read(fd, &tmp, 1) == 1)
    ; // discard bytes

  // build line: username:hashedpassword:uid:/\n
  char line[LINE_MAX];
  int p = 0;
  int i;

  // copy username
  for(i = 0; argv[1][i] && p < LINE_MAX-2; i++) line[p++] = argv[1][i];
  if(p >= LINE_MAX-2){ printf(2, "adduser: username too long\n"); close(fd); exit(); }
  line[p++] = ':';

  // copy hashed password
  for(i = 0; hashed[i] && p < LINE_MAX-2; i++) line[p++] = hashed[i];
  if(p >= LINE_MAX-2){ printf(2, "adduser: hash too long\n"); close(fd); exit(); }
  line[p++] = ':';

  // copy uid string
  for(i = 0; argv[3][i] && p < LINE_MAX-2; i++) line[p++] = argv[3][i];
  if(p >= LINE_MAX-2){ printf(2, "adduser: uid too long\n"); close(fd); exit(); }
  line[p++] = ':';

  // home dir default "/"
  if(p < LINE_MAX-2) { line[p++] = '/'; }
  line[p++] = '\n';

  // write it
  if(write(fd, line, p) != p){
    printf(2, "adduser: write failed\n");
    close(fd);
    exit();
  }

  printf(1, "User %s added successfully (password hashed)\n", argv[1]);

  close(fd);
  exit();
}