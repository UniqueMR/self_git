#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

extern char *optarg;
extern int opterr;

int main(int argc,char **argv)
{
        int c,index;

        struct option opts[] = {
        {"init",no_argument,NULL,'i'},
        {"status",no_argument,NULL,'s'},
        {"add",required_argument,NULL,'a'},
        {"commit",no_argument,NULL,'c'},
        {"reset",no_argument,NULL,'r'},
        {"rm",required_argument,NULL,'m'},
        {0,0,0,0}
        };

        opterr = 0;

        while((c=getopt_long(argc,argv,"isa:crm:",opts,NULL)) != -1)
        {
                switch(c)
                {
                        case 'i':
                                printf("Reinitialized existing Git repository!\n");
                                break;
                        case 's':
                                printf("On branch master\n");
                                break;
                        case 'a':
                                printf("%s is added to cache!\n",optarg);
                                break;
                        case 'c':
                                printf("Completed to commmit!\n");
                                break;
                        case 'r':
                                printf("Completed to Reset!\n");
                                break;
                        case 'm':
                                printf("%s has been removed!\n",optarg);
                                break;
                        case '?':
                                printf("Usage : \n"
                                "-h host : set ip address\n"
                                "-p port : set port\n"
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