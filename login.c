// user/login.c  -- secure login program for xv6 with password hashing

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define LINE_MAX 256
#define NAME_MAX 64
#define PASS_MAX 64

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

// Verify password against stored hash
int
verify_password(const char *password, const char *hash_stored)
{
  char computed[9];
  hash_password(password, computed);
  return strcmp(computed, hash_stored) == 0;
}

// parse a line of format: username:passwordhash:uid:home
// returns 1 on success, 0 on failure
int
parse_line(char *line, char *out_user, char *out_hash, int *out_uid)
{
  int i = 0, j = 0;
  int len = strlen(line);
  if(len == 0) return 0;

  // parse username (until ':')
  while(i < len && line[i] != ':' && j < NAME_MAX-1) {
    out_user[j++] = line[i++];
  }
  out_user[j] = '\0';
  if(i >= len || line[i] != ':') return 0;
  i++; // skip ':'

  // parse password hash (8 hex chars)
  j = 0;
  while(i < len && line[i] != ':' && j < PASS_MAX-1) {
    out_hash[j++] = line[i++];
  }
  out_hash[j] = '\0';
  if(i >= len || line[i] != ':') return 0;
  i++; // skip ':'

  // parse uid (decimal) until ':' or end
  int uid = 0;
  int found_digit = 0;
  while(i < len && line[i] >= '0' && line[i] <= '9') {
    found_digit = 1;
    uid = uid * 10 + (line[i] - '0');
    i++;
  }
  if(!found_digit) return 0;

  *out_uid = uid;
  return 1;
}

int
main(int argc, char *argv[])
{
  char username[NAME_MAX], password[PASS_MAX];
  char linebuf[LINE_MAX];
  int fd;

  for(;;) {
    // prompt
    printf(1, "login: ");
    gets(username, NAME_MAX);
    // strip trailing newline if present
    if(strlen(username) > 0 && username[strlen(username)-1] == '\n')
      username[strlen(username)-1] = '\0';

    printf(1, "password: ");
    gets(password, PASS_MAX);
    if(strlen(password) > 0 && password[strlen(password)-1] == '\n')
      password[strlen(password)-1] = '\0';

    // open users file fresh for this attempt (rewind)
    fd = open("/users", O_RDONLY);
    if(fd < 0) {
      printf(1, "login: cannot open /users\n");
      exit();
    }

    int ok = 0;
    int n;
    int pos = 0;
    // read byte-by-byte and assemble lines (simple and robust)
    while((n = read(fd, linebuf + pos, 1)) == 1) {
      if(linebuf[pos] == '\n') {
        linebuf[pos] = '\0';    // terminate line
        pos = 0;

        char u[NAME_MAX], hash[PASS_MAX];
        int uid;
        if(parse_line(linebuf, u, hash, &uid)) {
          // Compare username and verify password hash
          if(strcmp(u, username) == 0 && verify_password(password, hash)) {
            ok = 1;
            // set uid and break
            setuid(uid);
            break;
          }
        }
      } else {
        pos++;
        if(pos >= LINE_MAX-1) { // safety: truncate overly long lines
          linebuf[LINE_MAX-1] = '\0';
          pos = 0;
        }
      }
    }

    close(fd);

    if(!ok) {
      printf(1, "Login incorrect\n");
      continue;
    }

    // on success, exec a shell (child process of init will exec login)
    char *shargs[] = { "sh", 0 };
    exec("sh", shargs);
    // if exec returns, it failed
    printf(1, "login: exec sh failed\n");
    exit();
  }

  exit();
}
