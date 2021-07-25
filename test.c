#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

extern char *optarg;
extern int opterr;

int main(int argc,char **argv)
{
        int c,index;
        char host[128] = "127.0.0.1";
        int port = 8000;

        struct option opts[] = {
        {"host",required_argument,NULL,'h'},
        {"port",required_argument,NULL,'p'},
        {0,0,0,0}
        };

        opterr = 0;

        while((c=getopt_long(argc,argv,"h:p:",opts,NULL)) != -1)
        {
                switch(c)
                {
                        case 'h':
                                strcpy(host,optarg);
                                break;
                        case 'p':
                                port = atoi(optarg);
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

        printf( "ip   : %s\n"
                "port : %d\n",
                host,port);

        for(index = optind;index < argc;index++)
                printf("Non-option argument %s\n",argv[index]);
        return 0;
}