#include "pch.h"
#include "DFAbuilder.h"

// 多目符号 + 转义符（当操作数处理）
vector<string> symbol = {
	"\\+", "\\-", "\\*", "\\/", "\\?", "\\|", "\\.", "\\(", "\\)", "\\{", "\\}",
	"&&", "||", "<<", ">>", "+=", "-=", "*=", "/=", "%=", "^=",
	"&=", "|=", "<<=", ">>=", "=", "==", "!=", ">=", "<=", "->"
};

// 运算优先级表
map<char, int> op_priority = {
	{'*', 5},{'+', 5},				// 闭包（闭包、正闭包)
	{'?', 4},						// 可选运算符
	{'.', 3},						// 显式连接符
	{'|', 1},						// 选择符
	{'(', 0}						// 括号特殊处理
};

string DFAbuilder::opChange(string re)
{
	map<char, char> tmp;
	string processed = "";
	int i = 0, j = 0;
	while (i < re.size())
	{
		if (re[i] == '[')
		{
			j = i;
			while (re[j] != ']' && j < re.size()) ++j;
			string sub_re = re.substr(i, j - i + 1);
			for (int k = 0; k < sub_re.size(); ++k)
			{
				if (sub_re[k] == '-' && k > 1 && k < sub_re.size() - 2)
					tmp.insert({ sub_re[k - 1], sub_re[k + 1] });
			}
			if (!tmp.empty())
			{
				for (auto it : tmp)
				{
					string p = "(";
					p += it.first;
					for (int m = 0; m < int(it.second) - int(it.first); ++m)
						p += '|' + string(1, char(it.first + m + 1));
					processed += p + ')';
				}
				i = j + 1;
			}
			else
			{
				string p = "(" + string(1, re[i + 1]);
				for (int n = i + 2; n < j; ++n)
					p += '|' + string(1, re[n]);
				processed += p + ')';
				i = j + 1;
			}
		}
		if(i < re.size()) processed += re[i];
		++i;
	}
	return processed;
}

string DFAbuilder::addConcatenation(string re, vector<string> subre_name)
{
	string processed;
	// 转义状态跟踪
	bool escaped = false;
	
	map<int, int> sub_pos;
	if (subre_name.size() > 1)
	{
		for (string it : subre_name)
		{
			int last = 0;
			string tmp = re;
			while (tmp.find(it) != string::npos)
			{
				sub_pos.insert({ last + tmp.find(it), it.size() });
				last = tmp.find(it) + it.size();
				tmp = tmp.substr(last);
			}
		}
	}

	size_t i = 0;

	while ( i < re.size())
	{
		bool subname_flag = false;
		auto it = sub_pos.find(i);
		if (it != sub_pos.end())
		{
			subname_flag = true;
			string sub_re = re.substr(it->first, it->second);
			processed += sub_re;
			i += it->second;
			--i;
		}
		
		if(!subname_flag) processed += re[i];
		if (i + 1 < re.size())
		{
			// 判断当前字符是否为转义符
			bool is_escape = (re[i] == '\\');

			// 处理转义符逻辑
			if (re[i] == '\\' && !escaped)
			{
				escaped = true;
				++i;
				continue;
			}

			if (!escaped && i + 1 < re.size())
			{
				bool need_concat = false;

				// 左侧合法条件（扩展运算符范围）
				bool left_valid = (isalnum(re[i]) || re[i] == ')'
					|| re[i] == '*' || re[i] == '+'
					|| re[i] == ']' || re[i] == '{');

				// 右侧合法条件（支持更多运算符）
				bool right_valid = (isalnum(re[i + 1]) || re[i + 1] == '('
					|| re[i + 1] == '[' || re[i + 1] == '\\'
					|| re[i + 1] == '{');

				// 多字符运算符保护
				string part = string(1, re[i]) + string(1, re[i + 1]);
				if (find(symbol.begin(), symbol.end(), part) != symbol.end())
					need_concat = false;
				else
					need_concat = left_valid && right_valid;

				if (need_concat) processed += '.';
			}
		}
		escaped = false;
		++i;
	}
	return processed;
}

