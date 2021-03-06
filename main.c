#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rsa.h"


static int usage(FILE *fp)
{
	return fprintf(fp,
"Usage:\n"
"  rsa encrypt <keyfile> <message>\n"
"  rsa decrypt <keyfile> <ciphertext>\n"
"  rsa genkey <numbits>\n"
	);
}

/* Encode the string s into an integer and store it in x. We're assuming that s
 * does not have any leading \x00 bytes (otherwise we would have to encode how
 * many leading zeros there are). */
static void encode(mpz_t x, const char *s)
{
	mpz_import(x, strlen(s), 1, 1, 0, 0, s);
}

/* Decode the integer x into a NUL-terminated string and return the string. The
 * returned string is allocated using malloc and it is the caller's
 * responsibility to free it. If len is not NULL, store the length of the string
 * (not including the NUL terminator) in *len. */
static char *decode(const mpz_t x, size_t *len)
{
	void (*gmp_freefunc)(void *, size_t);
	size_t count;
	char *s, *buf;

	buf = mpz_export(NULL, &count, 1, 1, 0, 0, x);

	s = malloc(count + 1);
	if (s == NULL)
		abort();
	memmove(s, buf, count);
	s[count] = '\0';
	if (len != NULL)
		*len = count;

	/* Ask GMP for the appropriate free function to use. */
	mp_get_memory_functions(NULL, NULL, &gmp_freefunc);
	gmp_freefunc(buf, count);

	return s;
}

/* The "encrypt" subcommand.
 *
 * The return value is the exit code of the program as a whole: nonzero if there
 * was an error; zero otherwise. */
static int encrypt_mode(const char *key_filename, const char *message)
{
	/* TODO */
	struct rsa_key key;
	mpz_t x;
	mpz_t c;
	mpz_init(x);
	encode(x, message);
	if (mpz_cmp(x, key.n) > 0) {
		return 1;
	}
	mpz_init(c);
	rsa_key_init(&key);
	if (rsa_key_load_public(key_filename, &key) == 0) {
		rsa_encrypt(c, x, &key);
		gmp_printf("%Zd",c);
		rsa_key_clear(&key);
	}
	return rsa_key_load_public(key_filename, &key);
	
}

/* The "decrypt" subcommand. c_str should be the string representation of an
 * integer ciphertext.
 *
 * The return value is the exit code of the program as a whole: nonzero if there
 * was an error; zero otherwise. */
static int decrypt_mode(const char *key_filename, const char *c_str)
{
	/* TODO */
	struct rsa_key key;
	mpz_t c;
	mpz_t m;
	mpz_init(m);
	mpz_init_set_str(c, c_str, 10);
	if (mpz_cmp(c, key.n) > 0) {
		return 1;
	}
	rsa_key_init(&key);
	if (rsa_key_load_private(key_filename, &key) == 0){
		rsa_decrypt(m, c, &key);
		char* d = decode(m, NULL);
		printf("%s", d);
		rsa_key_clear(&key);
	}
	return rsa_key_load_private(key_filename, &key);
}

/* The "genkey" subcommand. numbits_str should be the string representation of
 * an integer number of bits (e.g. "1024").
 *
 * The return value is the exit code of the program as a whole: nonzero if there
 * was an error; zero otherwise. */
static int genkey_mode(const char *numbits_str)
{
	/* TODO */
	struct rsa_key key;
	mpz_t c;
	mpz_t d;
	mpz_init(d);
	mpz_init_set_str(c, numbits_str, 10);
	if (atoi(numbits_str) % 16 != 0) {
		return 1;
	}
	rsa_key_init(&key);
	rsa_genkey(&key, atoi(numbits_str));
	rsa_key_write(stdout, &key);
	
	return 0;
}

int main(int argc, char *argv[])
{
	/*mpz_t c;
	mpz_init(c);
	int i = mpz_set_str (c, argv[1], 10);
	mpz_clear(c);*/
	
	const char *command;

	if (argc < 2) {
		usage(stderr);
		return 1;
	}
	command = argv[1];

	if (strcmp(command, "-h") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "help") == 0) {
		usage(stdout);
		return 0;
	} else if (strcmp(command, "encrypt") == 0) {
		const char *key_filename, *message;

		if (argc != 4) {
			fprintf(stderr, "encrypt needs a key filename and a message\n");
			return 1;
		}
		key_filename = argv[2];
		message = argv[3];

		return encrypt_mode(key_filename, message);
	} else if (strcmp(command, "decrypt") == 0) {
		const char *key_filename, *c_str;

		if (argc != 4) {
			fprintf(stderr, "decrypt needs a key filename and a ciphertext\n");
			return 1;
		}
		key_filename = argv[2];
		c_str = argv[3];

		return decrypt_mode(key_filename, c_str);
	} else if (strcmp(command, "genkey") == 0) {
		const char *numbits_str;

		if (argc != 3) {
			fprintf(stderr, "genkey needs a number of bits\n");
			return 1;
		}
		numbits_str = argv[2];

		return genkey_mode(numbits_str);
	}

	usage(stderr);
	return 1;
}
