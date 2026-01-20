#include "security.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <string.h>

// SHA-256 based password hashing with salt
// For production, consider using bcrypt or argon2
bool hash_password(const char *password, char *hash_output, size_t hash_size) {
    if (!password || !hash_output || hash_size < 81) {  // 64 (hash) + 16 (salt) + 1 (null)
        return false;
    }
    
    // Generate 16-character salt
    char salt[17];
    generate_salt(salt, sizeof(salt));
    
    // Combine salt + password
    char salted_password[512];
    snprintf(salted_password, sizeof(salted_password), "%s%s", salt, password);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    
    if (!SHA256_Init(&sha256)) {
        return false;
    }
    
    if (!SHA256_Update(&sha256, salted_password, strlen(salted_password))) {
        return false;
    }
    
    if (!SHA256_Final(hash, &sha256)) {
        return false;
    }
    
    // Store salt first (16 chars) then hash (64 chars)
    strcpy(hash_output, salt);
    
    // Convert hash to hex string starting at position 16
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_output + 16 + (i * 2), "%02x", hash[i]);
    }
    hash_output[80] = '\0';
    
    return true;
}

bool verify_password(const char *password, const char *hash) {
    if (!password || !hash) {
        return false;
    }
    
    // Extract salt from stored hash (first 16 characters)
    char salt[17];
    strncpy(salt, hash, 16);
    salt[16] = '\0';
    
    // Combine salt + password
    char salted_password[512];
    snprintf(salted_password, sizeof(salted_password), "%s%s", salt, password);
    
    // Compute hash of salted password
    unsigned char computed_hash_bytes[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    
    if (!SHA256_Init(&sha256)) {
        return false;
    }
    
    if (!SHA256_Update(&sha256, salted_password, strlen(salted_password))) {
        return false;
    }
    
    if (!SHA256_Final(computed_hash_bytes, &sha256)) {
        return false;
    }
    
    // Convert to hex string
    char computed_hash_hex[65];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(computed_hash_hex + (i * 2), "%02x", computed_hash_bytes[i]);
    }
    computed_hash_hex[64] = '\0';
    
    // Compare with stored hash (skip first 16 chars which are salt)
    return strcmp(computed_hash_hex, hash + 16) == 0;
}

void generate_salt(char *salt, size_t salt_size) {
    if (!salt || salt_size < 17) {
        return;
    }
    
    unsigned char random_bytes[8];
    RAND_bytes(random_bytes, sizeof(random_bytes));
    
    for (int i = 0; i < 8; i++) {
        sprintf(salt + (i * 2), "%02x", random_bytes[i]);
    }
    salt[16] = '\0';
}
