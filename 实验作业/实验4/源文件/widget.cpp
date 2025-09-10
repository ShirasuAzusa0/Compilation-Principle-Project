#include <QString>
#include <string>
#include <QFile>
#include <QTextStream>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QPen>
#include <QtMath>
#include <QPen>
#include <QStandardItemModel>
#include <QDebug>
#include <QFileDialog>
#include <QTextCodec>
#include <QMessageBox>
#include <QTemporaryFile>
#include <map>
#include <unordered_map>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <unordered_set>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "widget.h"
#include "ui_widget.h"
#pragma execution_character_set("utf-8")
using namespace std;

// 文法规则字符串
string grammar_str;

// 结构化文法规则map（非终结符->所有右部分支）
unordered_map<string, set<string>> grammar_map;

// 文法规则集合（顺序id、左部、右部）
struct grammarUnit
{
    int gid;
    string left;
    string right;
    grammarUnit(string l, string r): gid(-1), left(l), right(r){}
};

// 文法数组（按照原先输入的文法规则顺序存储增广和划分后的文法规则）
deque<grammarUnit> grammar_deque;

// 起始符、增广后的起始符
string startSymbol, trueStartSymbol;

// first 集合
unordered_map<string, set<string>> firstSet;

// follow 集合
unordered_map<string, set<string>> followSet;
unordered_set<string> nonTerminals;

// 下一状态
struct nextStateUnit
{
    // 进入该状态所通过的字符（串）
    string s;
    // nextState的id
    int sid;
};

// LR(0)项目，形如[A->a.b]
struct Item {
    // gid，对应 grammar_deque 中产生式的序号
    int gid;
    // 右部各符号（串）
    vector<string> rhs;
    // "."符号，用于标志该符号在rhs中的位置
    int dot;
    bool operator <(Item const& o) const {
        if(gid != o.gid) return gid < o.gid;
        if(dot != o.dot) return dot < o.dot;
        return rhs < o.rhs;
    }

    bool operator==(Item const& o) const {
            return gid == o.gid
                && dot == o.dot
                && rhs == o.rhs;
    }
};

// LR(1)项目，形如 [A->a.b, c]
struct Item_LR1 {
    // gid，对应 grammar_deque 中产生式的序号
    int gid;
    // 右部各符号（串）
    vector<string> rhs;
    // "."符号，用于标志该符号在rhs中的位置
    int dot;
    // 展望符号（串）集合
    set<string> lookahead;

    bool operator<(Item_LR1 const& o) const {
        if (gid != o.gid) return gid < o.gid;
        if (dot != o.dot) return dot < o.dot;
        if (rhs != o.rhs) return rhs < o.rhs;
        return lookahead < o.lookahead;
    }
    bool operator==(Item_LR1 const& o) const {
        return gid==o.gid && dot==o.dot && rhs==o.rhs && lookahead==o.lookahead;
    }
};

// LR(0)自动机状态：一个项目集合，使用set便于快速查重
using State = set<Item>;

// LR(1)自动机状态：一个项目集合，使用set便于快速查重
using State_LR1 = set<Item_LR1>;

// 状态间的转移：从状态from经由符号（串）symbol转移到状态to
struct Trans {
    int from;
    int to;
    string symbol;
};

// 所有状态（LR0）
vector<State> states;

// 所有状态（LR1）
vector<State_LR1> states_LR1;

// 所有转移（LR0和LR1通用）
vector<Trans> trans;

// ACTION[state][terminal] = {类型, 编号}
// GOTO[state][nonterminal] = 目标状态
// ACTION表
struct Action {
    enum {SHIFT, REDUCE, ACCEPT, ERROR} act;
    int num;
};
vector<unordered_map<string, Action>> ACTION;

// GOTO表
vector<unordered_map<string, int>> GOTO_;

// 判断是否为非终结符（串）(允许大小写字母、数字和下划线，首字符为字母）
bool isNonTerminalName(const string& str)
{
    // 字符串为空或首字符不是字母的情况
    if (str.empty() || !isalpha(str[0])) return false;
    // 遍历串内所有字符，考虑字符不是字母、数字或下划线的情况
    for (char c : str)
        if(!isalnum(c) || c == '_') return false;
    return true;
}

// 将二型文法RHS按'|'划分为多个分支，并返回所有分支（仅右部产生式）
vector<string> splitAlternatives(const string& rhs)
{
    vector<string> parts;
    string part;
    istringstream ss(rhs);
    // 先通过字符串流ss将rhs读入到parts中，再判断是否有'|'符号
    while(getline(ss, part, '|'))
    {
        // 去掉前后空格
        auto l = part.find_first_not_of(" \t");
        auto r = part.find_last_not_of(" \t");
        // 将每次划分得到的非空左部分添加到vector中暂存
        if (l != string::npos)
            parts.push_back(part.substr(l, r - l + 1));
    }
    return parts;
}


// 辅助：从 trans/trans1 中按 BFS 计算每个状态到 0 号状态的最短距离（层次）
static std::vector<int> computeLayers(int n, const std::vector<Trans>& edges) {
    std::vector<int> layer(n, -1);
    std::queue<int> q;
    layer[0] = 0;
    q.push(0);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (auto &t : edges) {
            if (t.from == u && layer[t.to] == -1) {
                layer[t.to] = layer[u] + 1;
                q.push(t.to);
            }
        }
    }
    // 如果有未访问节点，统一放在最后一层
    int maxL = 0;
    for (int d : layer) if (d > maxL) maxL = d;
    for (int i = 0; i < n; ++i)
        if (layer[i] == -1) layer[i] = maxL + 1;
    return layer;
}

