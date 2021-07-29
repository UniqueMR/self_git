#include "cache.h"



const char *sha1_file_directory = NULL;
struct cache_entry **active_cache = NULL;
unsigned int active_nr = 0, active_alloc = 0;

void usage(const char *err)
{
	fprintf(stderr, "read-tree: %s\n", err);
	exit(1);
}

//得到输入字符的16进制值
static unsigned hexval(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return ~0;
}

//将表示16进制数的字符串转化为表示sha1值的字符串(字符串中的每个字符存储一个字节，总共20字节)
int get_sha1_hex(char *hex, unsigned char *sha1)
{
	int i;
	for (i = 0; i < 20; i++)//循环20次因为sha1值总是20字节
	{
		unsigned int val = (hexval(hex[0]) << 4) | hexval(hex[1]);
		if (val & ~0xff)
			return -1;
		*sha1++ = val;
		hex += 2;
	}
	return 0;
}

//将sha1转化成十六进制值
char * sha1_to_hex(unsigned char *sha1)
{
	static char buffer[50];
	static const char hex[] = "0123456789abcdef";
	char *buf = buffer;
	int i;

	for (i = 0; i < 20; i++)//循环20次因为sha1值总是20字节
	{
		unsigned int val = *sha1++;
		*buf++ = hex[val >> 4];//sha1字节的前四位作为索引，找到对应的十六进制值
		*buf++ = hex[val & 0xf];//sha1字节的后四位作为索引，找到对应的十六进制值
	}
	return buffer;
}

/*
 * NOTE! This returns a statically allocated buffer, so you have to be
 * careful about using it. Do a "strdup()" if you need to save the
 * filename.
 */
//根据sha1值得到sha1文件的路径
char *sha1_file_name(unsigned char *sha1)
{
	int i;
	static char *name, *base;
	if (!base) {
		char *sha1_file_directory = getenv(DB_ENVIRONMENT) ? : DEFAULT_DB_ENVIRONMENT;//首先定位sha1文件的目录
		int len = strlen(sha1_file_directory);
		base = malloc(len + 60);
		memcpy(base, sha1_file_directory, len);//将sha1文件目录（字符串首地址开始后len位拷贝到base中）
		memset(base+len, 0, 60);//将base+len地址开始的后60位设置为0
		base[len] = '/';
		base[len+3] = '/';
		name = base + len + 1;
	}
	//将sha1转换成16进制值
	for (i = 0; i < 20; i++) {
		static char hex[] = "0123456789abcdef";
		unsigned int val = sha1[i];
		char *pos = name + i*2 + (i > 0);
		*pos++ = hex[val >> 4];//sha1字节的前四位作为索引，找到对应的十六进制值
		*pos = hex[val & 0xf];//sha1字节的后四位作为索引，找到对应的十六进制值
	}
	return base;
}

//读取sha1文件（对传入的sha1进行解压操作）
void * read_sha1_file(unsigned char *sha1, char *type, unsigned long *size)
{
	z_stream stream;
	char buffer[8192];
	struct stat st;
	int i, fd, ret, bytes;
	void *map, *buf;//用void声明的指针，可以被任何类型赋值
	char *filename = sha1_file_name(sha1);//将sha1值转换为路径

	fd = open(filename, O_RDONLY);//根据路径获取文件
	if (fd < 0) {
		perror(filename);
		return NULL;
	}
	if (fstat(fd, &st) < 0)//fstat系统功能调用，获取文件信息于st中 
	{
		close(fd);
		return NULL;
	}
	//内存映射，将文件信息映射到内存中，内存空间自动分配（起点参数为null）
	map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (-1 == (int)(long)map)
		return NULL;

	/* Get the data stream */
	memset(&stream, 0, sizeof(stream));//将stream的所有位设置为0
	stream.next_in = map;//输入缓冲区起点为map
	stream.avail_in = st.st_size;//输入数据量为st_size
	stream.next_out = buffer;//输出缓冲区为buffer
	stream.avail_out = sizeof(buffer);//最大输出为buffer的容积

	//对数据流进行解压缩
	inflateInit(&stream);
	ret = inflate(&stream, 0);
	if (sscanf(buffer, "%10s %lu", type, size) != 2)
		return NULL;
	bytes = strlen(buffer) + 1;
	buf = malloc(*size);
	if (!buf)
		return NULL;

	//从源source所指的内存地址的起始位置开始拷贝n个字节到目标destin所指的内存地址的起始位置中
	//void *memcpy(void *destin, void *source, unsigned n);
	memcpy(buf, buffer + bytes, stream.total_out - bytes);//拷贝
	bytes = stream.total_out - bytes;
	if (bytes < *size && ret == Z_OK) {
		stream.next_out = buf + bytes;
		stream.avail_out = *size - bytes;
		while (inflate(&stream, Z_FINISH) == Z_OK)//不断进行解压缩操作，直到解压完成
			/* nothing */;
	}
	inflateEnd(&stream);
	return buf;
}