// 中缀转后缀表达式
string DFAbuilder::InfixtoPostfix(string processed)
{
	string postfix, part = "";
	stack<char> op_stack;
	size_t i = 0;
	while (i < processed.size())
	{
		char c = processed[i];
		if (isalnum(c))
		{
			postfix += c;											// 处理操作数（直接输出）
			++i;
		}
		else if (c == '\\')
		{
			if (i + 1 < processed.size())
			{
				char escaped_char = processed[i + 1];
				postfix += string(1, c) + string(1, escaped_char);
				i += 2;
			}
		}
		else if (c == '(')
		{
			op_stack.push(c);						// 处理左括号（入栈）
			++i;
		}
		else if (c == ')')											// 处理右括号（部分出栈）
		{
			while (!op_stack.empty() && op_stack.top() != '(')
			{
				postfix += op_stack.top();
				op_stack.pop();
			}
			op_stack.pop();
			++i;
		}
		else if(c == '*' || c == '?' || c == '+' || c == '.' || c == '|')
		{
			while (!op_stack.empty() && op_priority[c] <= op_priority[op_stack.top()])
			{
				postfix += op_stack.top();
				op_stack.pop();
			}
			op_stack.push(c);
			++i;
		}
		else
		{
			part = c + string(1, processed[i + 1]) + string(1, processed[i + 2]);
			if (find(symbol.begin(), symbol.end(), part) != symbol.end())
			{
				postfix += part;
				i += 3;
			}
			else
			{
				part = c + string(1, processed[i + 1]);
				if (find(symbol.begin(), symbol.end(), part) != symbol.end())
				{
					postfix += part;
					i += 2;
				}
				else
				{
					postfix += c;
					++i;
				}
			}
			part = "";
		}
	}
	while (!op_stack.empty())										// 处理栈内剩余的运算符
	{
		postfix += op_stack.top();
		op_stack.pop();
	}
	return postfix;
}

Node DFAbuilder::new_node()
{
	Node newNode;
	newNode.nodename = to_string(++nodenum);
	return newNode;
}

void DFAbuilder::elem_copy(Elem& dest, Elem source)
{
	for (int i = 0; i < source.edgeCount; ++i)
		dest.edgeSet.push_back(source.edgeSet[i]);
	dest.edgeCount += source.edgeCount;
}

// 处理普通字符
Elem DFAbuilder::act_Elem(string c)
{
	// 创建新节点（对应新的初态和终态）
	Node startnode = new_node();
	Node endnode = new_node();

	// 创建新边
	Edge newedge;
	newedge.startname = startnode.nodename;
	newedge.endname = endnode.nodename;
	newedge.tranSymbol = c;

	// 新NFA组成元素
	Elem newelem;
	newelem.edgeCount = 1;
	newelem.edgeSet.push_back(newedge);
	newelem.startname = newelem.edgeSet[0].startname;
	newelem.endname = newelem.edgeSet[0].endname;
	
	return newelem;
}

// 处理或运算
Elem DFAbuilder::act_Unit(Elem fir, Elem sec)
{
	Elem newelem;
	newelem.edgeCount = 0;
	Edge edge1, edge2, edge3, edge4;

	// 获得新的状态节点
	Node startnode = new_node();
	Node endnode = new_node();

	// 构建e1（连接起点和AB的起始点A）
	edge1.startname = startnode.nodename;
	edge1.endname = fir.startname;
	edge1.tranSymbol = "#";

	// 构建e2（连接起点和AB的起始点A）
	edge2.startname = startnode.nodename;
	edge2.endname = sec.startname;
	edge2.tranSymbol = "#";

	// 构建e3（连接起点和AB的起始点A）
	edge3.startname = fir.endname;
	edge3.endname = endnode.nodename;
	edge3.tranSymbol = "#";

	// 构建e4（连接起点和AB的起始点A）
	edge4.startname = sec.endname;
	edge4.endname = endnode.nodename;
	edge4.tranSymbol = "#";

	// 合并fir和sec
	elem_copy(newelem, fir);
	elem_copy(newelem, sec);

	// 新构建四条边
	newelem.edgeSet.push_back(edge1);
	newelem.edgeSet.push_back(edge2);
	newelem.edgeSet.push_back(edge3);
	newelem.edgeSet.push_back(edge4);

	newelem.edgeCount += 4;
	newelem.startname = startnode.nodename;
	newelem.endname = endnode.nodename;

	return newelem;
}

// 处理N(s)N(t)连接运算
Elem DFAbuilder::act_Join(Elem fir, Elem sec)
{
	// 将fir的结束状态和sec的开始状态合并，将sec的边复制给fir，将fir返回
	// 将sec总所有以StartState开头的边全部修改
	/*
	for (int i = 0; i < sec.edgeCount; ++i)
	{
		if (sec.edgeSet[i].startname == sec.startname)
			sec.edgeSet[i].startname = fir.endname;		// 该边e1的开始状态是N(t)的开始状态
		else if (sec.edgeSet[i].endname == sec.endname)
			sec.edgeSet[i].endname = fir.endname;		// 该边e2的结束状态是N(t)的起始状态
	}
	sec.startname = fir.endname;
	elem_copy(fir, sec);

	// 将fir的结束状态更新为sec的结束状态
	fir.endname = sec.endname;
	*/
	Edge edge;
	edge.startname = fir.endname;
	edge.endname = sec.startname;
	edge.tranSymbol = "#";
	fir.edgeCount++;
	fir.edgeSet.push_back(edge);
	elem_copy(fir, sec);
	fir.endname = sec.endname;
	return fir;
}

