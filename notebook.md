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

###笔记
* argc存储变量数量，argv存储变量内容。在本项目中，argc为终端输入字符串的数目，argv为终端输入的每个字符串（不同的字符串用空格隔开）
