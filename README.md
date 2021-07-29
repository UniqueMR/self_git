self_git
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
