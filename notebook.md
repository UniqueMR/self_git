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

### z_stream结构体：
* 属于zlib库
* 结构体定义
```c
typedef struct z_stream_s {
   Bytef   *next_in;//流的输入
   uInt    avail_in;//当avail_in下降到0时，更新avail_in和next_in 
   uLong   total_in; 

   Bytef   *next_out;//流的输出
   uInt    avail_out;//当avail_out下降到0时，更新next_out
   uLong   total_out;

   char    *msg;     
   struct internal_state FAR *state;

   //在调用初始化函数前，应用程序先初始化zalloc，zfree和opaque，其他字段都会被压缩库赋值，不会被应用程序更新
   alloc_func zalloc; 
   free_func zfree;  
   voidpf    opaque; 

   int    data_type; 
   uLong  adler;     
   uLong  reserved;  
} z_stream; 
```
* 工作原理

输入到 Zlib :: ZStream 实例中的数据临时存储到输入缓冲区的末尾，然后从缓冲区的开始处理输入缓冲区中的数据，直到不再有来自该流的输出产生为止（即，直到 #avail_out> 0处理后）。在处理期间，输出缓冲区被自动分配和扩展以保存所有输出数据。

### C语言文件处理 常见的C文件IO
* 文件描述符
	在UNIX系统种，一切皆文件，对于每个打开的（注意是打开的）文件都有一个对应的非负整数作为其文件描述符。内核（kernel）利用文件描述符（file descriptor）来访问文件。文件描述符是非负整数。打开现存文件或新建文件时，内核会返回一个文件描述符。读写文件也需要使用文件描述符来指定待读写的文件。
	
	其中标准输入（可理解为你在终端中输入的内容），标准输出（可理解为往终端上输出），标准错误（用来输出错误信息）分别对应0， 1， 2的默认文件描述符。所以再有其他打开的文件时，描述符会从未被使用的最小整数开始（从3开始）。
	
	不同的文件描述符也会指向同一个文件。相同的文件可以被不同的进程打开也可以在同一个进程中被多次打开
	
	文件描述符在形式上是一个非负整数。实际上，它是一个索引值，指向内核为每一个进程所维护的该进程打开文件的记录表。当程序打开一个现有文件或者创建一个新文件时，内核向进程返回一个文件描述符。
* open
```c
int open(const char *path, int oflag, ...mode);//oflag设定对文件操作权限的标志位
```
​ O_RDONLY：只读方式打开文件。
​ O_WRONLY：可写方式打开文件。
​ O_RDWR： 读写方式打开文件。
 O_CREAT：如果该文件不存在，就创建一个新的文件，并用第三个参数为其设置权限。
​ O_TRUNC：如文件已经存在，那么打开文件时将文件长度截取为0（即达到清空文件的目的）
​ O_APPEND：以添加方式打开文件，所有对文件的写操作都在文件的末尾进行（可理解为打开文件时将光标放在末尾）。

* close
fd为文件描述符，close用于关闭文件描述符所对应的文件
```c
int close(int fd);
```

* read
用于从打开文件读取数据
```c
ssize_t read(int fd, void *buf, size_t nbytes);//fd为已打开文件的描述符，buf用于存储从fd文件中读取到的数据,nbytes为读取的大小
```
ssize_t 为有符号整型

* write
向打开的文件写数据
```c
ssize_t write(int fd, const void * buf, size_t nbytes);//fd为文件的描述符，buf存放将要写入文件的数据，nbytes为要求写入的字数，返回值为已经写入的字数，出错则返回-1
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

* void和void*
void 不能用来声明变量，只能用于对函数返回类型的限定和对函数参数限定

void* 为不确定类型的指针，可以用来声明指针，接受任何类型的赋值。同时可以赋值给任何类型的变量，但是需要进行强制类型转换
```c
    int * a = NULL ；
    void * b ；
    a =（int *）b；
```

* mmap函数
属于sys/mman.h，实现内存映射，将普通文件映射到内存中

void *mmap(void *start,size_t length,int prot,int flags,int fd,off_t offsize);

参数start：指向欲映射的内存起始地址，通常设为 NULL，代表让系统自动选定地址，映射成功后返回该地址。

参数length：代表将文件中多大的部分映射到内存。

参数prot：映射区域的保护方式。可以为以下几种方式的组合：
	
	PROT_EXEC       映射区域可被执行
	PROT_READ       映射区域可被读取
	PROT_WRITE      映射区域可被写入
	PROT_NONE       映射区域不能存取
	参数flags：影响映射区域的各种特性。在调用mmap()时必须要指定MAP_SHARED 或MAP_PRIVATE。
	
* 栈和堆的区别
栈区(stack)：由编译器自动分配释放，存放函数的参数值，局部变量的值。操作方式类似于数据结构中的栈。
堆区(heap): 一般由程序员分配释放，若程序员不释放，程序结束时可能由OS回收。
全局区(static)：全局变量和静态变量的存储是放在一起的，初始化的全局变量和静态变量在同一区域，未初始化的全局变量和未初始化的静态变量在相邻的另一块区域
文字常量区：常量字符串存放在此
程序代码区：存放函数体的二进制代码

eg. void function(type arg,char* result)
在函数内部malloc分配空间与创建数组的区别：

在函数内部创建的变量均在栈上分配空间，当函数执行结束返回时，栈空间会被自动释放。在函数内部创建数组就属于这种情况，因此不能在函数间传递。

利用malloc分配空间在堆区，不会被自动释放，利于数组在函数间的传递

* 变量可见域

在函数内部定义的变量，当程序执行到它的定义处时，编译器为它在栈上分配空间，函数在栈上分配空间在此函数执行结束的时候就会释放掉

如果想将函数中此变量的值保存至下一次调用，最容易想到的方法是定义全局变量

* 静态数据的存储
全局（静态存储区）分为DATA段和BSS段。DATA段（全局初始化区）存放初始化的全局变量和静态变量；BSS段（全局未初始化区）存放未初始化的全局变量和静态变量，程序运行结束时自动释放。其中BSS段在程序执行前会被系统自动清零，所以未初始化的全局变量和静态变量在程序执行前已经为0

static的内部实现机制：静态数据成员要在程序一开始运行时就必须存在，不能在任何函数内分配空间和初始化。

static被引入以告知编译器，将变量存储在程序的静态存储区而非栈上空间，静态数据成员按定义出现的先后顺序依次初始化

注意静态成员嵌套时，要保证所有嵌套成员已经被初始化了

消除时的顺序是初始化的反顺序

* 项目的函数依赖关系
write_tree: write_sha1_file

commit_tree: write_sha1_file

write_sha1_file: write_sha1_buffer

index_fd: write_sha1_buffer

show_diff: show_differences read_sha1_file

cat-file: read_sha1_file