// 处理闭包运算
Elem DFAbuilder::act_Closure(Elem elem)
{
	Elem newelem;
	newelem.edgeCount = 0;
	Edge edge1, edge2, edge3, edge4;

	// 获得新状态节点
	Node startnode = new_node();
	Node endnode = new_node();

	// e1
	edge1.startname = startnode.nodename;
	edge1.endname = endnode.nodename;
	edge1.tranSymbol = "#";

	// e2
	edge2.startname = elem.endname;
	edge2.endname = elem.startname;
	edge2.tranSymbol = "#";

	// e3
	edge3.startname = startnode.nodename;
	edge3.endname = elem.startname;
	edge3.tranSymbol = "#";

	// e4
	edge4.startname = elem.endname;
	edge4.endname = endnode.nodename;
	edge4.tranSymbol = "#";

	// 构建单元
	elem_copy(newelem, elem);

	// 将新构建的四条边加入edgeSet
	newelem.edgeSet.push_back(edge1);
	newelem.edgeSet.push_back(edge2);
	newelem.edgeSet.push_back(edge3);
	newelem.edgeSet.push_back(edge4);

	newelem.edgeCount += 4;
	// 构建newelem的起始状态和结束状态
	newelem.startname = startnode.nodename;
	newelem.endname = endnode.nodename;

	return newelem;
}

// 处理正闭包运算
Elem DFAbuilder::act_pClosure(Elem elem)
{
	Elem newelem;
	newelem.edgeCount = 0;
	Edge edge1, edge2, edge3;

	// 获得新状态节点
	Node startnode = new_node();
	Node endnode = new_node();

	// e1
	edge1.startname = elem.endname;
	edge1.endname = endnode.nodename;
	edge1.tranSymbol = "#";

	// e2
	edge2.startname = elem.endname;
	edge2.endname = elem.startname;
	edge2.tranSymbol = "#";

	// e3
	edge3.startname = startnode.nodename;
	edge3.endname = elem.startname;
	edge3.tranSymbol = "#";

	// 构建单元
	elem_copy(newelem, elem);

	// 将新构建的三条边加入edgeSet
	newelem.edgeSet.push_back(edge1);
	newelem.edgeSet.push_back(edge2);
	newelem.edgeSet.push_back(edge3);

	newelem.edgeCount += 3;
	// 构建newelem的起始状态和结束状态
	newelem.startname = startnode.nodename;
	newelem.endname = endnode.nodename;

	return newelem;

}

// 处理可选运算
Elem DFAbuilder::act_Choosable(Elem fir, Elem sec)
{
	Edge edge1, edge2;
	edge1.startname = fir.endname;
	edge1.endname = sec.startname;
	edge1.tranSymbol = "#";
	edge2.startname = fir.startname;
	edge2.endname = fir.endname;
	edge2.tranSymbol = "#";
	fir.edgeCount+=2;
	fir.edgeSet.push_back(edge1);
	fir.edgeSet.push_back(edge2);
	elem_copy(fir, sec);
	fir.endname = sec.endname;
	return fir;
}


Elem DFAbuilder::buildNFA(string postfix, vector<string> subre_name)
{
	stack<Elem> elem_st;
	Elem elem, fir, sec;
	string part = "";
	char c;

	map<int, int> sub_pos;
	if (subre_name.size() > 1)
	{
		for (string it : subre_name)
		{
			int last = 0;
			string tmp = postfix;
			while (tmp.find(it) != string::npos)
			{
				sub_pos.insert({ last + tmp.find(it), it.size() });
				last = tmp.find(it) + it.size();
				tmp = tmp.substr(last);
			}
		}
	}

	size_t i = 0;
	while(i < postfix.size())
	{
		part = "";
		c = postfix[i];
		
		auto it = sub_pos.find(i);
		if (it != sub_pos.end())
		{
			string sub_posfix = postfix.substr(it->first, it->second);
			part = sub_posfix;
			i += it->second;
			--i;
		}
		else if (isalnum(c))
			part = c;
		else if (op_priority.find(c) == op_priority.end())
		{
			part = c + string(1, postfix[i + 1]) + string(1, postfix[i + 2]) + string(1, postfix[i + 3]);
			if (find(symbol.begin(), symbol.end(), part) != symbol.end())
				i += 3;
			else
			{
				part = c + string(1, postfix[i + 1]) + string(1, postfix[i + 2]);
				if (find(symbol.begin(), symbol.end(), part) != symbol.end())
					i += 2;
				else
				{
					part = c + string(1, postfix[i + 1]);
					if (find(symbol.begin(), symbol.end(), part) != symbol.end())
						++i;
					else
						part = c;
				}
			}
		}

		++i;

		switch (c) {
		case '*': 
			fir = elem_st.top();
			elem_st.pop();
			elem = act_Closure(fir);
			elem_st.push(elem);
			break;
		case '+':
			fir = elem_st.top();
			elem_st.pop();
			elem = act_pClosure(fir);
			elem_st.push(elem);
			break;
		case '?':
			sec = elem_st.top();
			elem_st.pop();
			fir = elem_st.top();
			elem_st.pop();
			elem = act_Choosable(fir, sec);
			elem_st.push(elem);
			break;
		case '|': 
			sec = elem_st.top();
			elem_st.pop();
			fir = elem_st.top();
			elem_st.pop();
			elem = act_Unit(fir, sec);
			elem_st.push(elem);
			break;
		case '.': 
			sec = elem_st.top();
			elem_st.pop();
			fir = elem_st.top();
			elem_st.pop();
			elem = act_Join(fir, sec);
			elem_st.push(elem);
			break;
		default:
			elem = act_Elem(part);
			elem_st.push(elem);
		}
	}
	elem = elem_st.top();
	elem_st.pop();
	nodenum = 0;
	return elem;
}


