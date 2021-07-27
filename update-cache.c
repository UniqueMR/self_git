#include "cache.h"

//对cache的命名进行比较
static int cache_name_compare(const char *name1, int len1, const char *name2, int len2)
{
	int len = len1 < len2 ? len1 : len2;//find the shorter length
	int cmp;

	//将name1和name2的前len个字节进行比较
	//返回值小于0说明name1 < name2，大于0说明name1 > name2，等于0说明相同
	cmp = memcmp(name1, name2, len);
	if (cmp)
		return cmp;
	if (len1 < len2)
		return -1;
	if (len1 > len2)
		return 1;
	return 0;
}//对cache的命名进行比较

//计算给定cache名称(输入参数name)所对应的位置
static int cache_name_pos(const char *name, int namelen)
{
	int first, last;

	first = 0;
	last = active_nr;
	while (last > first) {
		int next = (last + first) >> 1;
		struct cache_entry *ce = active_cache[next];//对cache进行扫描，逐一判断是否和当前名称匹配
		int cmp = cache_name_compare(name, namelen, ce->name, ce->namelen);
		//如果等于0说明匹配完成
		//这里取负号是为了使得匹配成功后的返回值小于0，从而通过返回值小于0判断匹配成功
		//最终还要-1是防止第一个cache就匹配成功，从而使得返回值并不小于0
		if (!cmp)
			return -next-1;

		//由于cache的分片编号按递增顺序排列，因此如果当前name比目标name“小”，则继续寻找
		if (cmp < 0) {
			last = next;
			continue;
		}
		first = next+1;
	}
	return first;
}

//在cache中删除文件，传入所要删除文件的路径
//（这里的path理解为所要删除问价的sha1值？）
static int remove_file_from_cache(char *path)
{
	int pos = cache_name_pos(path, strlen(path));//定位所要删除的文件
	if (pos < 0)//由cache_name_pos可知，当匹配成功时，返回值为-next-1小于0 
	{
		//memmove用于拷贝字节，如果目标区域和源区域有重叠的话，memmove能够保证源串在被覆盖之前将重叠区域的字节拷贝到目标区域中，但复制后源内容会被更改
		//原型：void *memmove( void* dest, const void* src, size_t count );由src所指内存区域复制count个字节到dest所指内存区域。
		//这里通过将后面cache的向前覆盖，从而实现对指定cache内容的删除
		pos = -pos-1;
		active_nr--;
		if (pos < active_nr)
			memmove(active_cache + pos, active_cache + pos + 1, (active_nr - pos - 1) * sizeof(struct cache_entry *));
	}
}

static int add_cache_entry(struct cache_entry *ce)
{
	int pos;

	//定位所要加入文件的位置
	pos = cache_name_pos(ce->name, ce->namelen);

	/* existing match? Just replace it */
	if (pos < 0) {
		active_cache[-pos-1] = ce;
		return 0;
	}

	/* Make sure the array is big enough .. */
	if (active_nr == active_alloc) {
		active_alloc = alloc_nr(active_alloc);
		active_cache = realloc(active_cache, active_alloc * sizeof(struct cache_entry *));
	}

	/* Add it in.. */
	active_nr++;//增加后更新
	if (active_nr > pos)
		//利用memmove，先腾出一个位置，再将cache插入
		memmove(active_cache + pos + 1, active_cache + pos, (active_nr - pos - 1) * sizeof(ce));
	active_cache[pos] = ce;
	return 0;
}

