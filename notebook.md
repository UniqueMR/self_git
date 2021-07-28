### Git的三种对象
* blob对象：保存文件快照
* tree对象：记录目录结构和blob对象索引
* commit对象：包含指向前述tree对象的指针和所有提交信息

### Git的三个区
* 工作区(Workspace):直接修改代码的地方
* 暂存区(index)：数据暂时存放的区域，用于在工作区和版本库之间进行数据交流
* 版本库(commit history)：存放已经提交的数据

### 可执行文件的具体分工
* init-db：创建初始化仓库
* update-cache：将workspace的变更写到索引文件(index)中
* write-tree：将之前所有的变更整理成tree对象
* commit-tree：将指定的tree对象写到本地的版本库(commit history)中
* show-diff：查看工作区和暂存区的文件差异
* read-tree：读取tree对象信息

### objects文件
存储三个重要对象的载体
* 默认路径：.dircache/objects
* 路径可由环境变量SHA1_FILE_DIRECTORY指定
    文件路径和名称根据SHA1值决定，取SHA1值的第一个字节的hex值(十六进制值)为目录，其它字节的hex值为名称
* 使用cat-file查看object文件是什么类型的对象

### 索引文件
* 默认路径： .dircache/index
* 用来存储变更文件的相关信息，当运行update-cache时会添加变更文件的信息到索引文件中
* .dircache/index.lock存在表示当前工作区被锁定，无法进行提交操作
* 使用hexdump命令查看索引文件的内容

### 哈希算法
* 初代版本使用的是sha1算法

### zlib压缩算法
* 利用zlib进行压缩操作的流程
1. 创建z_stream对象stream和SHA_CTX对象c
```c
z_stream stream; 
SHA_CTX c;
```
2. 对stream对象进行初始化操作
```c
	memset(&stream, 0, sizeof(stream));
	deflateInit(&stream, Z_BEST_COMPRESSION);
	size = deflateBound(&stream, len);
	compressed = malloc(size);

```

3. 进行压缩操作
```c
	stream.next_in = buf;//buf为sha1值
	stream.avail_in = len;
	stream.next_out = compressed;
	stream.avail_out = size;
	while (deflate(&stream, Z_FINISH) == Z_OK);//不断进行压缩直到压缩完成
	deflateEnd(&stream);
	size = stream.total_out;
```

4. 获取sha1值(利用之前创建的SHA_CTX对象)
```c
	SHA1_Init(&c);
	SHA1_Update(&c, compressed, size);
	SHA1_Final(sha1, &c);
```


### sha1(secure hash algorithm 1)
* sha1可以生成一个被称为消息摘要的160位（20字节）散列值，散列值的通常呈现形式为40个16进制数
* 相关函数

int SHA1_Init(SHA_CTX *c): SHA1_Init() 是一个初始化参数，它用来初始化一个 SHA_CTX 结构，该结构存放弄了生成 SHA1 散列值的一些参数，在应用中可以不用关系该结构的内容。


int SHA1_Update(SHA_CTX *c, const void *data, unsigned long len): SHA1_Update() 函数正是可以处理大文件的关键。它可以反复调用，比如说我们要计算一个 5G 文件的散列值，我们可以将该文件分割成多个小的数据块，对每个数据块分别调用一次该函数，这样在最后就能够应用 SHA1_Final() 函数正确计算出这个大文件的 sha1 散列值。


int SHA1_Final(unsigned char *md, SHA_CTX *c)

### struct stat结构体：保存文件状态信息的结构体
```c
struct stat 
{ 
   dev_t     st_dev;     /* 文件所在设备的标识  */ 
   ino_t     st_ino;     /* 文件结点号  */ 
   mode_t    st_mode;    /* 文件保护模式  */ 
   nlink_t   st_nlink;   /* 硬连接数  */ 
   uid_t     st_uid;     /* 文件用户标识  */ 
   gid_t     st_gid;     /* 文件用户组标识  */ 
   dev_t     st_rdev;    /* 文件所表示的特殊设备文件的设备标识  */ 
   off_t     st_size;    /* 总大小，字节为单位  */ 
   blksize_t st_blksize; /* 文件系统的块大小  */ 
   blkcnt_t  st_blocks;  /* 分配给文件的块的数量，512字节为单元  */ 
   time_t    st_atime;   /* 最后访问时间  */ 
   time_t    st_mtime;   /* 最后修改时间  */ 
   time_t    st_ctime;   /* 最后状态改变时间  */ 
}; 
```

###笔记
* argc存储变量数量，argv存储变量内容。在本项目中，argc为终端输入字符串的数目，argv为终端输入的每个字符串（不同的字符串用空格隔开）

* 错误处理：undefined reference to
这种错误指向所涉及的函数未能找到具体的实现。

未能找到具体实现的原因是多方面的。就本例而言，虽然程序中声明了zlib库和openssl/sha库，但是和stdio.h不同，这两个库在头文件中并没有对函数进行实现，而仅仅只是对相应的函数进行了声明，因此导致缺少相应函数的实现

解决的方法是在链接的步骤中加入相应的参数，告知连接器需要对相应的源文件进行链接

```
gcc command.c init-db.c read-cache.c update-cache.c cat-file.c -o git -lz -lcrypto
```
其中-lz对应zlib，-lcrypto对应openssl/sha
