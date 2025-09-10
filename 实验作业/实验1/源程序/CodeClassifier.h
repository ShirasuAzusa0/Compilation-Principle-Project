#include "pch.h"
#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;
#pragma once

class CodeClassifier
{
private:
	// type用于指定当先选择的编程语言，便于以后的其他语言单词分类器的拓展开发
	string type;
	// 私有函数，用于Rust单词分类
	string Rust_Classify(string code);
public:
	// 无参构造函数
	CodeClassifier() { type = "NULL"; }
	// 设置选取的编程语言
	void SetType(string t) { type = t; }
	// 获取当前选取的编程语言
	string GetType() { return type; }
	// 单词分类函数（外部入口）
	string Classify(string code);
	// 析构函数
	~CodeClassifier() {}
};