// 检查DFA状态是否在状态集合中，即dfastates中是否能找到targetstate
bool DFAbuilder::isDFAStateInVector(const vector<DFAState>& dfastates, const DFAState& targetstate)
{
	// for循环遍历 dfastates 中的每一个状态
	for (const DFAState& state : dfastates)
		// 若找到匹配的状态，则返回true
		if (state.statename == targetstate.statename)
			return true;
	// 若遍历完整个状态集合仍未找到匹配的状态，则返回false
	return false;
}

// 检查转换边是否在边集合中，比如a->b是否已经在边集合中
bool DFAbuilder::isTransitionInVector(DFAState dfastate, DFAState dfanextstate, string symbol, vector<DFATransition> dfatransitions)
{
	// 用for循环遍历边集合dfatransitions中的每条转换边
	for (const DFATransition& transition : dfatransitions)
		// 若找到，返回true
		if (transition.fromstate.statename == dfastate.statename && transition.tostate.statename == dfanextstate.statename && transition.transitionsymbol == symbol)
			return true;
	// 若找不到，返回false
	return false;
}

// 计算NFA状态的ε闭包，也即计算NFA图中从某一状态可通过ε转换到所有状态的集合（包括当前状态）
DFAState DFAbuilder::eClosure(const set<string>& nfastates, Elem nfa)
{
	DFAState eClosureState;
	eClosureState.nfaStates = nfastates;

	stack<string> state_st;

	// 初始化栈，将初始状态入栈，一开始nfastate中只有NFA_elem.startname
	for (const string& nfastate_name : nfastates)
		state_st.push(nfastate_name);

	while (!state_st.empty())
	{
		string cur_state = state_st.top();
		state_st.pop();

		// 遍历NFA图的边
		for (int i = 0; i < nfa.edgeCount; ++i)
		{
			Edge cur_edge = nfa.edgeSet[i];

			// 如果边的起始状态是当前状态，并且边的转换符号为“#”（也即ε转换边），则将目标状态加入ε闭包
			if (cur_edge.startname == cur_state && cur_edge.tranSymbol == "#")
				// 检查目标状态是否已经在ε闭包中，避免重复添加
				if (eClosureState.nfaStates.find(cur_edge.endname) == eClosureState.nfaStates.end())
				{
					eClosureState.nfaStates.insert(cur_edge.endname);
					state_st.push(cur_edge.endname);
				}
		}
	}

	// 为ε闭包分配一个唯一的名称
	for (const string& nfastate_name : eClosureState.nfaStates)
		eClosureState.statename += nfastate_name + ' ';

	return eClosureState;
}

// move函数，用于计算DFA状态的转移，即获取通过非空转移到的下一节点
DFAState DFAbuilder::move(const DFAState& dfastate, string transitionsymbol, Elem nfa)
{
	DFAState nextstate;

	// 遍历DFAState中的每一个NFA状态
	for (const string& nfastate_name : (dfastate.nfaStates))
		// 在这里遍历所有NFA状态的边
		for (int i = 0; i < nfa.edgeCount; ++i)
		{
			Edge cur_edge = nfa.edgeSet[i];

			// 若边的起始状态就是当前状态，且边的转换符号等于输入符号，则将目标状态加入nextstate
			if (cur_edge.startname == nfastate_name && cur_edge.tranSymbol == transitionsymbol && cur_edge.tranSymbol != "#")
				nextstate.nfaStates.insert(cur_edge.endname);
		}

	// 为nextstate分配一个唯一的名称
	for (const string& nfastate_name : nextstate.nfaStates)
		nextstate.statename += nfastate_name + ' ';

	return nextstate;
}



