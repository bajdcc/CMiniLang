# CMiniLang（可自举的C编译器）

在[CParser](https://github.com/bajdcc/CParser)的基础上，改进一些功能。

## 介绍

使用C++14，以及CMake使代码可以跨平台编译（因此舍弃VS）。

更改了CParser项目中的诸多bug。

代码参考[write-a-C-interpreter](https://github.com/lotabout/write-a-C-interpreter)。

本项目中的Lexer由我自己编写，参考了[CEval](https://github.com/bajdcc/CEval)中的部分代码。Parser和VM暂时是使用**write-a-C-interpreter**项目中的代码，自举文件**xc.txt**也是。

特性：

- 手动LL分析
- 生成抽象语法树（结点扁平化、POD，结点由**内存池**提供，无需考虑析构、引用计数、结点多态等问题，内存池由自己实现）
- 根据AST生成指令（带简单的静态类型分析）
- 虚拟机中的任意地址由VMM（软件实现虚页机制）提供转换，与物理内存隔离
- 虚拟机中的MALLOC指令由软件实现（用内存池）

后期：

- 改善虚拟机指令，使之与VMM兼容
- 改善自举代码

----

## 主要功能

1. 解析C文件（完成）
2. 生成抽象语法树（完成）
3. 构造指令集（待完善）
4. 建立虚拟机（待完善）

## 进度

1. 词法分析（LL手写识别，比regex库高效）
   1. 识别数字（科学计数+十六进制）
   2. 识别变量名
   3. 识别空白字符
   4. 识别字符（支持所有转义）
   5. 识别字符串（支持所有转义）
   6. 识别注释
   7. 识别关键字
   8. 识别操作符
   9. 错误处理（快速失败）
2. 语法分析并生成AST
   1. 识别函数
   2. 识别枚举
   3. 识别表达式
   4. 识别基本结构
   5. 生成AST（完成**AST打印功能**）
3. 语义分析并生成中间代码
   1. 语义分析
   2. 生成代码（针对多数AST，待完善）
4. 虚拟机
   1. 实现虚页（已实现，分代码段，数据段，栈，堆）
   2. 实现MALLOC（已实现，参考[CLib::memory.h](https://github.com/bajdcc/learnstl/blob/master/code/02/memory.h)）
   3. 统一系统调用（计划中）
   4. 构建标准库（常用数据结构，计划中）

## 使用

先用CMake进行编译，然后操作：`CMiniLang xc.txt xc.txt test.txt`，注意文件在code文件夹中。
   
## 截图

### 词法分析

![](https://pic4.zhimg.com/v2-12fcbe73a8340d20a9488ae0228ff11f.png)

### 解释器

![](https://pic1.zhimg.com/v2-855b2e604a19e44a9f0f52e2a0eca010_r.png)

## 参考

1. [write-a-C-interpreter](https://github.com/lotabout/write-a-C-interpreter)
