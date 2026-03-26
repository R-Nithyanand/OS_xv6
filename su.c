// su.c -- switch user in xv6 with password hashing

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define LINE_MAX 256
#define NAME_MAX 64
#define PASS_MAX 64

// Simple DJB2 hash
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

// Verify password
int
verify_password(const char *password, const char *hash_stored)
{
  char computed[9];
  hash_password(password, computed);
  return strcmp(computed, hash_stored) == 0;
}

// small local strncpy replacement
static void
xstrncpy(char *dst, const char *src, int n) {
  int i;
  for (i = 0; i < n-1 && src[i]; i++) dst[i] = src[i];
  dst[i] = '\0';
}

// parse a line of format: username:passwordhash:uid:home
int
parse_line(char *line, char *out_user, char *out_hash, int *out_uid)
{
  int i = 0, j = 0;
  int len = strlen(line);
  if(len == 0) return 0;

  // username
  j = 0;
  while(i < len && line[i] != ':' && j < NAME_MAX-1) out_user[j++] = line[i++];
  out_user[j] = '\0';
  if(i >= len || line[i] != ':') return 0;
  i++; // skip ':'

  // password hash
  j = 0;
  while(i < len && line[i] != ':' && j < PASS_MAX-1) out_hash[j++] = line[i++];
  out_hash[j] = '\0';
  if(i >= len || line[i] != ':') return 0;
  i++; // skip ':'

  // uid (decimal)
  int uid = 0;
  int found = 0;
  while(i < len && line[i] >= '0' && line[i] <= '9') {
    found = 1;
    uid = uid * 10 + (line[i] - '0');
    i++;
  }
  if(!found) return 0;
  *out_uid = uid;
  return 1;
}

int
main(int argc, char *argv[])
{
  if(argc < 2){
    printf(2, "usage: su username\n");
    exit();
  }

  char target[NAME_MAX];
  xstrncpy(target, argv[1], NAME_MAX);

  // prompt for password
  char passwd[PASS_MAX];
  printf(1, "password: ");
  gets(passwd, PASS_MAX);
  if(strlen(passwd) > 0 && passwd[strlen(passwd)-1] == '\n')
    passwd[strlen(passwd)-1] = '\0';

  // open /users and search for user
  int fd = open("/users", O_RDONLY);
  if(fd < 0){
    printf(2, "su: cannot open /users\n");
    exit();
  }

  char line[LINE_MAX];
  int pos = 0;
  int n;
  int found = 0;
  int uid = -1;

  while((n = read(fd, line + pos, 1)) == 1) {
    if(line[pos] == '\n') {
      line[pos] = '\0';
      pos = 0;

      char u[NAME_MAX], hash[PASS_MAX];
      int parsed_uid;
      if(parse_line(line, u, hash, &parsed_uid)) {
        // Verify username and password hash
        if(strcmp(u, target) == 0 && verify_password(passwd, hash)) {
          found = 1;
          uid = parsed_uid;
          break;
        }
      }
    } else {
      pos++;
      if(pos >= LINE_MAX-1) pos = LINE_MAX-2;
    }
  }

  close(fd);

  if(!found){
    printf(1, "Authentication failed\n");
    exit();
  }

  // set uid for this process and exec a new shell
  if(setuid(uid) < 0){
    printf(2, "su: setuid failed\n");
    exit();
  }

  // exec shell (replace current process)
  char *shargs[] = { "sh", 0 };
  exec("sh", shargs);

  // exec failed
  printf(2, "su: exec sh failed\n");
  exit();
}