// 把连线截短，让箭头在圆边而不是中心
static void addEdgeWithArrow(QGraphicsScene* scene,
                             const QPointF& p1, const QPointF& p2,
                             double nodeR,
                             const QString& label = QString())
{
    // 计算方向向量
    double dx = p2.x() - p1.x(), dy = p2.y() - p1.y();
    double D = std::hypot(dx, dy);
    if (D < 1e-6) return;
    // 起点移出圆心
    QPointF s = p1 + QPointF(dx/D * nodeR, dy/D * nodeR);
    // 终点移到圆边
    QPointF e = p2 - QPointF(dx/D * nodeR, dy/D * nodeR);

    // 主线
    scene->addLine(s.x(), s.y(), e.x(), e.y(), QPen(Qt::black));

    // 箭头
    double angle = std::atan2(dy, dx);
    double aSz = 8.0;
    QPointF pa = e - QPointF(aSz * std::cos(angle - M_PI/6),
                             aSz * std::sin(angle - M_PI/6));
    QPointF pb = e - QPointF(aSz * std::cos(angle + M_PI/6),
                             aSz * std::sin(angle + M_PI/6));
    scene->addLine(e.x(), e.y(), pa.x(), pa.y(), QPen(Qt::black));
    scene->addLine(e.x(), e.y(), pb.x(), pb.y(), QPen(Qt::black));

    // 标签放在线段中点偏上
    if (!label.isEmpty()) {
        QPointF mid = (s + e) * 0.5;
        auto txt = scene->addText(label);
        txt->setPos(mid.x()+2, mid.y()+2);
    }
}


// 文法规则处理函数
void handleGrammar()
{
    // first step: 按行拆分
    // 将整个多行的文法规则字符串grammar_str按行拆分，去掉空行后存入lines
    vector<string> lines;
    istringstream iss(grammar_str);
    string line;
    while(getline(iss, line))
    {
        if(!line.empty())
            lines.push_back(line);
    }

    // second step: 解析每条文法规则
    bool firstRule = true;
    for (auto& line : lines)
    {
        // 找到符号"->"，通过符号"->"将每行文法规则划分为左部和右部（LHS 和 RHS）
        auto pos = line.find("->");
        // 跳过不合法行（找不到符号"->"）
        if(pos == string::npos) continue;

        // LHS 和 RHS，划分左右段
        string lhs = line.substr(0, pos);
        string rhs = line.substr(pos + 2);

        // 去除左右空格
        auto trim = [](string& s)
        {
            auto l = s.find_first_not_of(" \t");
            auto r = s.find_last_not_of(" \t");
            if (l == string::npos)
            {
                s.clear();
                return;
            }
             s = s.substr(l, r - l + 1);
        };
        trim(lhs);
        trim(rhs);

        // 判断是否为合法非终结符（串）
        if (!isNonTerminalName(lhs))
        {
            // 后续有待增加其他可视化提示
            cerr << "Error: 非法的非终结符名字" << lhs << endl;
            continue;
        }

        // 记录开始符号
        if(firstRule)
        {
            startSymbol = lhs;
            trueStartSymbol = lhs;
            firstRule = false;
        }

        // 拆分多分支（将或运算中的每条分支单独拆分出来），若无分支则直接存储
        auto alts = splitAlternatives(rhs);
        for(string& alt : alts)
        {
            grammar_map[lhs].insert(alt);
            grammar_deque.emplace_back(lhs, alt);
        }
    }

    // third step: 增广，若开始符号有多条产生式，则加“^”开头的新开始，确保总的开始符号有且仅有一个
    // 增广后统一用"^"作为总的开始符号
    // grammar_map[startSymbol].size() > 1 意味着开始符号（串）不唯一
    if(grammar_map[startSymbol].size() >= 1)
    {
        grammar_deque.push_front(grammarUnit("^", startSymbol));
        trueStartSymbol = "^";
    }

    // forth step: 编号
    int gid = 0;
    for (auto& unit : grammar_deque)
        unit.gid = gid++;
}

// 将用空格分隔的符号拆分成单个符号，并忽略空串符号ε
// 遍历每个分词，遇到空串则跳过，非空串则加入到vector中，最后返回这个不包含空串的vector
vector<string> splitSymbols(const string& str)
{
    vector<string> res;
    istringstream ss(str);
    string symbol;
    while(ss >> symbol)
    {
        if(symbol == "ε") continue;
        res.push_back(symbol);
    }
    return res;
}

// 判断是否为终结符(串)，若符号不在nonTerminals中且不等于ε，则视为终结符
bool isTerminal(const string& symbol)
{
    return nonTerminals.find(symbol) == nonTerminals.end() && symbol != "ε";
}

// 求 first 集合
void getFirstSet()
{
    // 初始化非终结符集合，将grammar_map中的所有左部非终结符加入到nonTerminals中
    for(auto& [lhs, rhsSet] : grammar_map)
        nonTerminals.insert(lhs);

    bool changed;
    do
    {
        changed = false;
        // 遍历grammar_map中所有的右部产生式
        for(const auto& [lhs, rhsSet] : grammar_map)
        {
            // 遍历右部每个分支
            for(const string& rhs : rhsSet)
            {
                // 获取当前分支的每个分词
                auto symbols = splitSymbols(rhs);
                // 标记是否所有符号能推出ε
                bool epsilon_in_all = true;

                // 遍历每个分词（符号（串））
                for(const string& symbol : symbols)
                {
                    // 若为终结符（串），则直接加入当前非终结符（串）的first集合中
                    if(isTerminal(symbol))
                    {
                        if(firstSet[lhs].insert(symbol).second)
                            changed = true;
                        // 终结符（串）无法退出ε
                        epsilon_in_all = false;
                        // 停止对当前产生式的扫描遍历
                        break;
                    }
                    // 若为非终结符（串），则将这个非终结符（串）的 first 集合去除ε后添加到当前非终结符（串）中
                    else
                    {
                        for(const string& fsymbol : firstSet[symbol])
                            if(fsymbol != "ε" && firstSet[lhs].insert(fsymbol).second)
                                changed = true;
                        // 若该非终结符（串）的first集合不含ε，则无需继续遍历后续符号
                        if(!firstSet[symbol].count("ε"))
                        {
                            epsilon_in_all = false;
                            break;
                        }
                    }
                }

                // 若所有符号都能推到ε，则将ε加入到当前非终结符（串）的first集合中
                if (epsilon_in_all)
                    if (firstSet[lhs].insert("ε").second)
                        changed = true;
            }
        }
    } while(changed);   // 循环直至无新元素的加入
}

