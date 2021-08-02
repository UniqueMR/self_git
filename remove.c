#include "cache.h"
extern int read_cache();

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
    int entries = read_cache();
    if (entries < 0) {
		perror("cache corrupted");
		return -1;
	}

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
static int remove_file_from_cache(char *path)
{
	int pos = cache_name_pos(path, strlen(path));//定位所要删除的文件
    printf("%d\n",pos);
	if (pos < 0)//由cache_name_pos可知，当匹配成功时，返回值为-next-1小于0 
	{
		//memmove用于拷贝字节，如果目标区域和源区域有重叠的话，memmove能够保证源串在被覆盖之前将重叠区域的字节拷贝到目标区域中，但复制后源内容会被更改
		//原型：void *memmove( void* dest, const void* src, size_t count );由src所指内存区域复制count个字节到dest所指内存区域。
		//这里通过将后面cache的向前覆盖，从而实现对指定cache内容的删除
		pos = -pos-1;
		for(int i = pos + 1;i < active_nr;i++)
            active_cache[i - 1] = active_cache[i];
        active_nr--;
	}
}

int remove_function(int argc,char** argv)
{
    char* path = argv[2];
    remove_file_from_cache(path);
    for(int i = 0;i < active_nr;i++)
        printf("%s\n",active_cache[i]->name);
}