//写sha1文件（对输入的buf进行压缩操作）
int write_sha1_file(char *buf, unsigned len)
{
	int size;
	char *compressed;
	z_stream stream;
	unsigned char sha1[20];
	SHA_CTX c;

	/* Set it up */
	memset(&stream, 0, sizeof(stream));
	deflateInit(&stream, Z_BEST_COMPRESSION);
	size = deflateBound(&stream, len);
	compressed = malloc(size);

	/* Compress it */
	stream.next_in = buf;//stream输入缓冲区起点设置为buf,输入文件内容
	stream.avail_in = len;//输入长度设置为len 
	stream.next_out = compressed;//输出缓冲区起点为compressed容器，存储压缩结果
	stream.avail_out = size;//输出长度设置为size
	while (deflate(&stream, Z_FINISH) == Z_OK)
		/* nothing */;
	deflateEnd(&stream);
	size = stream.total_out;

	/* Sha1.. */
	//将压缩结果转化为20字节sha1值
	SHA1_Init(&c);
	SHA1_Update(&c, compressed, size);
	SHA1_Final(sha1, &c);

	if (write_sha1_buffer(sha1, compressed, size) < 0)
		return -1;
	printf("%s\n", sha1_to_hex(sha1));//打印sha1值
	return 0;
}

//写sha1文件
int write_sha1_buffer(unsigned char *sha1, void *buf, unsigned int size)
{
	char *filename = sha1_file_name(sha1);//根据sha1值得到sha1文件的路径
	int i, fd;

	fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, 0666);//获取文件资源（由SHA1文件目录和sha1值共同决定）
	if (fd < 0)
		return (errno == EEXIST) ? 0 : -1;
	write(fd, buf, size);
	close(fd);
	return 0;
}

static int error(const char * string)
{
	fprintf(stderr, "error: %s\n", string);
	return -1;
}

//验证cache_header
static int verify_hdr(struct cache_header *hdr, unsigned long size)
{
	SHA_CTX c;
	unsigned char sha1[20];

	if (hdr->signature != CACHE_SIGNATURE)
		return error("bad signature");
	if (hdr->version != 1)
		return error("bad version");
	SHA1_Init(&c);
	//int SHA1_Update(SHA_CTX *c, const void *data, unsigned long len);
	//第二个参数const void *data为void类型的指针，可以被任何地址赋值
	SHA1_Update(&c, hdr, offsetof(struct cache_header, sha1));
	SHA1_Update(&c, hdr+1, size - sizeof(*hdr));
	SHA1_Final(sha1, &c);
	if (memcmp(sha1, hdr->sha1, 20))
		return error("bad header sha1");
	return 0;
}

//读缓冲区
int read_cache(void)
{
	int fd, i;
	struct stat st;
	unsigned long size, offset;
	void *map;
	struct cache_header *hdr;

	errno = EBUSY;
	if (active_cache)
		return error("more than one cachefile");
	errno = ENOENT;
	sha1_file_directory = getenv(DB_ENVIRONMENT);//获取sha1文件目录
	if (!sha1_file_directory)
		sha1_file_directory = DEFAULT_DB_ENVIRONMENT;
	if (access(sha1_file_directory, X_OK) < 0)
		return error("no access to SHA1 file directory");
	fd = open(".dircache/index", O_RDONLY);//以只读方式打开index文件
	if (fd < 0)
		return (errno == ENOENT) ? 0 : error("open failed");

	map = (void *)-1;
	if (!fstat(fd, &st)) {
		map = NULL;
		size = st.st_size;
		errno = EINVAL;
		if (size > sizeof(struct cache_header))
			map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);//将文件映射至内存
	}
	close(fd);//关闭index文件
	if (-1 == (int)(long)map)
		return error("mmap failed");

	hdr = map;//hdr指针指向文件在内存中映射的首地址
	if (verify_hdr(hdr, size) < 0)
		goto unmap;

	active_nr = hdr->entries;
	active_alloc = alloc_nr(active_nr);
	active_cache = calloc(active_alloc, sizeof(struct cache_entry *));

	offset = sizeof(*hdr);
	for (i = 0; i < hdr->entries; i++) {
		struct cache_entry *ce = map + offset;
		offset = offset + ce_size(ce);
		active_cache[i] = ce;
	}
	return active_nr;

unmap:
	munmap(map, size);
	errno = EINVAL;
	return error("verify header failed");
}

