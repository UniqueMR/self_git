self_git


* 项目环境说明：


该工程运行于Linux环境，也可运行于windows下的linux子系统WSL，在Linux命令行输入指令可实现相应功能

* 命令行使用说明：


./git --init: 初始化git仓库，创建git的工作目录

./git --add <filename>: 添加文件或目录到暂存区

./git  --rm <filename>: 将特定文件从暂存区中删除

./git  --status <filename>: 查看特定文件是否已加入暂存区

./git --cat-file <sha1>: 查看objects文件内容

./git --diff: 查看工作区和暂存区中的文件差异

./git --commit <parent sha1> < changelog: 将本地变更提交到版本库中

./git --read-tree <sha1>: 读取并解析指定sha1值的tree对象，输出变更文件的信息 

* 开发日志：
  
7月23日：
1. 环境配置：安装linux发行版、git和gcc
2. 源码研究：拷贝git源码（0.99）,编译并运行
3. 命令行解析工具: 实现从Linux Terminal读取指令

7月24日：
1. 开发init命令，并实现init命令的命令行解析
2. 进一步学习git的原理
3. 尝试开发add命令

7月25日：
1. 尝试read-cache和update-cache的实现
2. 结合命令操作理解git原理，探究object中的文件类型及内容
3. 研究git源码，探究C中存储文件的stat对象

7月26日：

7月27日：
1. 成功实现update-cache，对应add命令，将workspace文件经过zlib压缩算法变换得到的sha1哈希值存储到index中
2. 成功实现cat-file，查看对应文件的类型以及sha1值
3. 将cat-file与update-cache功能整合进命令行，实现与用户的交互
4. 进一步理解了C程序从源代码到可执行文件的过程

7月28日：
1. 成功实现show-diff，对应diff命令，将workspace中的文件状态和index中的文件状态进行比较并显示
2. 成功实现write-tree，将记录当前blob的tree文件写入cache中
3. 尝试实现commit-tree，将记录当前版本的commit文件写入cache中
4. 成功用makefile进行多文件编译
5. 进一步理解了利用zlib进行压缩和解压的操作流程

7月29日：
1. 成功实现commit-tree
2. 基本实现git的主要功能
3. 进一步理解unix系统中的文件格式，以及C语言处理文件的方式
4. 进一步理解sha1数据类型，以及sha1与16进制相互转化的方法

7月30日：

7月31日：

8月1日：

8月2日：
1. commim增添加入parent功能
2. 增加status功能，查看指定文件是否加入暂存区
3. 进一步了解index文件及其所发挥的功能
4. 深入学习C语言内存管理，对C语言内存资源中堆和栈的区别有更加清醒的认识，理解了malloc与创建数组在内存分配上的不同

8月3日：
1. 增加rm功能，从index中删除指定文件