// 求follow集合
void getfollowSet()
{
    // 初始化，将增广文法起始符号的follow集合包含结束符号$
    followSet[trueStartSymbol].insert("$");
    bool changed;
    do
    {
        changed = false;
        // 遍历文法规则序列，按用户输入的文法规则顺序处理产生式
        for(const auto& unit : grammar_deque)
        {
            const string& lhs = unit.left;
            // 获取当前非终结符（串）的产生式序列
            auto symbols = splitSymbols(unit.right);

            // 针对序列中每个非终结符（串）B，计算其follow集合
            for(size_t i = 0; i < symbols.size(); ++i)
            {
                string B = symbols[i];
                // 若B为终结符（串），则跳过它
                if(!nonTerminals.count(B)) continue;

                // 用于判断右侧剩余符号是否均可推ε
                bool epsilon_suffix = true;

                // 考察B之后的符号（串）beta
                for(size_t j = i + 1; j < symbols.size(); ++j)
                {
                    const string& beta = symbols[j];
                    epsilon_suffix = false;

                    // 若beta是终结符，则加入到B的follow集合中
                    if(isTerminal(beta))
                    {
                        if(followSet[B].insert(beta).second)
                            changed = true;
                        break;
                    }

                    // beta为非终结符，则将first(beta)-{ε}加入到B的follow集合中
                    for(const string& fsymbol : firstSet[beta])
                        if(fsymbol != "ε" && followSet[B].insert(fsymbol).second)
                            changed = true;

                    // beta不含有ε则停止查找
                    if(!firstSet[beta].count("ε")) break;
                    // 含有ε则继续查找下一符号（串）
                    else epsilon_suffix = true;
                }

                // 若beta全可推得ε或beta为空，则将左部的follow集合加入到B的follow集合中
                if(epsilon_suffix)
                {
                    for(const string& f : followSet[lhs])
                    {
                        if(followSet[B].insert(f).second)
                            changed = true;
                    }
                }
            }
        }
    } while(changed);   // 循环直至无新元素增加
}

// 展示FIRST集合和FOLLOW集合
void displayFirstandFollow(QTableView* tableView1, QTableView* tableView2)
{
    QStandardItemModel* model1 = new QStandardItemModel();
    QStandardItemModel* model2 = new QStandardItemModel();
    model1->setHorizontalHeaderLabels(QStringList() << "非终结符" << "FIRST集合");
    model2->setHorizontalHeaderLabels(QStringList() << "非终结符" << "FOLLOW集合");

    for (const auto& nt : nonTerminals)
    {
        QString FIRSTSet = "{ ";
        for(const string& s : firstSet[nt])
            FIRSTSet += QString::fromStdString(s) + " ";
        FIRSTSet += "}";

        QString FOLLOWSet = "{ ";
        for(const string& s : followSet[nt])
            FOLLOWSet += QString::fromStdString(s) + " ";
        FOLLOWSet += "}";

        QList<QStandardItem*> row1, row2;
        row1 << new QStandardItem(QString::fromStdString(nt))
            << new QStandardItem(FIRSTSet);

        row2 << new QStandardItem(QString::fromStdString(nt))
             << new QStandardItem(FOLLOWSet);

        model1->appendRow(row1);
        model2->appendRow(row2);
    }

    tableView1->setModel(model1);
    tableView1->resizeColumnsToContents();

    tableView2->setModel(model2);
    tableView2->resizeColumnsToContents();
}

// 计算 FIRST(β a)，其中 β 是符号序列，a 是单个 lookahead
// 规则：
//   1）把 β 的每个符号 X 的 FIRST(X)\{ε} 都加入结果；
//   2）如果 β 中所有符号都能推出 ε（或 β 为空），再把 a 加入结果。
set<string> firstOfSequence(const vector<string>& beta, const string& a)
{
    set<string> result;
    bool allEpsilon = true;

    // 1) 扫描 β 中每个符号
    for (const auto& X : beta)
    {
        if (isTerminal(X))
        {
            // β 第一个符号就是终结符，直接加入它，结束
            result.insert(X);
            allEpsilon = false;
            break;
        }
        // X 是非终结符，则把 FIRST(X)\{ε} 加入
        for (const auto& f : firstSet[X])
            if (f != "ε")
                result.insert(f);

        // 如果 X 的 FIRST 中不含 ε，就不再继续 β 中剩余符号
        if (!firstSet[X].count("ε"))
        {
            allEpsilon = false;
            break;
        }
    }

    // 2) 如果 β 全部能推 ε（或 β 为空），把 lookahead a 加入
    if (allEpsilon)
        result.insert(a);

    return result;
}

// LR(0)的CLOSURE 闭包计算
State closure(const State& I)
{
    State C = I;
    bool added;
    do
    {
        added = false;
        for(auto it = C.begin(); it != C.end(); ++it)
        {
            const Item& item = *it;
            // 若"."符号后面有非终结符B
            if(item.dot < (int)item.rhs.size())
            {
                string B = item.rhs[item.dot];
                if(nonTerminals.count(B))
                {
                    // 对每条B->e 添加入[B -> . e]
                    for(auto& unit : grammar_deque)
                    {
                        if(unit.left == B)
                        {
                            vector<string> symbols = splitSymbols(unit.right);
                            Item newItem{unit.gid, symbols, 0};
                            auto p = C.insert(newItem);
                            if(p.second)
                            {
                                added = true;
                                it = C.begin();
                            }
                        }
                    }
                }
            }
        }
    } while(added);
    return C;
}

