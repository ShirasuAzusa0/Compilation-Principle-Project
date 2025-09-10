
  
   1.算术表达式转换为四元组

      exp2quadruples.cpp   源程序
      exp2quadruples.exe   可执行程序
      exp2quadruples.txt   测试数据文件



在程序编译时，提示函数不安全，编译器设置方法

 在ProjectProperties-> C / C ++-> Preprocessor-> Preprocessor Definitions中，添加：

  _CRT_SECURE_NO_DEPRECATE



