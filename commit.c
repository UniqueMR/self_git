#include "cache.h"

extern int write_tree(int argc,char **argv);
extern int commmit_tree(int argc,char **argv);

int commit(int argc,char **argv)
{
    write_tree(argc,argv);
    commit_tree(argc,argv);
}