void DFAbuilder::buildDFA(Elem& NFA_elem, vector<DFAState>& dfaStates, vector<DFATransition>& dfaTransitions)
{
	// 初始化DFA状态集合和转换关系
	set<string> nfa_Initial_StateSet;
	nfa_Initial_StateSet.insert(NFA_elem.startname);
	// 计算NFA初始状态的ε闭包,得到DFA图的第一个节点（起点）
	DFAState dfa_Initial_State = eClosure(nfa_Initial_StateSet, NFA_elem);
	dfaStates.push_back(dfa_Initial_State);

	// 开始构建DFA
	for (size_t i = 0; i < dfaStates.size(); ++i)
	{
		DFAState dfastate = dfaStates[i];
		for (int j = 0; j < NFA_elem.edgeCount; ++j)
		{
			string symbol = NFA_elem.edgeSet[j].tranSymbol;
			DFAState nextstate = move(dfastate, symbol, NFA_elem);
			// 计算move操作后的ε闭包
			DFAState dfanextstate = eClosure(nextstate.nfaStates, NFA_elem);

			if (!nextstate.nfaStates.empty())
			{
				// 若下一状态不为空，且在DFA状态集合中还未添加，则加入DFA状态集合
				if (!isDFAStateInVector(dfaStates, dfanextstate))
					dfaStates.push_back(dfanextstate);
				// 对于边也要去重，因为等于a的边可能会遍历到两次
				// 若当前边在DFA转换关系中还未添加，则加DFA转换关系
				if (!isTransitionInVector(dfastate, dfanextstate, symbol, dfaTransitions))
					dfaTransitions.push_back({ dfastate, dfanextstate, symbol });
			}
		}
	}
}


void DFAbuilder::minimizeDFA(vector<string>& endstates, vector<string>& notendstates, vector<DFATransition>& dfaTransitions)
{
	// 初始化分区（接受状态组与非接受状态组）
	vector<set<string>> partitions;
	// 结束状态组
	partitions.push_back(set<string>(endstates.begin(), endstates.end()));
	// 非结束状态组
	partitions.push_back(set<string>(notendstates.begin(), notendstates.end()));

	// 记录状态所属的分区编号
	unordered_map<string, int> statePartition;
	for (const auto& state : endstates) statePartition[state] = 0;
	for (const auto& state : notendstates) statePartition[state] = 1;

	// 构建状态转移表（格式为：源状态——转移条件——>目标状态）
	// 将DFA的转移关系转换为双层哈希表，快速查询任意状态在特定转移条件下的转移目标
	unordered_map<string, unordered_map<string, string>> transitionTable;
	for (const auto& trans : dfaTransitions)
		transitionTable[trans.fromstate.statename][trans.transitionsymbol] = trans.tostate.statename;

	// Hopcroft算法迭代划分
	// 通过队列管理待处理的分区，初始处理所有非接受状态和接受状态，push进去的为分区序号
	queue<int> processQueue;
	processQueue.push(0);				// 接受状态分区
	processQueue.push(1);				// 非接受状态分区

	while (!processQueue.empty())
	{
		// 获取当前分区序号，确认当前处理的分区
		int currentPart = processQueue.front();
		processQueue.pop();

		// 获取当前分区的所有符号
		// 通过符号集合，收集当前分区中所有状态的所有输入符号（转换条件），用于后续划分
		set<string> symbols;
		for (const auto& state : partitions[currentPart])
			for (const auto& item : transitionTable[state])
				symbols.insert(item.first);

		// 对每个符号进行切割
		for (const auto& symbol : symbols)
		{
			// 根据符号转移后的目标分区对当前分区进行分组，若两个状态在相同符号下转移到不同分区，则需要分割新的分区
			unordered_map<int, set<string>> splitMap;

			// 根据符号转移后的目标分区进行分组
			for (const auto& state : partitions[currentPart])
			{
				if (transitionTable[state].count(symbol))
				{
					string target = transitionTable[state][symbol];
					int targetPart = statePartition[target];
					splitMap[targetPart].insert(state);
				}
				// splitMap[-1]表示当前状态在当前符号（转移条件）下无转移
				else splitMap[-1].insert(state);
			}

			// 如果产生新的分区，则动态更新现有分区
			// 通过不断细化分区直至无法分割，得到最终分区结果
			if (splitMap.size() > 1)
			{
				partitions.erase(partitions.begin() + currentPart);
				int newIndex = currentPart;
				for (const auto& item : splitMap)
				{
					if (newIndex == currentPart)
						partitions.insert(partitions.begin() + newIndex, item.second);
					else
					{
						partitions.push_back(item.second);
						processQueue.push(partitions.size() - 1);
					}

					// 更新状态分区映射
					for (const auto& s : item.second)
						statePartition[s] = newIndex;
					++newIndex;
				}
			}
		}
	}

	// 构建最小化后的状态和转换
	vector<string> minimizedEnd, minimizedNotEnd;
	vector<DFATransition> minimizedTrans;

	// 创建分区代表映射（选择每个分区的第一个状态作为代表）
	unordered_map<string, string> partitionRep;
	for (const auto& part : partitions)
	{
		// 处理分区为空的情况
		if (part.empty()) continue;
		string rep = *part.begin();
		for (const auto& s : part)
			partitionRep[s] = rep;

		// 分类接受/非接受状态
		if (find(endstates.begin(), endstates.end(), rep) != endstates.end())
			minimizedEnd.push_back(rep);
		else
			minimizedNotEnd.push_back(rep);
	}

	// 构建最小化转换关系
	// 合并等价状态，每个分区选择第一个状态作为代表
	set<string> processedSymbols;
	for (const auto& trans : dfaTransitions)
	{
		string fromRep = partitionRep[trans.fromstate.statename];
		string toRep = partitionRep[trans.tostate.statename];
		string symbol = trans.transitionsymbol;

		// 去重处理
		if (processedSymbols.count(fromRep + symbol + toRep)) continue;

		DFATransition tmp;
		tmp.fromstate.statename = fromRep;
		tmp.tostate.statename = toRep;
		tmp.transitionsymbol = symbol;

		minimizedTrans.push_back(tmp);
		processedSymbols.insert(fromRep + symbol + toRep);
	}

	// 去除死（孤立）状态
	minimizedEnd.erase(
		std::remove_if(minimizedEnd.begin(), minimizedEnd.end(),
			[&](const std::string& state) {
				for (auto& trans : minimizedTrans) {
					if (state == trans.fromstate.statename ||
						state == trans.tostate.statename) {
						return false;
					}
				}
				return true;
			}),
		minimizedEnd.end()
	);
			

	// 更新结果
	endstates = minimizedEnd;
	notendstates = minimizedNotEnd;
	dfaTransitions = minimizedTrans;
}


