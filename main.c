#include "tiger.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		puts("Message expected as argument.");
		return -1;
	}
	
	uint8_t res[24];

	tiger((uint64_t*)argv[1], strlen(argv[1]), 3, (uint64_t*)res);

	for (int i = 0; i < 24; ++i)
		printf("%02hhX", res[i]);

	putchar('\n');
	return 0;
}
