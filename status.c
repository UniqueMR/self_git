#include "cache.h"

static int verify_path(char *path)
{
	char c;

	goto inside;
	for (;;) {
		if (!c)
			return 1;
		if (c == '/') {
inside:
			c = *path++;
			if (c != '/' && c != '.' && c != '\0')
				continue;
			return 0;
		}
		c = *path++;
	}
}

int status(int argc,char** argv)
{
    printf("cache entry status:\n");
    printf("\n");

    int i, newfd, entries;
    struct stat st;

	entries = read_cache();
	if (entries < 0) {
		perror("cache corrupted");
		return -1;
	}

    for (i = 2 ; i < argc; i++) 
    {
		char *path = argv[i];
		if (!verify_path(path)) 
			continue;
        int j;
        for(j = 0; j < entries ;j++)
        {
            if(!strcmp(active_cache[j]->name,path))
			{
				printf("%s has been added to cache!\n",path);
				break;
			}
        }
        if(j == entries)
            printf("%s has not been added!\n",path);
	}

    return 0;
}