#include "cache.h"

//检查sha1值是否有效
static int check_valid_sha1(unsigned char *sha1)
{
	char *filename = sha1_file_name(sha1);//由sha1值转换得到路径
	int ret;

	/* If we were anal, we'd check that the sha1 of the contents actually matches */
	ret = access(filename, R_OK);//判断函数是否具有指定的权限（指定的权限取决于第二个参数，在这里是R_OK，即判断函数是否具有只读权限）
	if (ret)
		perror(filename);
	return ret;
}

//将数值val转换成数组的形式（类比数据流）（以大字节序存储，即数组的高位存储整数的低位）
static int prepend_integer(char *buffer, unsigned val, int i)
{
	buffer[--i] = '\0';
	do {
		buffer[--i] = '0' + (val % 10);
		val /= 10;
	} while (val);
	return i;
}

#define ORIG_OFFSET (40)	/* Enough space to add the header of "tree <size>\0" */

//向cache中写入tree文件
int write_tree(int argc, char **argv,unsigned char* sha1)
{
	unsigned long size, offset, val;
	int i, entries = read_cache();//读取cache，返回blob文件的数量
	char *buffer;

	char* _sha1 = (char*)malloc(20*sizeof(char));

	if (entries <= 0) {
		fprintf(stderr, "No file-cache to create a tree of\n");
		exit(1);
	}

	/* Guess at an initial size */
	size = entries * 40 + 400;
	buffer = malloc(size);
	offset = ORIG_OFFSET;

	for (i = 0; i < entries; i++) 
	{
		struct cache_entry *ce = active_cache[i];
		if (check_valid_sha1(ce->sha1) < 0)
			exit(1);
		if (offset + ce->namelen + 60 > size)//用于应对空间不够的情况，重新分配空间 
		{
			size = alloc_nr(offset + ce->namelen + 60);
			buffer = realloc(buffer, size);
		}
		offset += sprintf(buffer + offset, "%o %s", ce->st_mode, ce->name);//输出cache entry的文件类型和文件名
		buffer[offset++] = 0;
		memcpy(buffer + offset, ce->sha1, 20);//将cache entry的sha1值复制到buffer中
		offset += 20;//维护offset
	}

	i = prepend_integer(buffer, offset - ORIG_OFFSET, ORIG_OFFSET);
	i -= 5;
	memcpy(buffer+i, "tree ", 5);//完成前置操作后，将文件类型tree写入

	buffer += i;
	offset -= i;

	write_sha1_file(buffer, offset,_sha1);//写sha1文件，buffer为需要写入的内容
	strcpy(sha1,_sha1);
	return 0;
}
