#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "include.h"
#include "cache.h"

extern char *optarg;
extern int opterr;

int main(int argc,char **argv)
{
        int c,index;

        struct option opts[] = {
        {"init",no_argument,NULL,'i'},
        {"status",no_argument,NULL,'s'},
        {"diff",no_argument,NULL,'d'},
        {"add",required_argument,NULL,'a'},
        {"commit",no_argument,NULL,'c'},
        {"reset",no_argument,NULL,'r'},
        {"rm",required_argument,NULL,'m'},
        {"cat-file",required_argument,NULL,'f'},
        {0,0,0,0}
        };

        opterr = 0;

        while((c=getopt_long(argc,argv,"isda:crm:f:",opts,NULL)) != -1)
        {
                switch(c)
                {
                        case 'i':
                                init(argc,argv);
                                printf("Reinitialized existing Git repository!\n");
                                break;
                        case 's':
                                printf("On branch master\n");
                                break;
                        case 'a':
                                update_cache(argc, argv);
                                printf("%s is added to cache!\n",optarg);
                                break;
                        case 'd':
                                show_diff(argc,argv);
                                break;
                        case 'c':
                                commit(argc,argv);
                                printf("Completed to commmit!\n");
                                break;
                        case 'r':
                                printf("Completed to Reset!\n");
                                break;
                        case 'm':
                                printf("%s has been removed!\n",optarg);
                                break;
                        case 'f':
                                status(argc,argv);
                                break;
                        case '?':
                                printf("Usage : \n"
                                "--init : init a git repository\n"
                                "--status : current status of git repository\n"
                                "--add : add file to cache\n"
                                "--commit : commit cache into commit history\n"
                                "--reset : reset commit\n"
                                "--rm : remove file from cache\n"
                                );
                                return 1;
                        default:
                                break;
                }
        }

        for(index = optind;index < argc;index++)
                printf("Non-option argument %s\n",argv[index]);
        return 0;
}