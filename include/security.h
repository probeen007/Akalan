#ifndef SECURITY_H
#define SECURITY_H

#include "common.h"

// Password hashing
bool hash_password(const char *password, char *hash_output, size_t hash_size);

// Password verification
bool verify_password(const char *password, const char *hash);

// Generate salt for password hashing
void generate_salt(char *salt, size_t salt_size);

#endif // SECURITY_H