void generateDotFile_NFA(const Elem& nfa, string filename) {
	ofstream dotFile(filename + "NFAgraph.dot");

	if (dotFile.is_open()) {
		dotFile << "digraph NFA {\n";
		dotFile << "  rankdir=LR;  // 横向布局\n\n";
		dotFile << " node [shape = circle];   // 状态节点\n\n";

		dotFile << nfa.endname << " [shape=doublecircle];\n";
		// 添加 NFA 状态
		dotFile << "  " << nfa.startname << " [label=\"Start State: " << nfa.startname << "\"];\n";
		dotFile << "  " << nfa.endname << " [label=\"End State: " << nfa.endname << "\"];\n";

		// 添加 NFA 转移
		for (int i = 0; i < nfa.edgeCount; i++) {
			const Edge& currentEdge = nfa.edgeSet[i];
			dotFile << "  " << currentEdge.startname << " -> " << currentEdge.endname << " [label=\"" << currentEdge.tranSymbol << "\"];\n";
		}

		dotFile << "}\n";

		dotFile.close();
		std::cout << "NFA DOT file generated successfully.\n";
	}
	else {
		cerr << "Unable to open NFA DOT file.\n";
	}

	wstring wfilename(filename.begin(), filename.end());
	wstring dotCmd_tmp = L"..\\packages\\Graphviz.2.38.0.2\\dot.exe -Tsvg "
		+ wfilename + L"NFAgraph.dot -o " + wfilename + L"NFA.svg";

	wstring dotCmd = dotCmd_tmp;

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	bool success = CreateProcessW(
		NULL,
		&dotCmd[0], // 命令行字符串
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi
	);

	wstring dotCmd_tmp_1 = L"..\\packages\\Graphviz.2.38.0.2\\dot.exe -Tbmp "
		+ wfilename + L"NFAgraph.dot -o " + wfilename + L"NFA.bmp";

	wstring dotCmd_1 = dotCmd_tmp_1;

	STARTUPINFO si_1 = { sizeof(si_1) };
	PROCESS_INFORMATION pi_1;
	bool success_1 = CreateProcessW(
		NULL,
		&dotCmd_1[0], // 命令行字符串
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si_1,
		&pi_1
	);
}