// LR(1)的CLOSURE 闭包计算
State_LR1 closure_LR1(const State_LR1& I) {
    State_LR1 C = I;
    bool added;
    do {
        added = false;
        for (auto it = C.begin(); it != C.end(); ++it) {
            const Item_LR1& item = *it;
            if (item.dot < (int)item.rhs.size()) {
                string B = item.rhs[item.dot];
                if (nonTerminals.count(B)) {
                    // β = item.rhs[item.dot+1...]
                    vector<string> beta(item.rhs.begin() + item.dot + 1, item.rhs.end());
                    for (const auto& a : item.lookahead) {
                        // FIRST(β a)
                        set<string> firsts = firstOfSequence(beta, a);
                        for (auto& t : firsts) {
                            for (auto& unit : grammar_deque) if (unit.left == B) {
                                Item_LR1 newIt;
                                newIt.gid = unit.gid;
                                newIt.rhs = splitSymbols(unit.right);
                                newIt.dot = 0;
                                newIt.lookahead = {t};
                                if (C.insert(newIt).second) {
                                    added = true;
                                    it = C.begin();
                                }
                            }
                        }
                    }
                }
            }
        }
    } while (added);
    return C;
}

// LR(0)的GOTO计算
State GOTO(const State& I, const string& X)
{
    State J;
    // 项目中".X"的移进
    for(auto& it : I)
    {
        if(it.dot < (int)it.rhs.size() && it.rhs[it.dot] == X)
        {
            Item moved = it;
            moved.dot++;
            J.insert(moved);
        }
    }
    return closure(J);
}

// LR(1)的GOTO计算
State_LR1 GOTO_LR1(const State_LR1& I, const string& X)
{
    State_LR1 J;
    // 项目中".X"的移进
    for(auto& it : I)
    {
        if(it.dot < (int)it.rhs.size() && it.rhs[it.dot] == X)
        {
            Item_LR1 moved = it;
            moved.dot++;
            J.insert(moved);
        }
    }
    return closure_LR1(J);
}

// LR0生成DFA程序入口
int getLR0()
{
    // 初态：closure([S'->. S])，增广后的开始位置
    vector<string> startSymbols = splitSymbols(grammar_deque.front().right);
    Item startItem{grammar_deque.front().gid, startSymbols, 0};
    states.push_back(closure({startItem}));

    bool added;
    do
    {
        added = false;
        for(int i = 0; i < (int)states.size(); ++i)
        {
            // 对每个可能的符号（串）X（终结或非终结）
            set<string> symbols;
            for(auto& it : states[i])
            {
                if(it.dot < (int)it.rhs.size())
                    symbols.insert(it.rhs[it.dot]);
            }
            for(auto& X : symbols)
            {
                State J = GOTO(states[i], X);
                if(J.empty()) continue;
                // 看 J 是否已存在
                auto jt = find(states.begin(), states.end(), J);
                int j;
                if(jt == states.end())
                {
                    states.push_back(J);
                    j = states.size() - 1;
                    added = true;
                }
                else
                    j = distance(states.begin(), jt);
                trans.push_back({i, j, X});
            }
        }
    } while(added);

    // 二次去重
    int max_from = trans[1].from;
    for (const auto& t : trans)
        if (t.from > max_from)
            max_from = t.from;

    trans.erase(
            std::remove_if(
                trans.begin(),
                trans.end(),
                [max_from](const Trans& t) { return t.from == max_from; }
            ),
            trans.end()
        );

    return states.size();
}

// LR1生成DFA程序入口
void getLR1()
{
    states_LR1.clear(); trans.clear();
        // 初始项目 [S'->.S, $]
        vector<string> S0 = splitSymbols(grammar_deque.front().right);
        Item_LR1 start{grammar_deque.front().gid, S0, 0, set<string>{"$"}};
        states_LR1.push_back(closure_LR1({start}));

        bool added;
        do {
            added = false;
            for (int i = 0; i < (int)states_LR1.size(); ++i) {
                set<string> syms;
                for (auto& it : states_LR1[i]) if (it.dot < (int)it.rhs.size())
                    syms.insert(it.rhs[it.dot]);
                for (auto& X : syms) {
                    State_LR1 J = GOTO_LR1(states_LR1[i], X);
                    if (J.empty()) continue;
                    auto jt = find(states_LR1.begin(), states_LR1.end(), J);
                    int j;
                    if (jt == states_LR1.end()) {
                        states_LR1.push_back(J);
                        j = states_LR1.size() - 1;
                        added = true;
                    } else j = distance(states_LR1.begin(), jt);
                    trans.push_back({i, j, X});
                }
            }
        } while(added);
}

// 构建LR1分析表
void buildLR1Table() {
    int N = states_LR1.size();
    ACTION.assign(N, {});
    GOTO_.assign(N, {});

    // 1) 先把 trans1 中的所有 (from, symbol)->to 映射缓存起来
    std::map<std::pair<int, std::string>, int> transMap;
    for (auto& t : trans) {
        transMap[{t.from, t.symbol}] = t.to;
    }

    // 2) 填 ACTION 和 GOTO 表
    for (int i = 0; i < N; ++i) {
        // 2.1) ACTION 部分
        for (auto& it : states_LR1[i]) {
            // 移进：dot < rhs.size() 且下一个符号是终结符
            if (it.dot < (int)it.rhs.size() && isTerminal(it.rhs[it.dot])) {
                std::string a = it.rhs[it.dot];
                // 从 transMap 里查下一个状态 j
                auto key = std::make_pair(i, a);
                auto pos = transMap.find(key);
                if (pos != transMap.end()) {
                    int j = pos->second;
                    ACTION[i][a] = { Action::SHIFT, j };
                }
            }
            // 接受：增广产生式 S' -> S .，gid==0 且 dot 到末尾
            else if (it.dot == (int)it.rhs.size() && it.gid == 0) {
                ACTION[i]["$"] = { Action::ACCEPT, 0 };
            }
            // 规约：dot 在末尾（且不是增广产生式）
            else if (it.dot == (int)it.rhs.size()) {
                for (auto& a : it.lookahead) {
                    ACTION[i][a] = { Action::REDUCE, it.gid };
                }
            }
        }

        // 2.2) GOTO 部分：对于所有非终结符转移
        for (auto& nt : nonTerminals) {
            auto key = std::make_pair(i, nt);
            auto pos = transMap.find(key);
            if (pos != transMap.end()) {
                GOTO_[i][nt] = pos->second;
            }
        }
    }
}

