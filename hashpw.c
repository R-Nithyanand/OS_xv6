// hashpw.c - Host tool to generate password hashes for users file
// Compile on host: gcc -o hashpw hashpw.c
// Usage: ./hashpw password

#include <stdio.h>
#include <string.h>

// Simple DJB2 hash (same as in xv6)
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
  if(argc < 2) {
    fprintf(stderr, "usage: hashpw password\n");
    fprintf(stderr, "       hashpw password username uid\n");
    return 1;
  }
  
  char hash[9];
  hash_password(argv[1], hash);
  
  if(argc >= 4) {
    // Generate full line for users file
    printf("%s:%s:%s:/\n", argv[2], hash, argv[3]);
  } else {
    // Just print the hash
    printf("%s\n", hash);
  }
  
  return 0;
}
