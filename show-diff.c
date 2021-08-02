#include "cache.h"

#define MTIME_CHANGED	0x0001
#define CTIME_CHANGED	0x0002
#define OWNER_CHANGED	0x0004
#define MODE_CHANGED    0x0008
#define INODE_CHANGED   0x0010
#define DATA_CHANGED    0x0020

//文件状态匹配，对比stat结构体中的所有状态变量
static int match_stat(struct cache_entry *ce, struct stat *st)
{
	unsigned int changed = 0;

	if (ce->mtime.sec  != (unsigned int)st->st_mtim.tv_sec ||
	    ce->mtime.nsec != (unsigned int)st->st_mtim.tv_nsec)
		changed |= MTIME_CHANGED;
	if (ce->ctime.sec  != (unsigned int)st->st_ctim.tv_sec ||
	    ce->ctime.nsec != (unsigned int)st->st_ctim.tv_nsec)
		changed |= CTIME_CHANGED;
	if (ce->st_uid != (unsigned int)st->st_uid ||
	    ce->st_gid != (unsigned int)st->st_gid)
		changed |= OWNER_CHANGED;
	if (ce->st_mode != (unsigned int)st->st_mode)
		changed |= MODE_CHANGED;
	if (ce->st_dev != (unsigned int)st->st_dev ||
	    ce->st_ino != (unsigned int)st->st_ino)
		changed |= INODE_CHANGED;
	if (ce->st_size != (unsigned int)st->st_size)
		changed |= DATA_CHANGED;
	return changed;
}

//显示不同
static void show_differences(struct cache_entry *ce, struct stat *cur,
	void *old_contents, unsigned long long old_size)
{
	static char cmd[1000];
	FILE *f;

	//C 库函数 int snprintf(char *str, size_t size, const char *format, ...) 设将可变参数(...)按照 format 格式化成字符串，并将字符串复制到 str 中，size 为要写入的字符的最大数目，超过 size 会被截断。
	snprintf(cmd, sizeof(cmd), "diff -u - %s", ce->name);
	//Linux中的popen()函数可以在程序中执行一个shell命令，并返回命令执行的结果。
	//有两种操作模式，分别为读和写。在读模式中，程序中可以读取到命令的输出，其中有一个应用就是获取网络接口的参数。在写模式中，最常用的是创建一个新的文件或开启其他服务等。
	//如果type是"r"则文件指针连接到command的标准输出；如果type是"w"则文件指针连接到command的标准输入。
	//command参数是一个指向以NULL结束的shell命令字符串的指针。这行命令将被传到bin/sh并使用-c标志，shell将执行这个命令。
	//popen()的返回值是个标准I/O流，必须由pclose来终止。前面提到这个流是单向的（只能用于读或写）。向这个流写内容相当于写入该命令的标准输入，命令的标准输出和调用popen()的进程相同；与之相反的，从流中读数据相当于读取命令的标准输出，命令的标准输入和调用popen()的进程相同。
	//如果调用fork()或pipe()失败，或者不能分配内存将返回NULL，否则返回标准I/O流
	//此处用以将cmd中的命令"diff -u - ce->name"写入
	f = popen(cmd, "w");
	//size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) 把 ptr 所指向的数组中的数据写入到给定流 stream 中。
	//ptr -- 这是指向要被写入的元素数组的指针。
	//size -- 这是要被写入的每个元素的大小，以字节为单位
	//nmemb -- 这是元素的个数，每个元素的大小为 size 字节
	//stream -- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输出流。
	fwrite(old_contents, old_size, 1, f);
	//命令的标准输出和调用popen()的进程相同
	pclose(f);
}

int show_diff(int argc, char **argv)
{
	int entries = read_cache();//initialize the cache information
	//entries记载了cache中的文件数量
	//同时cache中的内容被加载到全局变量active_cache中
	int i;

	//errors in initializing cache information
	if (entries < 0) {
		perror("read_cache");
		exit(1);
	}

	for (i = 0; i < entries; i++) {
		struct stat st;
		struct cache_entry *ce = active_cache[i];
		int n, changed;
		unsigned int mode;
		unsigned long size;
		char type[20];
		void *new;

		//stat获取文件状态到st中
		if (stat(ce->name, &st) < 0) 
		{
			printf("%s: %s\n", ce->name, strerror(errno));
			continue;
		}
		changed = match_stat(ce, &st);//将暂存区读入ce和现有状态st进行比较

		//如果没有改变，输出ok
		if (!changed) {
			printf("%s: ok\n", ce->name);
			continue;
		}

		//如果发生改变
		printf("%.*s:  ", ce->namelen, ce->name);//输出暂存区的名字和长度
		for (n = 0; n < 20; n++)
			printf("%02x", ce->sha1[n]);
		printf("\n");
		new = read_sha1_file(ce->sha1, type, &size);//new获取ce->sha1文件解压后的
		show_differences(ce, &st, new, size);//展示不同
		free(new);
	}
	return 0;
}