// 检查移进-规约冲突
bool SLR1_check1()
{
    for (int i = 0; i < (int)states.size(); ++i)
    {
        const State& st = states[i];
        // 本状态上可移进的“终结符”
        set<string> shift_terms;
        for (auto& it : st)
        {
            if (it.dot < (int)it.rhs.size())
            {
                const string& X = it.rhs[it.dot];
                // 只把真正的终结符算作 shift，这里可以直接用 isTerminal
                if (isTerminal(X))
                    shift_terms.insert(X);
            }
        }

        // 对所有规约项目，若它的 FOLLOW 集合与 shift_terms 有交集，就冲突
        for (auto& it : st)
        {
            if (it.dot == (int)it.rhs.size())  // 规约项目
            {
                const string& B = grammar_deque[it.gid].left;
                for (auto& a : followSet[B])
                {
                    if (shift_terms.count(a))
                        return true;
                }
            }
        }
    }
    return false;
}

// 检查规约-规约冲突
bool SLR1_check2()
{
    for (int i = 0; i < (int)states.size(); ++i)
    {
        const State& st = states[i];
        // 收集本状态所有规约项目对应的非终结符
        vector<string> reduces;
        for (const auto& it : st)
        {
            if (it.dot == (int)it.rhs.size())
                reduces.push_back(grammar_deque[it.gid].left);
        }

        // 两两比较 FOLLOW 集合是否有交集
        for (size_t p = 0; p + 1 < reduces.size(); ++p)
        {
            for (size_t q = p + 1; q < reduces.size(); ++q)
            {
                const auto& F1 = followSet[reduces[p]];
                const auto& F2 = followSet[reduces[q]];
                for (const auto& a : F1)
                {
                    if (F2.count(a))
                        return true;
                }
            }
        }
    }
    return false;
}

// SLR(1)分析函数
int SLR1Analyse()
{
    bool flag1 = SLR1_check1();
    bool flag2 = SLR1_check2();
    //移进规约、规约规约冲突
    if(flag1 && flag2) return 3;
    // 规约规约冲突
    else if(flag2) return 2;
    // 移进规约冲突
    else if(flag1) return 1;
    // 无冲突，是SLR(1)文法
    else return 0;
}

// LR0绘制DFA函数
void drawDFA(QGraphicsView* view)
{
    QGraphicsScene* scene = new QGraphicsScene();
    view->setScene(scene);

    // first step: 构造DFA先
    getLR0();

    int N = (int)states.size();
    auto layer = computeLayers(N, trans);

    // 按层分组
    std::map<int, std::vector<int>> groups;
    for (int i = 0; i < N -1; ++i) groups[layer[i]].push_back(i);

    // 计算布局参数
    double nodeR = 30.0;
    double vGap = 120.0;  // 垂直层间距
    double hGap = 100.0;  // 水平节点间距

    // 生成坐标
    std::vector<QPointF> pos(N);
    for (auto& kv : groups) {
        int lvl = kv.first;
        auto& vec = kv.second;
        int m = (int)vec.size();
        double y = lvl * vGap;
        // 水平中心对齐
        double width = (m - 1) * hGap;
        for (int j = 0; j < m; ++j) {
            double x = j * hGap - width/2;
            pos[vec[j]] = QPointF(x, y);
        }
    }

    // 画节点
    for (int i = 0; i < N - 1; ++i) {
        bool isAccept = false;
        for (auto& it : states[i])
            if (it.gid == 0 && it.dot == (int)it.rhs.size())
                isAccept = true;
        // 外圆
        scene->addEllipse(
            pos[i].x()-nodeR, pos[i].y()-nodeR,
            nodeR*2, nodeR*2,
            QPen(Qt::black),
            isAccept ? QBrush(Qt::green) : QBrush(Qt::transparent)
        );
        if (isAccept) {
            scene->addEllipse(
                pos[i].x()-nodeR+4, pos[i].y()-nodeR+4,
                nodeR*2-8, nodeR*2-8,
                QPen(Qt::black)
            );
        }
        scene->addText(QString::number(i))
             ->setPos(pos[i].x()-6, pos[i].y()-10);
    }

    // 画所有边，确保箭头指向圆边而非圆心
    for (auto& t : trans) {
        // 判断是否为自循环
        if (t.from == t.to) {
            // 中心
            QPointF c = pos[t.from];
            // 半径
            double r = nodeR;
            // 环绕半径
            double loopR = r * 2.0;  // 增大环绕半径

            // 起点：节点顶部
            QPointF start = c + QPointF(0, -r);
            // 控制点1：左上方更远处
            QPointF ctrl1 = c + QPointF(loopR*1.5, -loopR*1.5);
            // 控制点2：左上方近处
            QPointF ctrl2 = c + QPointF(loopR*1.0, -loopR*0.5);
            // 终点：节点左侧（不是闭合回起点）
            QPointF end = c + QPointF(r, 0);

            QPainterPath path(start);
            path.cubicTo(ctrl1, ctrl2, end);
            scene->addPath(path, QPen(Qt::black));

            // 计算曲线终点处的切线方向（ctrl2到end的向量）
            QPointF tangent = end - ctrl2;
            double angle = std::atan2(tangent.y(), tangent.x());

            // 绘制箭头
            double aSz = 8.0;
            QPointF pA = end - QPointF(aSz * std::cos(angle - M_PI/6),
                                      aSz * std::sin(angle - M_PI/6));
            QPointF pB = end - QPointF(aSz * std::cos(angle + M_PI/6),
                                      aSz * std::sin(angle + M_PI/6));
            scene->addLine(end.x(), end.y(), pA.x(), pA.y(), QPen(Qt::black));
            scene->addLine(end.x(), end.y(), pB.x(), pB.y(), QPen(Qt::black));

            // 标签放在曲线中间（ctrl1附近）
            QGraphicsTextItem* label = scene->addText(QString::fromStdString(t.symbol));
            label->setPos(ctrl2.x() - label->boundingRect().width()/2,
                          ctrl2.y() - 20);
        }
        else {
            // 其他普通的转移
            addEdgeWithArrow(scene, pos[t.from], pos[t.to], nodeR,
                              QString::fromStdString(t.symbol));
        }
    }

    view->show();
}