//维护index文件
static int index_fd(const char *path, int namelen, struct cache_entry *ce, int fd, struct stat *st)
{
	z_stream stream;
	int max_out_bytes = namelen + st->st_size + 200;//最大输出字节数
	void *out = malloc(max_out_bytes);
	void *metadata = malloc(namelen + 200);
	//属于sys/mman.h，实现内存映射，将普通文件映射到内存中
	//void *mmap(void *start,size_t length,int prot,int flags,int fd,off_t offsize);
	//参数start：指向欲映射的内存起始地址，通常设为 NULL，代表让系统自动选定地址，映射成功后返回该地址。
	//参数length：代表将文件中多大的部分映射到内存。
	//参数prot：映射区域的保护方式。可以为以下几种方式的组合：
	//PROT_EXEC       映射区域可被执行
	//PROT_READ       映射区域可被读取
	//PROT_WRITE      映射区域可被写入
	//PROT_NONE       映射区域不能存取
	//参数flags：影响映射区域的各种特性。在调用mmap()时必须要指定MAP_SHARED 或MAP_PRIVATE。
	void *in = mmap(NULL, st->st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	SHA_CTX c;

	close(fd);
	if (!out || (int)(long)in == -1)
		return -1;

	//C 库函数 void *memset(void *str, int c, size_t n) 复制字符 c（一个无符号字符）到参数 str 所指向的字符串的前 n 个字符。
	//此处将stream初始化为0
	memset(&stream, 0, sizeof(stream));
	//deflate是一种压缩算法，这里进行压缩前的初始化
	deflateInit(&stream, Z_BEST_COMPRESSION);

	/*
	 * ASCII size + nul byte
	 */	
	stream.next_in = metadata;
	stream.avail_in = 1+sprintf(metadata, "blob %lu", (unsigned long) st->st_size);
	stream.next_out = out;
	stream.avail_out = max_out_bytes;
	while (deflate(&stream, 0) == Z_OK)
		/* nothing */;

	/*
	 * File content
	 */
	stream.next_in = in;
	stream.avail_in = st->st_size;
	while (deflate(&stream, Z_FINISH) == Z_OK)
		/*nothing */;

	//结束压缩
	deflateEnd(&stream);
	
	//生成sha1值
	SHA1_Init(&c);
	SHA1_Update(&c, out, stream.total_out);
	SHA1_Final(ce->sha1, &c);

	return write_sha1_buffer(ce->sha1, out, stream.total_out);
}

//向cache中增加文件
static int add_file_to_cache(char *path)
{
	int size, namelen;
	struct cache_entry *ce;
	struct stat st;
	int fd;

	//打开路径为path的文件
	fd = open(path, O_RDONLY);
	if (fd < 0)//打开失败  
	{
		if (errno == ENOENT)
			return remove_file_from_cache(path);
		return -1;
	}
	//fd为文件描述词，st为保存文件信息的结构体
	if (fstat(fd, &st) < 0) {
		close(fd);
		return -1;
	}
	namelen = strlen(path);
	size = cache_entry_size(namelen);
	ce = malloc(size);
	//为ce分配大小为size的内存空间
	memset(ce, 0, size);
	//将路径path前namelen长度的内容复制到ce->name中
	memcpy(ce->name, path, namelen);
	//将st中的所有内容复制到ce中
	ce->ctime.sec = st.st_ctime;
	ce->ctime.nsec = st.st_ctim.tv_nsec;
	ce->mtime.sec = st.st_mtime;
	ce->mtime.nsec = st.st_mtim.tv_nsec;
	ce->st_dev = st.st_dev;
	ce->st_ino = st.st_ino;
	ce->st_mode = st.st_mode;
	ce->st_uid = st.st_uid;
	ce->st_gid = st.st_gid;
	ce->st_size = st.st_size;
	ce->namelen = namelen;

	if (index_fd(path, namelen, ce, fd, &st) < 0)
		return -1;

	return add_cache_entry(ce);
}

//写cache
static int write_cache(int newfd, struct cache_entry **cache, int entries)
{
	SHA_CTX c;
	struct cache_header hdr;
	int i;

	hdr.signature = CACHE_SIGNATURE;
	hdr.version = 1;
	hdr.entries = entries;

	SHA1_Init(&c);
	SHA1_Update(&c, &hdr, offsetof(struct cache_header, sha1));
	for (i = 0; i < entries; i++) {
		struct cache_entry *ce = cache[i];
		int size = ce_size(ce);
		SHA1_Update(&c, ce, size);
	}
	SHA1_Final(hdr.sha1, &c);

	if (write(newfd, &hdr, sizeof(hdr)) != sizeof(hdr))
		return -1;

	for (i = 0; i < entries; i++) {
		struct cache_entry *ce = cache[i];
		int size = ce_size(ce);
		if (write(newfd, ce, size) != size)
			return -1;
	}
	return 0;
}		

/*
 * We fundamentally don't like some paths: we don't want
 * dot or dot-dot anywhere, and in fact, we don't even want
 * any other dot-files (.dircache or anything else). They
 * are hidden, for chist sake.
 *
 * Also, we don't want double slashes or slashes at the
 * end that can make pathnames ambiguous. 
 */
//检验路径是否符合要求
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

int update_cache(int argc, char **argv)
{
	int i, newfd, entries;

	entries = read_cache();
	if (entries < 0) {
		perror("cache corrupted");
		return -1;
	}
	//获取".dircache/index.lock"文件的描述符
	newfd = open(".dircache/index.lock", O_RDWR | O_CREAT | O_EXCL, 0600);
	if (newfd < 0) {
		perror("unable to create new cachefile");
		return -1;
	}
	for (i = 1 ; i < argc; i++) {
		char *path = argv[i];
		if (!verify_path(path)) {
			fprintf(stderr, "Ignoring path %s\n", argv[i]);
			continue;
		}
		if (add_file_to_cache(path)) {
			fprintf(stderr, "Unable to add %s to database\n", path);
			goto out;
		}
	}
	if (!write_cache(newfd, active_cache, active_nr) && !rename(".dircache/index.lock", ".dircache/index"))
		return 0;
out:
	unlink(".dircache/index.lock");
}
