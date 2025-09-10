#include "pch.h"
#include "CodeClassifier.h"

//regex integer_token(R"((0b[01_]+|0o[0-7_]+[0x[0-9a-fA-F_]+|\d[\d_]*))");

//关键字
vector<string> keywords = {
	"as", "break", "const", "continue", "crate", "else", "enum", "extern", "false", "fn",
	"for", "if", "impl", "in", "let", "loop", "match", "mod", "move", "mut", "pub", "ref",
	"return", "self", "static", "struct", "super", "trait", "true", "type", "unsafe", "use", "where", "while"
};

//操作符
vector<string> symbol_1 = {
	"+", "-", "*", "/", "%", "^", "!", "&", "|", "&&", "||", "<<", ">>", "+=", "-=", "*=", "/=", "%=", "^=",
	"&=", "|=", "<<=", ">>=", "=", "==", "!=", ">", "<", ">=", "<=", "@", "_", ".", "..", "...", "..=", ":",
	"::", "->", "#", "$", "?"
};

//分隔符
vector<string> symbol_2 = { "{", "}", "[", "]", "(", ")", ";", "," };

// 确认选择的编程语言，调用不同的
string CodeClassifier::Classify(string code)
{
	if (GetType() == "Rust")
		return Rust_Classify(code);
	else
		return "error";
}