// LR1绘制DFA函数
void drawDFA_LR1(QGraphicsView* view)
{
    // 1) 构造 LR(1) 的 DFA，填充 states1, trans1
    getLR1();

    // 2) 新建 Scene，并关联到 view
    QGraphicsScene* scene = new QGraphicsScene(view);

    int N = (int)states_LR1.size();
    // 复用同一个 BFS 层次算法
    // 但是 Trans1 的类型略有不同，需要做个适配
    std::vector<Trans> tmp;
    tmp.reserve(trans.size());
    for (auto& t : trans)
        tmp.push_back({t.from, t.to, t.symbol});
    auto layer = computeLayers(N, tmp);

    std::map<int, std::vector<int>> groups;
    for (int i = 0; i < N; ++i) groups[layer[i]].push_back(i);

    double nodeR = 30.0, vGap = 120.0, hGap = 100.0;
    std::vector<QPointF> pos(N);
    for (auto& kv : groups) {
        int lvl = kv.first;
        auto& vec = kv.second;
        int m = (int)vec.size();
        double y = lvl * vGap;
        double width = (m - 1) * hGap;
        for (int j = 0; j < m; ++j) {
            double x = j * hGap - width/2;
            pos[vec[j]] = QPointF(x, y);
        }
    }

    // 画节点（标接受状态）
    for (int i = 0; i < N; ++i) {
        bool isAccept = false;
        for (auto& it : states_LR1[i])
            if (it.gid == 0 && it.dot == (int)it.rhs.size())
                isAccept = true;

        scene->addEllipse(
            pos[i].x()-nodeR, pos[i].y()-nodeR,
            nodeR*2, nodeR*2,
            QPen(Qt::black),
            isAccept ? QBrush(Qt::green) : QBrush(Qt::transparent)
        );
        if (isAccept) {
            scene->addEllipse(
                pos[i].x()-nodeR+4, pos[i].y()-nodeR+4,
                nodeR*2-8, nodeR*2-8,
                QPen(Qt::black)
            );
        }
        scene->addText(QString::number(i))
             ->setPos(pos[i].x()-6, pos[i].y()-10);
    }

    // 画边
    for (auto& t : trans) {
        // 判断是否为自循环
        if (t.from == t.to) {
            // 中心
            QPointF c = pos[t.from];
            // 半径
            double r = nodeR;
            // 环绕半径
            double loopR = r * 2.0;  // 增大环绕半径

            // 起点：节点顶部
            QPointF start = c + QPointF(0, -r);
            // 控制点1：左上方更远处
            QPointF ctrl1 = c + QPointF(loopR*1.5, -loopR*1.5);
            // 控制点2：左上方近处
            QPointF ctrl2 = c + QPointF(loopR*1.0, -loopR*0.5);
            // 终点：节点左侧（不是闭合回起点）
            QPointF end = c + QPointF(r, 0);

            QPainterPath path(start);
            path.cubicTo(ctrl1, ctrl2, end);
            scene->addPath(path, QPen(Qt::black));

            // 计算曲线终点处的切线方向（ctrl2到end的向量）
            QPointF tangent = end - ctrl2;
            double angle = std::atan2(tangent.y(), tangent.x());

            // 绘制箭头
            double aSz = 8.0;
            QPointF pA = end - QPointF(aSz * std::cos(angle - M_PI/6),
                                      aSz * std::sin(angle - M_PI/6));
            QPointF pB = end - QPointF(aSz * std::cos(angle + M_PI/6),
                                      aSz * std::sin(angle + M_PI/6));
            scene->addLine(end.x(), end.y(), pA.x(), pA.y(), QPen(Qt::black));
            scene->addLine(end.x(), end.y(), pB.x(), pB.y(), QPen(Qt::black));

            // 标签放在曲线中间（ctrl1附近）
            QGraphicsTextItem* label = scene->addText(QString::fromStdString(t.symbol));
            label->setPos(ctrl2.x() - label->boundingRect().width()/2,
                          ctrl2.y() - 20);
        }
        else {
            // 其他普通的转移
            addEdgeWithArrow(scene, pos[t.from], pos[t.to], nodeR,
                              QString::fromStdString(t.symbol));
        }
    }

    // 6) 把 scene 挂到 view 上
    view->setScene(scene);
}

