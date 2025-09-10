#include <iostream>
#include <fstream>
#include <string>
#include <stack>
#include <queue>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <gdiplus.h> 
using namespace std;
#pragma once

// NFA图的节点
struct Node
{
	string nodename;			// 节点名称
};

// NFA图的边
struct Edge
{
	string startname;		// 边的起始节点名称
	string endname;			// 边的结束节点名称
	string tranSymbol;		// 边的转换条件符号
};

// NFA图的组成单元，一个大NFA单元可以由多个小NFA单元拼接组合而来
struct Elem
{
	int edgeCount;			// NFA单元的边数
	vector<Edge> edgeSet;	// NFA单元的边的集合
	string startname;		// NFA单元的起始节点名
	string endname;			// NFA单元的结束节点名
};

// DFA图的状态
struct DFAState
{
	set<string> nfaStates;	// DFA图的状态集合（将从某节点通过ε到达的节点）
	string statename;
};

// DFA图的转换关系
struct DFATransition
{
	DFAState fromstate;
	DFAState tostate;
	string transitionsymbol;
};

class DFAbuilder
{
private:
	int nodenum;
public:
	// 无参构造函数
	DFAbuilder() { nodenum = 0; }
	// 处理[...]->...|...的转换
	string opChange(string re);
	// 隐性连接显性化函数
	string addConcatenation(string re, vector<string> subre_name);
	// 中缀转后缀表达式
	string InfixtoPostfix(string processed);
	
	// 组成单元拷贝函数
	void elem_copy(Elem& dest, Elem source);
	// 创建新节点
	Node new_node();
	// 处理普通字符
	Elem act_Elem(string c);
	// 处理或运算符
	Elem act_Unit(Elem fir, Elem sec);
	// 处理连接运算符
	Elem act_Join(Elem fir, Elem sec);
	// 处理闭包运算符
	Elem act_Closure(Elem elem);
	// 处理正闭包运算符
	Elem act_pClosure(Elem elem);
	// 处理可选运算符
	Elem act_Choosable(Elem fir, Elem sec);

	// 检查DFA状态是否在状态集合中，即dfastates中是否找到targetstate
	bool isDFAStateInVector(const vector<DFAState>& dfastates, const DFAState& targetstate);
	// 检查转换边是否在边集合中，如a->b是否已在集合中
	bool isTransitionInVector(DFAState dfastate, DFAState dfanextstate, string symbol, vector<DFATransition> dfatransitions);
	// 计算NFA状态的ε闭包
	DFAState eClosure(const set<string>& nfastates, Elem nfa);
	// move函数，用于计算DFA状态的转移
	DFAState move(const DFAState& dfastate, string transitionsymbol, Elem nfa);

	// 根据正则表达式构造NFA图
	Elem buildNFA(string postfix, vector<string> subre_name);
	// 根据NFA图构建DFA图
	void buildDFA(Elem& NFA_elem, vector<DFAState>& dfaStates, vector<DFATransition>& dfaTransitions);
	// DFA最小化
	static void minimizeDFA(vector<string>& endstates, vector<string>& notendstates, vector<DFATransition>& dfaTransitions);
	// 析构函数
	~DFAbuilder() {}
};

// 创建NFA的Dot文件并绘制成png图
void generateDotFile_NFA(const Elem& nfa, string filename);
// 创建DFA的Dot文件并绘制成png图
void generateDotFile_DFA(vector<string>& endstates, vector<DFAState>& dfaStates, vector<DFATransition>& dfaTransitions, const Elem& nfa, string filename);
// 生成词法分析函数
string code_generate(string token_name, vector<string>& endstates, vector<DFATransition>& dfaTransitions);

string regexReplace(vector<string>& fromparts, vector<string> backparts, string regex);