//生成DFA的dot文件
void generateDotFile_DFA(vector<string>& endstates, vector<DFAState>& dfaStates, vector<DFATransition>& dfaTransitions, const Elem& nfa, string filename) {
	vector<string> notendstates;
	
	// 打开名为 "dfa_graph.dot" 的文件流
	ofstream dotFile(filename + "DFAgraph.dot");

	// 如果文件流成功打开
	if (dotFile.is_open()) {
		// 写入 DOT 文件的头部信息
		dotFile << "digraph DFA {\n";
		dotFile << "  rankdir=LR;  // 横向布局\n\n";
		dotFile << " node [shape = circle];   // 初始状态\n\n";
		for (DFAState dfastate : dfaStates)
			if(dfastate.statename.find(nfa.endname) != string::npos)
				dotFile << "\"" << dfastate.statename << "\" [shape = doublecircle];\n";

		// 添加DFA状态
		for (const auto& state : dfaStates) {
			dotFile << "  \"" << state.statename << "\"";
			dotFile << " [label=\"State " << state.statename;
			if (state.statename == dfaStates.front().statename) dotFile << "\\n(startState)";
			if (state.statename == dfaStates.back().statename || state.statename.find(nfa.endname) != string::npos) {
				dotFile << "\\n(endState)";
				endstates.push_back(state.statename);
			}
			else notendstates.push_back(state.statename);
			dotFile << "\"];\n";
		}
		dotFile << "\n";

		// 添加DFA转移
		for (const auto& transition : dfaTransitions) {
			dotFile << "  \"" << transition.fromstate.statename << "\" -> \"" << transition.tostate.statename << "\" [label=\"" << transition.transitionsymbol << "\"];\n";
		}

		// 写入 DOT 文件的尾部信息
		dotFile << "}\n";

		// 关闭文件流并输出成功信息
		dotFile.close();
		std::cout << "DFA DOT file generated successfully.\n";
	}
	else {
		// 输出错误信息，表示无法打开 DOT 文件
		std::cerr << "Unable to open DOT file.\n";
	}
	wstring wfilename(filename.begin(), filename.end());
	wstring dotCmd_tmp = L"\"..\\packages\\Graphviz.2.38.0.2\\dot.exe\" -Tsvg \""
		+ wfilename + L"DFAgraph.dot\" -o \"" + wfilename + L"DFA.svg\"";

	wstring dotCmd = dotCmd_tmp;

	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	bool success = CreateProcessW(
		NULL,
		&dotCmd[0], // 命令行字符串
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi
	);

	wstring dotCmd_tmp_2 = L"\"..\\packages\\Graphviz.2.38.0.2\\dot.exe\" -Tbmp \""
		+ wfilename + L"DFAgraph.dot\" -o \"" + wfilename + L"DFA.bmp\"";

	wstring dotCmd_2 = dotCmd_tmp_2;

	STARTUPINFO si_2 = { sizeof(si_2) };
	PROCESS_INFORMATION pi_2;
	bool success_2 = CreateProcessW(
		NULL,
		&dotCmd_2[0], // 命令行字符串
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si_2,
		&pi_2
	);

	
	// DFA最小化
	DFAbuilder::minimizeDFA(endstates, notendstates, dfaTransitions);
	vector<string> States;
	States.insert(States.end(), endstates.begin(), endstates.end());
	States.insert(States.end(), notendstates.begin(), notendstates.end());
	
	// 打开名为 "dfa_graph.dot" 的文件流
	ofstream dotFile2(filename + "minDFAgraph.dot");

	// 如果文件流成功打开
	if (dotFile2.is_open()) {
		// 写入 DOT 文件的头部信息
		dotFile2 << "digraph minDFA {\n";
		dotFile2 << "  rankdir=LR;  // 横向布局\n\n";
		dotFile2 << " node [shape = circle];   // 初始状态\n\n";
		for (string dfastate : States)
			if (dfastate.find(nfa.endname) != string::npos)
				dotFile2 << "\"" << dfastate << "\" [shape = doublecircle];\n";

		// 添加DFA状态
		for (const auto& state : States) {
			dotFile2 << "  \"" << state << "\"";
			dotFile2 << " [label=\"State " << state;
			if (state == dfaStates.front().statename) dotFile2 << "\\n(startState)";
			if (state == dfaStates.back().statename || state.find(nfa.endname) != string::npos) {
				dotFile2 << "\\n(endState)";
				endstates.push_back(state);
			}
			else notendstates.push_back(state);
			dotFile2 << "\"];\n";
		}
		dotFile2 << "\n";

		// 添加DFA转移
		for (const auto& transition : dfaTransitions) {
			dotFile2 << "  \"" << transition.fromstate.statename << "\" -> \"" << transition.tostate.statename << "\" [label=\"" << transition.transitionsymbol << "\"];\n";
		}

		// 写入 DOT 文件的尾部信息
		dotFile2 << "}\n";

		// 关闭文件流并输出成功信息
		dotFile2.close();
		std::cout << "DFA DOT file generated successfully.\n";
	}
	else {
		// 输出错误信息，表示无法打开 DOT 文件
		std::cerr << "Unable to open DOT file.\n";
	}
	wstring wfilename2(filename.begin(), filename.end());
	wstring dotCmd_tmp2 = L"\"..\\packages\\Graphviz.2.38.0.2\\dot.exe\" -Tsvg \""
		+ wfilename2 + L"minDFAgraph.dot\" -o \"" + wfilename2 + L"minDFA.svg\"";

	wstring dotCmd2 = dotCmd_tmp2;

	STARTUPINFO si2 = { sizeof(si2) };
	PROCESS_INFORMATION pi2;
	bool success2 = CreateProcessW(
		NULL,
		&dotCmd2[0], // 命令行字符串
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si2,
		&pi2
	);

	wstring dotCmd_tmp2_2 = L"\"..\\packages\\Graphviz.2.38.0.2\\dot.exe\" -Tbmp \""
		+ wfilename2 + L"minDFAgraph.dot\" -o \"" + wfilename2 + L"minDFA.bmp\"";

	wstring dotCmd2_2 = dotCmd_tmp2_2;

	STARTUPINFO si2_2 = { sizeof(si2_2) };
	PROCESS_INFORMATION pi2_2;
	bool success2_2 = CreateProcessW(
		NULL,
		&dotCmd2_2[0], // 命令行字符串
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si2_2,
		&pi2_2
	);
}