// 构建并展示LR1分析表
void ShowTable_LR1(QTableView* tableView)
{
    // 1) 构造 LR(1) 的 DFA 和分析表
    getLR1();    // 产生 states1, trans1
    buildLR1Table();  // 产生 ACTION, GOTO_

    // 2) 提取终结符集（RHS 中出现，且是终结符），并加入 '$'
    std::set<std::string> terminalSet;
    for (auto& unit : grammar_deque) {
        auto syms = splitSymbols(unit.right);
        for (auto& sym : syms) {
            if (isTerminal(sym))
                terminalSet.insert(sym);
        }
    }
    terminalSet.insert("$");

    // 转成 vector 便于按列索引访问
    std::vector<std::string> terminals(terminalSet.begin(), terminalSet.end());

    // 3) 构造表头：State + 终结符列 + 非终结符列
    QStringList headers;
    headers << "State";
    for (auto& t : terminals)
        headers << QString::fromStdString(t);
    for (auto& A : nonTerminals)
        headers << QString::fromStdString(A);

    // 4) 创建 QStandardItemModel
    int rowCount = static_cast<int>(states_LR1.size());
    int colCount = 1 + static_cast<int>(terminals.size()) + static_cast<int>(nonTerminals.size());
    QStandardItemModel* model = new QStandardItemModel(rowCount, colCount);
    model->setHorizontalHeaderLabels(headers);

    // 5) 填充 ACTION 和 GOTO
    for (int i = 0; i < rowCount; ++i) {
        // 状态号
        model->setItem(i, 0, new QStandardItem(QString::number(i)));

        // ACTION 列
        for (int c = 0; c < static_cast<int>(terminals.size()); ++c) {
            const std::string& t = terminals[c];
            QString cell;
            auto itAct = ACTION[i].find(t);
            if (itAct != ACTION[i].end()) {
                switch (itAct->second.act) {
                    case Action::SHIFT:
                        cell = QString("s%1").arg(itAct->second.num);
                        break;
                    case Action::REDUCE: {
                        auto& prod = grammar_deque[itAct->second.num];
                        cell = QString("r%1(%2→%3)")
                               .arg(itAct->second.num)
                               .arg(QString::fromStdString(prod.left))
                               .arg(QString::fromStdString(prod.right));
                        break;
                    }
                    case Action::ACCEPT:
                        cell = "acc";
                        break;
                    default:
                        break;
                }
            }
            model->setItem(i, 1 + c, new QStandardItem(cell));
        }

        // GOTO 列
        int base = 1 + static_cast<int>(terminals.size());
        int idx  = 0;
        for (auto& A : nonTerminals) {
            QString cell;
            auto itGoto = GOTO_[i].find(A);
            if (itGoto != GOTO_[i].end()) {
                cell = QString::number(itGoto->second);
            }
            model->setItem(i, base + idx, new QStandardItem(cell));
            ++idx;
        }
    }

    // 6) 绑定到 QTableView 并调整列宽
    tableView->setModel(model);
    tableView->resizeColumnsToContents();
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

// 将文法规则文件的内容载入到程序中
void Widget::on_pushButton_load_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"),
                                                    QDir::homePath(),
                                                    tr("文本文件 (*.txt);;所有文件(*.*)"));
    if(!filePath.isEmpty())
    {
        ifstream inputFile;
        QTextCodec* code = QTextCodec::codecForName("GB2312");

        string selectedFile = code->fromUnicode(filePath.toStdString().c_str()).data();
        inputFile.open(selectedFile.c_str(), ios::in);

        // 处理文件读取失败的错误
        if(!inputFile)
        {
            QMessageBox::critical(this, "错误信息", "文件导入错误！无法打开文件，请检查路径或文件是否被占用");
            cerr << "Error opening file." << endl;
        }

        // 读取文件内容并显示在文法规则输入框 plainTextEdit 中
        stringstream buffer;
        buffer << inputFile.rdbuf();
        QString fileContent = QString::fromStdString(buffer.str());
        ui->plainTextEdit->setPlainText(fileContent);
    }
}

// 将输入的文法规则保存到文本文件中
void Widget::on_pushButton_save_clicked()
{
   QString saveFilePath = QFileDialog::getSaveFileName(this,
                                                   tr("保存结果文件"),
                                                   QDir::homePath(),
                                                   tr("文本文件 (*.txt)"));
    if(!saveFilePath.isEmpty() && !ui->plainTextEdit->toPlainText().isEmpty())
    {
        QFile outputFile(saveFilePath);
        if(outputFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream stream(&outputFile);
            stream << ui->plainTextEdit->toPlainText();
            outputFile.close();
            QMessageBox::about(this, "提示", "文法规则已成功保存到文件中！");
        }
    }
    else if(ui->plainTextEdit->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, "提示", "当前文法规则输入框为空！");
    }
}

void Widget::on_pushButton_help_clicked()
{
    QString helpMessage = "输入的文法均为2型文法（上下文无关文法），文法规则为了处理上的简单，输入时均默认输入的第一个非终结符就是文法的开始符号，用@表示空串";
    ui->plainTextEdit_help->setPlainText(helpMessage);
}

void Widget::on_pushButton_firstandfollow_clicked()
{
    grammar_map.clear();
    grammar_deque.clear();
    nonTerminals.clear();
    firstSet.clear();
    followSet.clear();
    grammar_str = ui->plainTextEdit->toPlainText().toStdString();
    handleGrammar();
    getFirstSet();
    getfollowSet();
    displayFirstandFollow(ui->tableView_firstSet, ui->tableView_followSet);
}

void Widget::on_pushButton_check_clicked()
{
    grammar_map.clear();
    grammar_deque.clear();
    nonTerminals.clear();
    states.clear();
    trans.clear();
    firstSet.clear();
    followSet.clear();
    startSymbol.clear();
    trueStartSymbol.clear();

    grammar_str = ui->plainTextEdit->toPlainText().toStdString();
    handleGrammar();
    getFirstSet();
    getfollowSet();
    getLR0();
    string res;
    int status = SLR1Analyse();
    switch (status)
    {
    case 0: res = "该文法是 SLR(1) 文法"; break;
    case 1: res = "该文法发生移进-规约冲突，不是SLR(1)文法"; break;
    case 2: res = "该文法发生规约-规约冲突，不是SLR(1)文法"; break;
    case 3: res = "该文法发生移进-规约冲突和规约-规约冲突，不是SLR(1)文法"; break;
    default: res="未知错误"; break;
    }
    QString qres = QString::fromStdString(res);
    ui->plainTextEdit_check->setPlainText(qres);
}