// Rust语言单词分类函数
string CodeClassifier::Rust_Classify(string code)
{
	// 给代码文本末尾添加上换行符
	code += "\r\n";
	// 用vector容器存放Rust代码的每一行
	vector <string> code_perrow;
	// res字符串用来存放分类的结果
	string res;
	// row暂存每行代码，part、front_part、sub_part用于存放当前判断的字段以及其前后字段
	string row = "", part = "", front_part = "", sub_part = "";
	int j = 0;
	// 将每行代码分别存入vector容器中，通过换行符来划分
	for (int i = 0; i < code.length(); i++)
	{
		if(code[i] != '\r' && code[i] != '\n') row += code[i];
		if(code[i+1] == '\r' && code[i+2] == '\n')
		{
			code_perrow.push_back(row);
			row = "";
		}
	}
	// 从vector容器中读取出每一行代码，并对读出的代码进行单词分类
	int pos = 0, rows = 0;
	for (int i = 0; i < code_perrow.size(); i++)
	{
		if (rows)
		{
			i += rows - 1;
			rows = 0;
			continue;
		}
		// 将程序的当前行存放到res字符串中
		if (code_perrow[i] != "" && code_perrow[i] != " ")
			res += code_perrow[i] + "\r\n";
		pos = 0;
		// 逐一读取当前行的代码的字符
		while (pos < code_perrow[i].length())
		{
			// 当字符为英文字符时，考虑是关键字、标识符、原生字符串、字节或字节串
			if ((code_perrow[i][pos] >= 'A' && code_perrow[i][pos] <= 'Z') || (code_perrow[i][pos] >= 'a' && code_perrow[i][pos] <= 'z'))
			{
				while ((code_perrow[i][pos] >= 'A' && code_perrow[i][pos] <= 'Z') || (code_perrow[i][pos] >= 'a' && code_perrow[i][pos] <= 'z') || (code_perrow[i][pos] >= '0' && code_perrow[i][pos] <= '9') || code_perrow[i][pos] == '_')
				{
					part += code_perrow[i][pos];
					pos++;
				}
				if (code_perrow[i][pos] == '\'')
				{
					part += string(1, code_perrow[i][pos++]);
					while (code_perrow[i][pos] != '\'')
						part += string(1, code_perrow[i][pos++]);
					part += code_perrow[i][pos++];
					// 判断是字节
					if (part.substr(0, 2) == "b\'") res += part + "：字节字面量\r\n";
					else res += part + "：字节字面量错误\r\n";
				}
				else if (code_perrow[i][pos] == '\"')
				{
					part += string(1, code_perrow[i][pos++]);
					while (code_perrow[i][pos] != '\"')
						part += string(1, code_perrow[i][pos++]);
					part += code_perrow[i][pos++];
					// 判断是字节串
					if (part.substr(0, 2) == "b\"") res += part + "：字节串字面量\r\n";
					else res += part + "：字节串字面量错误\r\n";
				}
				else if (code_perrow[i][pos] == '#')
				{
					part += string(1, code_perrow[i][pos++]);
					while (code_perrow[i][pos] != '#')
						part += string(1, code_perrow[i][pos++]);
					part += string(1, code_perrow[i][pos++]);
					// 判断为原生字符串
					if (part.substr(0, 3) == "br#" || part.substr(0, 2) == "r#")
						res += part + "：原生字符串字面量\r\n";
					else res += part + "：原生字符串字面量错误\r\n";
				}
				// 在存放关键字的vector容器中查询，若找到则为关键字，否则为标识符
				else if (find(keywords.begin(), keywords.end(), part) != keywords.end())
					res += part + "：关键字\r\n";
				else
					res += part + "：标识符\r\n";
				// 当前读取位置pos回退一位
				pos--;
			}
			// 考虑是否是操作符
			else if (find(symbol_1.begin(), symbol_1.end(), string(1, code_perrow[i][pos])) != symbol_1.end())
			{
				part += string(1, code_perrow[i][pos]) + string(1, code_perrow[i][pos + 1]);
				// 考虑是否为双字符的操作符
				if (find(symbol_1.begin(), symbol_1.end(), part) != symbol_1.end())
				{
					sub_part = part;
					pos++;
					part += string(1, code_perrow[i][pos + 1]);
					//考虑是否是三字符的操作符
					if (find(symbol_1.begin(), symbol_1.end(), part) != symbol_1.end())
					{
						res += part + "：操作符\r\n";
						pos++;
					}
					else
						res += sub_part + "：操作符\r\n";
				}
				// 考虑是否为注释
				else
				{
					// 单行注释
					if (part == "//")
					{
						pos += 2;
						while (pos < code_perrow[i].length())
						{
							part += string(1, code_perrow[i][pos]);
							pos++;
						}
						res += part + "：注释\r\n";
					}
					// 多行注释
					else if (part == "/*")
					{
						int j = 0;
						j = code.find("/*") + 2;
						pos += 2;
						res += part + "\r\n";
						while (code[j] != '*' && code[j+1] != '/')
						{
							res += code[j];
							j++;
							pos++;
							if (code[j] == '\r' && code[j + 1] == '\n')
								rows++;
						}
						pos++;
						res += "\r\n*/：长（或多行）注释\r\n";
					}
					// 单字符操作符
					else
						res += string(1, code_perrow[i][pos]) + "：操作符\r\n";
				}
			}
			// 考虑是否是分隔符
			else if (find(symbol_2.begin(), symbol_2.end(), string(1, code_perrow[i][pos])) != symbol_2.end())
				res += string(1, code_perrow[i][pos]) + "：分隔符\r\n";
			// 考虑是否为数字字面量
			else if (code_perrow[i][pos] >= '0' && code_perrow[i][pos] <= '9')
			{
				int type = 0, num = 0;
				sub_part = part += string(1, code_perrow[i][pos]);
				part += string(1, code_perrow[i][pos + 1]);
				pos += 2;
				// 二进制整数
				if (part[1] == 'b')
					while ((code_perrow[i][pos] <= '1' && code_perrow[i][pos] >= '0') || (code_perrow[i][pos] == '_' || code_perrow[i][pos] == '.' || code_perrow[i][pos] == 'E' || code_perrow[i][pos] == '+' || code_perrow[i][pos] == 'u' || code_perrow[i][pos] == 'i' || code_perrow[i][pos] == 's' || code_perrow[i][pos] == 'z' || code_perrow[i][pos] == 'e'))
					{
						if (code_perrow[i][pos] <= '1' && code_perrow[i][pos] >= '0')
							num++;
						part += string(1, code_perrow[i][pos]);
						pos++;
						if (code_perrow[i][pos] == 'u' || code_perrow[i][pos] == 'i')
						{
							part += string(1, code_perrow[i][pos++]);
							while (code_perrow[i][pos] <= '9' && code_perrow[i][pos] >= '0')
								part += string(1, code_perrow[i][pos++]);
						}
					}
				// 八进制整数
				else if (part[1] == 'd')
					while ((code_perrow[i][pos] <= '7' && code_perrow[i][pos] >= '0') || (code_perrow[i][pos] == '_' || code_perrow[i][pos] == '.' || code_perrow[i][pos] == 'E' || code_perrow[i][pos] == '+' || code_perrow[i][pos] == 'u' || code_perrow[i][pos] == 'i' || code_perrow[i][pos] == 's' || code_perrow[i][pos] == 'z' || code_perrow[i][pos] == 'e'))
					{
						if (code_perrow[i][pos] <= '7' && code_perrow[i][pos] >= '0')
							num++;
						part += code_perrow[i][pos];
						pos++;
						if (code_perrow[i][pos] == 'u' || code_perrow[i][pos] == 'i')
						{
							part += string(1, code_perrow[i][pos++]);
							while (code_perrow[i][pos] <= '9' && code_perrow[i][pos] >= '0')
								part += string(1, code_perrow[i][pos++]);
						}
					}
				// 十六进制整数
				else if (part[1] == 'x')
					while ((code_perrow[i][pos] <= '9' && code_perrow[i][pos] >= '0') || (code_perrow[i][pos] >= 'A' && code_perrow[i][pos] <= 'F') || (code_perrow[i][pos] >= 'a' && code_perrow[i][pos] <= 'f') || code_perrow[i][pos] == '_' || code_perrow[i][pos] == '.' || code_perrow[i][pos] == 'E' || code_perrow[i][pos] == '+' || code_perrow[i][pos] == 'u' || code_perrow[i][pos] == 'i' || code_perrow[i][pos] == 's' || code_perrow[i][pos] == 'z' || code_perrow[i][pos] == 'e')
					{
						if (code_perrow[i][pos] <= '9' && code_perrow[i][pos] >= '0' || (code_perrow[i][pos] >= 'A' && code_perrow[i][pos] <= 'F'))
							num++;
						part += code_perrow[i][pos];
						sub_part = code_perrow[i][pos];
						pos++;
						if (code_perrow[i][pos] == 'u' || code_perrow[i][pos] == 'i')
						{
							part += string(1, code_perrow[i][pos++]);
							while (code_perrow[i][pos] <= '9' && code_perrow[i][pos] >= '0')
								part += string(1, code_perrow[i][pos++]);
						}
					}
				// 十进制整数
				else
				{
					pos--;
					part = sub_part;
					while ((code_perrow[i][pos] >= '0' && code_perrow[i][pos] <= '9') || (code_perrow[i][pos] == '_' || code_perrow[i][pos] == '.' || code_perrow[i][pos] == 'E' || code_perrow[i][pos] == '+' || code_perrow[i][pos] == 'u' || code_perrow[i][pos] == 'i' || code_perrow[i][pos] == 's' || code_perrow[i][pos] == 'z' || code_perrow[i][pos] == 'e'))
					{
						if (code_perrow[i][pos] <= '9' && code_perrow[i][pos] >= '0')
							num++;
						part += code_perrow[i][pos];
						pos++;
						if (code_perrow[i][pos] == 'u' || code_perrow[i][pos] == 'i')
						{
							part += string(1, code_perrow[i][pos++]);
							while (code_perrow[i][pos] <= '9' && code_perrow[i][pos] >= '0')
								part += string(1, code_perrow[i][pos++]);
						}
					}
				}
				// 通过后缀判断是否为浮点数（f32、f64）
				sub_part = "";
				if (code_perrow[i][pos] == 'f')
				{
					sub_part = "f";
					pos++;
					while (code_perrow[i][pos] >= '3' && code_perrow[i][pos] <= '6')
						sub_part += string(1, code_perrow[i][pos++]);
				}
				// 通过浮点是否存在以及其位置来确定是否为浮点数
				if (part.find(".") != string::npos)
				{
					if (part.find(".") + 1 >= part.length())
						return (res += "浮点数错误：" + part);
					else if (!isdigit(part[part.find(".") + 1]))
					{
						type = 0;
						part = part.substr(0, part.find("."));
					}
					else type = 1;
				}
				// 通过E和E+来确定是否为浮点数（科学计数法格式）
				else if (part.find("E") != string::npos && part.find("E+") != string::npos)
				{
					if (part.find("E+") + 1 >= part.length() || !isdigit(part[part.find("+") + 1]))
						return (res += "浮点数错误：" + part);
					else type = 1;
				}
				else if (part.length() >= 3 && (sub_part == "f32" || sub_part == "f64" || part.substr(part.size() - 3) != "f32" || part.substr(part.size() - 3) != "u64"))
				{
					type = 1;
					part += sub_part;
				}
				// 通过后缀判断是否为整数（u8、u16、u32、u64、u128、usize、i8、i16、i32、i64、i128、isize）
				if (part.find("u") != string::npos && (part.substr(part.size() - 2) != "u8" && part.substr(part.size() - 3) != "u16" && part.substr(part.size() - 3) != "u32" && part.substr(part.size() - 3) != "u64" && part.substr(part.size() - 4) != "u128" && part.substr(part.size() - 5) != "usize"))
					res += part + "：数字字面量错误\r\n";
				else if (part.find("i") != string::npos && (part.substr(part.size() - 2) != "i8" && part.substr(part.size() - 3) != "i16" && part.substr(part.size() - 3) != "i32" && part.substr(part.size() - 3) != "i64" && part.substr(part.size() - 4) != "i128" && part.substr(part.size() - 5) != "isize"))
					res += part + "：数字字面量错误\r\n";
				if (!num && part.length() > 1)
					res += part + "：数字字面量错误\r\n";
				// 根据刚才的判断给出最终字面量单词分类结果
				else
				{
					if (!type)
					{
						int x = part.find("b");
						if (part.find("b") != -1)
							res += part + "：字面量（二进制整数）\r\n";
						else if (part.find("d") != -1)
							res += part + "：字面量（八进制整数）\r\n";
						else if (part.find("x") != -1)
							res += part + "：字面量（十六进制整数）\r\n";
						else
							res += part + "：字面量（十进制整数）\r\n";
					}
					else
						res += part + "：字面量（浮点数）\r\n";
				}
				pos--;
			}
			// 单个字符字面量
			else if (code_perrow[i][pos] == '\'')
			{
				res += string(1, code_perrow[i][pos]);
				res += string(1, code_perrow[i][pos + 1]) + string(1, code_perrow[i][pos + 2]) + "：字符字面量\r\n";
				pos += 2;
			}
			// 字符串字面量
			else if (code_perrow[i][pos] == '\"')
			{
				res += string(1, code_perrow[i][pos]);
				pos++;
				while (code_perrow[i][pos] != '\"')
				{
					part += string(1, code_perrow[i][pos]);
					pos++;
				}
				res += part + string(1, code_perrow[i][pos]) + "：字符串字面量\r\n";
			}
			pos++;
			part = sub_part = "";
		}
		if(code_perrow[i] != "" && code_perrow[i] != " ")
			res += "\r\n";
	}
	return res;
}