string regexReplace(vector<string>& fromparts, vector<string> backparts, string regex)
{
	string res = "";
	int last = 0;
	map<string, string> replacemap;
	for (size_t i = 0; i < fromparts.size() - 1; ++i)
		replacemap.insert({ fromparts[i], backparts[i] });
	
	map<int, int> sub_pos;
	for (string it : fromparts)
	{
		if (it == fromparts.back()) break;
		int last = 0;
		string tmp = regex;
		while (tmp.find(it) != string::npos)
		{
			sub_pos.insert({ last + tmp.find(it), it.size() });
			last = tmp.find(it) + it.size();
			tmp = tmp.substr(last);
		}
	}

	int pos = 0;
	bool replaceflag = false;
	while (pos < regex.size())
	{
		replaceflag = false;
		auto it = sub_pos.find(pos);
		if (it != sub_pos.end())
		{
			string sub_re = regex.substr(it->first, it->second);
			replaceflag = true;
			res += replacemap[sub_re];
			pos += it->second;
			--pos;
		}
		if (!replaceflag) res += regex[pos];
		++pos;
	}
	
	return res;
}


string code_generate(string token_name, vector<string>& endstates, vector<DFATransition>& dfaTransitions)
{
	map<pair<string, string>, string> transition_table;
	map<string, string> token_map;
	for (auto& it : dfaTransitions)
		transition_table.insert({ {it.fromstate.statename, it.transitionsymbol},it.tostate.statename });
	for (auto& it : endstates)
		token_map.insert({ it, token_name });

	string code = R"(
#include <iostream>
#include <string>
#include <unordered_map>
using namespace std;

// DFA states transition table
unordered_map<int, unordered_map<char, int>> transitions = {
)";
	// 生成转移表代码
	map<string, map<string, string>> grouped;
	for (auto& entry : transition_table) {
		const auto& key_pair = entry.first;							// 原始键（pair<int, char>）
		const string state = key_pair.first;						// 分解键的第一个部分
		const string input_char = key_pair.second;					// 分解键的第二个部分
		const string target_state = entry.second;					// 值（目标状态）
		grouped[state][input_char] = target_state;
	}

	for (auto& outer_entry : grouped) {
		const string state = outer_entry.first;
		const auto& trans = outer_entry.second;

		code += "    {" + state + ", {\n";

		for (auto& inner_entry : trans) {
			const string ch = inner_entry.first;
			const string target = inner_entry.second;

			code += "        {'" + ch + "', " + target + "},\n";
		}

		code += "    }},\n";
	}

	// 生成接受状态映射
	code += "unordered_map<int, string> accept_tokens = {\n";
	for (auto& entry : token_map) {
		const string state = entry.first;
		const string token = entry.second;
		code += "    {" + state + ", \"" + token + "\"},\n";
	}
	code += "};\n\n";

	// 生成词法分析函数
	code += R"(string lex_analyze(const string& input) {
    int current_state = 0; // the start state
    string token;
    string result;
    
    for (char c : input) {
        auto& state_map = transitions[current_state];
        auto it = state_map.find(c);
        
        if (it == state_map.end()) {
            // deal with the error
            cerr << "Lexical error at: " << c << endl;
            return "";
        }
        
        current_state = it->second;
        token += c;
        
        // check the accepted states
        if (accept_tokens.count(current_state)) {
            result += accept_tokens[current_state] + " ";
            token.clear();
        }
    }
    
    // deal with the last token
    if (!token.empty() && accept_tokens.count(current_state)) {
        result += accept_tokens[current_state];
    }
    
    return result;
}

int main() {
    string input;
    cout << "Input string: ";
    getline(cin, input);
    cout << "Tokens: " << lex_analyze(input) << endl;
    return 0;
})";
	return code;
}