void Widget::on_pushButton_LR0DFA_clicked()
{
    grammar_map.clear();
    grammar_deque.clear();
    nonTerminals.clear();
    states.clear();
    trans.clear();
    grammar_str = ui->plainTextEdit->toPlainText().toStdString();
    handleGrammar();
    getFirstSet();
    getfollowSet();
    getLR0();
    drawDFA(ui->graphicsView_LR0);
}

void Widget::on_pushButton_LR1DFA_clicked()
{
    grammar_map.clear();
    grammar_deque.clear();
    nonTerminals.clear();
    states.clear();
    trans.clear();
    firstSet.clear();
    followSet.clear();
    startSymbol.clear();
    trueStartSymbol.clear();

    grammar_str = ui->plainTextEdit->toPlainText().toStdString();
    handleGrammar();
    getFirstSet();
    getfollowSet();
    getLR1();
    drawDFA_LR1(ui->graphicsView_LR1);
}

void Widget::on_pushButton_LR1table_clicked()
{
    grammar_map.clear();
    grammar_deque.clear();
    nonTerminals.clear();
    states.clear();
    trans.clear();
    firstSet.clear();
    followSet.clear();
    startSymbol.clear();
    trueStartSymbol.clear();

    grammar_str = ui->plainTextEdit->toPlainText().toStdString();
    handleGrammar();
    getFirstSet();
    getfollowSet();
    getLR1();
    ShowTable_LR1(ui->tableView_LR1);
}

// 选做部分
void doLR1Parse(const QStringList& inputTokens, QStandardItemModel *parseModel)
{
    // 构建分析表
    getLR1();
    buildLR1Table();

    // 初始化：状态栈、符号栈、输入指针
    vector<int> stateStack;
    QStringList symbolStack;
    stateStack.push_back(0);
    symbolStack.push_back("$");

    // 输入指针
    int ip = 0;
    QStringList tokens = inputTokens;

    while (true) {
        int curState = stateStack.back();
        QString a = tokens[ip];

        // 查 ACTION 表
        auto itAct = ACTION[curState].find(a.toStdString());
        QString actionStr;
        if (itAct == ACTION[curState].end()) {
            actionStr = "error";
        } else {
            const Action& act = itAct->second;
            switch (act.act) {
            case Action::SHIFT:
                actionStr = QString("s%1").arg(act.num);
                // 记录当前行
                parseModel->appendRow(QList<QStandardItem*>{
                    new QStandardItem(symbolStack.join(" ") + " | " + QString::number(stateStack.back())),
                    new QStandardItem(tokens.mid(ip).join(" ")),
                    new QStandardItem(actionStr)
                });
                // 执行 shift
                symbolStack.push_back(a);
                stateStack.push_back(act.num);
                ip++;
                break;

            case Action::REDUCE: {
                int rid = act.num;
                auto& prod = grammar_deque[rid];
                // 右部符号数
                int k = splitSymbols(prod.right).size();
                // 记录当前行
                parseModel->appendRow(QList<QStandardItem*>{
                    new QStandardItem(symbolStack.join(" ") + " | " + QString::number(stateStack.back())),
                    new QStandardItem(tokens.mid(ip).join(" ")),
                    new QStandardItem(QString("r%1(%2→%3)")
                                      .arg(rid)
                                      .arg(QString::fromStdString(prod.left))
                                      .arg(QString::fromStdString(prod.right)))
                });
                // 执行 reduce：弹出 k 个符号和 k 个状态
                for (int i = 0; i < k; ++i) {
                    symbolStack.pop_back();
                    stateStack.pop_back();
                }
                // 压入左部非终结符
                QString A = QString::fromStdString(prod.left);
                symbolStack.push_back(A);
                // GOTO
                int gotoState = GOTO_[ stateStack.back() ][ prod.left ];
                stateStack.push_back(gotoState);
                break;
            }

            case Action::ACCEPT:
                // 记录最后一行
                parseModel->appendRow(QList<QStandardItem*>{
                    new QStandardItem(symbolStack.join(" ") + " | " + QString::number(stateStack.back())),
                    new QStandardItem("$"),
                    new QStandardItem("acc")
                });
                QMessageBox::information(nullptr, "结果", "句子被成功接受！");
                return;

            default:
                QMessageBox::critical(nullptr, "错误", "出现无法识别的动作！");
                return;
            }
        }

        // 若出错，则跳出
        if (actionStr == "error") {
            QMessageBox::warning(nullptr, "错误", "分析过程出现错误，无法继续");
            return;
        }
    }
}

void Widget::on_pushButton_sentence_clicked()
{
    grammar_map.clear();
    grammar_deque.clear();
    nonTerminals.clear();
    states.clear();
    trans.clear();
    firstSet.clear();
    followSet.clear();
    startSymbol.clear();
    trueStartSymbol.clear();

    grammar_str = ui->plainTextEdit->toPlainText().toStdString();
    handleGrammar();
    getFirstSet();
    getfollowSet();

    QStandardItemModel *parseModel = new QStandardItemModel(this);
    // 设置 parseModel 表头
    parseModel->setHorizontalHeaderLabels(
            QStringList() << "栈" << "剩余输入" << "动作");
        ui->tableView_LR1Tokens->setModel(parseModel);
        ui->tableView_LR1Tokens->horizontalHeader()->setSectionResizeMode(
            QHeaderView::Stretch);

    QString inputTokens = ui->plainTextEdit_sentence->toPlainText().trimmed();
    if(inputTokens.isEmpty())
    {
        QMessageBox::warning(this, "提示", "待分析的句子不能为空");
        return ;
    }
    // 按空格分词
       QStringList tokens = inputTokens.split(' ', QString::SkipEmptyParts);
       // 加上结束符
       tokens << "$";

       // 清空旧结果
       parseModel->removeRows(0, parseModel->rowCount());

       // 执行 LR(1) 分析，逐步向 parseModel 中添加行
       doLR1Parse(tokens, parseModel);
}

