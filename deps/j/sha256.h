#ifndef _SHA256_H
#define _SHA256_H 
#include <stdint.h>

/* Number of bytes in a SHA-256 hash value. */
#define SHA256_SIZE  32
#define SHA256_BUF   33 // NUL byte


/* SHA-256 context block. */
struct sha256_t {
        unsigned char   input[64];
        uint32_t        len_input;
        uint32_t        A, B, C, D, E, F, G, H;
        uint64_t        len_total;
};


extern void sha256_init(struct sha256_t *sha);
extern void sha256_data(struct sha256_t *sha, const void *buffer, unsigned long len);
extern void sha256_hash(struct sha256_t *sha, unsigned char hash[SHA256_SIZE]);

extern void sha256    (const void *data, unsigned long len, unsigned char *hash);
extern void sha256_fd (int fd, unsigned char *hash);
extern int  sha256_cmp(unsigned char *sum1, unsigned char *sum2);

extern void sha256_print(unsigned char *hash);
extern void sha256_sprint(char *dst, unsigned char *hash);



#endif  

