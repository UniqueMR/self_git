#include "cache.h"

extern int write_tree(int argc,char **argv,unsigned char *sha1);
extern int commmit_tree(int argc,char **argv,unsigned char *tree_sha1);

int commit(int argc,char **argv)
{
    unsigned char *sha1 = (char*)malloc(20 * sizeof(unsigned char));
    write_tree(argc,argv,sha1);
    commit_tree(argc,argv,sha1);
}