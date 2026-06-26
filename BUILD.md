# Build

如何编译MineSweeper Run？

首先下载源代码，控制台版源代码为.c单文件，

ege版为.cpp.c单文件，自行去掉.c，成为.cpp单文件。

## 小熊猫C++

直接点击工具栏编译即可

作者使用小熊猫C++ 3.2携带的MinGW GCC 11.4，EGE版本为24.11。

如使用最新小熊猫C++ 3.4的MinGW GCC 11.5，EGE版本为25.11，编译ege版，编译体积会更大，且没有什么帮助。

## Visual Studio

vs编译，你可能必须执行以下步骤：

将源代码文件保存为GBK或GB2312格式（小熊猫C++右下角状态栏，或vs高级保存选项）

关闭SDL检查（不要告诉我你用vs不会这个）

在约640行，SetConsoleTitle()内字符串前加L，使用宽字符串

ege版：头部取消注释#define SHOW_CONSOLE
