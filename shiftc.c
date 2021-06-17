#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int shift_cipher(const uint8_t* message, size_t size, uint8_t* encrypted_message, const uint8_t* key, size_t key_len)
{
	if(!message || size == 0 || !encrypted_message) return 1;
	for(size_t i=0; i<size; ++i)
	{
		if((message[i] >= 'A' && message[i] <= 'Z'))
			encrypted_message[i] = ((message[i] - 'A') + key[i%key_len]) % 26 + 'A';
		else if((message[i] >= 'a' && message[i] <= 'z'))
			encrypted_message[i] = ((message[i] - 'a') + key[i%key_len]) % 26 + 'a';
		else encrypted_message[i] = message[i];
	}
	return 0;
}

int shift_decipher(const uint8_t* encrypted_message, size_t size, uint8_t* decrypted_message, const uint8_t* key, size_t key_len)
{
	if(!encrypted_message || size == 0 || !decrypted_message) return 1;
	for(size_t i=0; i<size; ++i)
	{
		if(encrypted_message[i] >= 'A' && encrypted_message[i] <= 'Z')
			decrypted_message[i] = (26 + (encrypted_message[i] - 'A') - key[i%key_len]) % 26 + 'A';
		else if(encrypted_message[i] >= 'a' && encrypted_message[i] <= 'z')
			decrypted_message[i] = (26 + (encrypted_message[i] - 'a') - key[i%key_len]) % 26 + 'a';
		else decrypted_message[i] = encrypted_message[i];
	}
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc > 3 && argc < 26+3) // shiftc -c "message" 1 2 3 4
	{
		const char* command = argv[1];
		int cipher_mode = 0 == strcmp(command, "-c") ? 0 : (0 == strcmp(command, "-d") ? 1 : -1);
		if(!(cipher_mode == 0 || cipher_mode == 1))
		{
			fprintf(stderr, "Invalid command! '%s'\n", command);
			return 1;
		}
		const char* message = argv[2];
		size_t msg_len = strlen(message), key_len = argc - 3;
		uint8_t keys[26];
		printf("Keys[%u]: ", key_len);
		for(int i=3; i<argc; ++i)
		{
			keys[i-3] = (uint8_t)atoi(argv[i]);
			printf("%hhu ", keys[i-3]);
		}
		printf("\n");
		char *buffer = calloc(msg_len+1, 1);
		if(!buffer) return -2;
		if(cipher_mode == 0) // cipher
		{
			if(0 != shift_cipher(message, msg_len, buffer, keys, key_len))
			{
				fprintf(stderr, "> Cipher failed!\n");
				free(buffer);
				return -3;
			}
			else
			{
				buffer[msg_len] = '\0';
				printf("\"%s\"\n", buffer);
			}
		}
		else
		{
			if(0 != shift_decipher(message, msg_len, buffer, keys, key_len))
			{
				fprintf(stderr, "> Decipher failed!\n");
				free(buffer);
				return -3;
			}
			else
			{
				buffer[msg_len] = '\0';
				printf("\"%s\"\n", buffer);
			}
		}
		free(buffer);
		return 0;
	}
	puts("shiftc [-c|-d] \"<message>\" <key_0> [key_1] ...\n");
	return 0;
}