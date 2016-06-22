
#ifndef md5_INCLUDED
#  define md5_INCLUDED

#define DIGEST_LEN 16 // 摘要长度
#define HASH_HEX_LEN 1+2*DIGEST_LEN // Hash值长度

typedef unsigned char md5_byte_t; /* 8-bit byte */
typedef unsigned int md5_word_t; /* 32-bit word */

/* Define the state of the MD5 Algorithm. */
typedef struct md5_state_s {
    md5_word_t count[2];	/* message length in bits, lsw first */
    md5_word_t abcd[4];		/* digest buffer */
    md5_byte_t buf[64];		/* accumulate block */
} md5_state_t;


/* Initialize the algorithm. */
void md5_init(md5_state_t *pms);

/* Append a string to the message. */
void md5_append(md5_state_t *pms, const md5_byte_t *data, int nbytes);

/* Finish the message and return the digest. */
void md5_finish(md5_state_t *pms, md5_byte_t digest[16]);

// 根据MD5算计算字符串的Hash值
void MD5_Hash(char *hash, char *input,int len);

#endif /* md5_INCLUDED */
