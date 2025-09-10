#include<iostream>
#include<string>
#include<math.h>
#include<map>

using namespace std;

enum TokenID {IF,END,REPEAT,UNTIL,ELSE,READ,WRITE,THEN,ASSIGN,SEMI,ID,NUMBER,ADD,SUB,MUL,DIV,LBRACKET,RBRACKET,ENDFILE };

map<string, TokenID>keywords;


struct TokenStru  // Token 结构
{
  TokenID  ID;
  int val;
  char op;
  char word[20];
};

struct OperandsNode  // 四元组操作数结构
{
 int flag;   // 操作数是值=0，还是临时变量=1，空=2
 int val;     // 操作数为值时则存放值
 char NewVar[10];  // 操作数为临时变量时则存放临时变量名
};

OperandsNode exp();
OperandsNode term();
OperandsNode factor();

TokenStru token;

int varOrder=1;  // 记录当前准备产生的临时变量序号


char buffer[255];
int pos=0;

void GetToken()
{
   int s;

   while (buffer[pos]==32||buffer[pos]==9)  // spacebar or tab 
	   pos++; 

   if((buffer[pos]>='0' )&& (buffer[pos]<='9'))  // digit
   {
	   s=0;
	   while (buffer[pos]>='0' && buffer[pos]<='9') 
	   {  s=s*10+buffer[pos]-'0';   pos++ ; } 
       token.ID=NUMBER; token.val=s;  
   }
   else if (isalpha(buffer[pos])) 
   {   int len=0;
	   while (isalpha(buffer[pos])  || isdigit(buffer[pos]) || buffer[pos]=='_')
	   {
		   token.word[len]=buffer[pos];
		   len++;
		   pos++;
	   }
	   token.word[len]='\0';

	   map<string, TokenID>::iterator it;
	   it=keywords.find(token.word);    // key word
	   if (it!=keywords.end())
	      token.ID=keywords[token.word];
	   else token.ID=ID;

   }
   else if(buffer[pos]!='\0')
   {
     switch(buffer[pos])
	 {
       case '+': token.ID=ADD; token.op=buffer[pos];  break;  // +
	   case '-': token.ID=SUB; token.op=buffer[pos];  break; // -
	   case '*': token.ID=MUL; token.op=buffer[pos];  break;   // *
	   case '/': token.ID=DIV; token.op=buffer[pos];  break;   // /
	   case '(': token.ID=LBRACKET; token.op=buffer[pos];  break; 
	   case ')': token.ID=RBRACKET; token.op=buffer[pos];  break;
	   case ';': token.ID=SEMI; token.op=buffer[pos];  break;
	   case ':': if (buffer[pos+1]=='=') 
				 { pos++;token.ID=ASSIGN; strcpy(token.word,":="); break;
				 } 
		         else cout<<" Error Input at: "<<pos+1; exit(1);   
	   case '$': token.ID=ENDFILE; break;
	   default:  cout<<" Error Input at: "<<pos+1; exit(1);   
	 }
	 pos++;
   }
   else 
   {
	   cin.getline(buffer,255);
	   pos=0;
       GetToken();
   }
  
   if (token.ID==ENDFILE) exit(1);

}

void match(TokenID expecttokenid )
{
   if (token.ID== expecttokenid)
        GetToken();
   else{cout<<" Error Input at position: "<<pos+1; exit(1);}

}

void OutputOperand(OperandsNode x)  // 输出一个操作数
{

    cout<<" , ";
	if (x.flag==0)   // 根据其值输出值还是临时变量名作为操作数
		cout<<x.val;
	else if (x.flag==1)
	   cout<<x.NewVar;
    else cout<<" ";
}

void OutputOP(TokenStru token)
{
   if (token.ID==ASSIGN || token.ID==READ || token.ID==WRITE)
	   cout<<token.word;   // 输出操作码:= 或 read 或write
   else 
     cout<<token.op;   // 输出操作码，即运算符
}

void Gen(TokenStru token,OperandsNode x,OperandsNode y,OperandsNode z)  // 产生一条四元组
{
	cout<<"(";    
	OutputOP(token);   // 输出操作码，即运算符
	OutputOperand(x);  // 第一个操作数
	OutputOperand(y);   // 第二个操作数
	OutputOperand(z);   // 第三个操作数
	cout<<" ) ";
	cout<<endl;

}

OperandsNode exp()
{ 
   TokenStru op;
   char str[5];
   OperandsNode x,y,z;

   x=term();   // 获取一个操作数

   while(token.ID==ADD || token.ID==SUB)
   { 
	  op=token;
	  match(token.ID);  
      y=term();    // 获取一个操作数

      z.flag=1;strcpy(z.NewVar,"t");  // 产生一个新的临时变量
	  itoa(varOrder,str,10);strcat(z.NewVar,str);

      varOrder++;// 产生了一个新临时变量后，序号+1
      Gen(op,x,y,z);    //生成一个四元组
      x=z; 
   }
   return x;
}

OperandsNode term()
{   
   TokenStru op;

   char str[5];
   OperandsNode x,y,z;
   
   x=factor();// 获取一个操作数

   while(token.ID==MUL || token.ID==DIV)
   { 
      op=token;
	  match(token.ID);  
      y=factor();// 获取一个操作数

      z.flag=1;strcpy(z.NewVar,"t");   // 产生一个新的临时变量
	  itoa(varOrder,str,10);strcat(z.NewVar,str);

      varOrder++;  // 产生了一个新临时变量后，序号+1
      Gen(op,x,y,z);  //生成一个四元组
      x=z; 
   }
   return x;
}

OperandsNode factor()
{
   OperandsNode x;

  switch(token.ID)
  {
    case LBRACKET:  match(LBRACKET);
	                x=exp();   // 产生一个操作数
				    match(RBRACKET);
	                break;
	case NUMBER:    x.flag=0; x.val=token.val;  // 产生一个操作数
					match(NUMBER); 
					break;
	case ID:        x.flag=1; strcpy(x.NewVar,token.word);  // 产生一个操作数
					match(ID); 
					break;
	default: { cout<<"error input at position: " <<pos+1;  exit(1); }

  }
  return x;
}

void statement(void);

void stmt_sequence(void)
{ statement();
  while ((token.ID!=ENDFILE) && (token.ID!=END) &&
         (token.ID!=ELSE) && (token.ID!=UNTIL))
  { 
    match(SEMI);
    statement();
  }
  return ;
}

void assign_stmt(void)
{ 
  OperandsNode x,y,z;
  TokenStru op;

  y.flag=2;
  z.flag=1;
  strcpy(z.NewVar,token.word);

  match(ID);
  op=token;
  match(ASSIGN);

  x=exp();
  Gen(op,x,y,z);  //生成一个四元组

  return ;
}

void read_stmt(void)
{ 
  OperandsNode x,y,z;
  TokenStru op;

  op=token;
  match(READ);

  x.flag=y.flag=2;
  z.flag=1;
  strcpy(z.NewVar,token.word);
  match(ID);

  Gen(op,x,y,z);  //生成一个四元组

  return ;
}

void write_stmt(void)
{ 

  OperandsNode x,y,z;
  TokenStru op;

  op=token;
  match(WRITE);

  x.flag=y.flag=2;
  
  z=exp(); 

  Gen(op,x,y,z);  //生成一个四元组

  return ;

}

void repeat_stmt(void)
{

}

void if_stmt(void)
{ 
  match(IF);
  exp();
  match(THEN);
  stmt_sequence();
  if (token.ID==ELSE) {
    match(ELSE);
    stmt_sequence();
  }
  match(END);
  return ;
}

void statement(void)
{ 
  switch (token.ID) 
  {
    case IF :  if_stmt(); break;
    case REPEAT :repeat_stmt(); break;
    case ID :  assign_stmt(); break;
    case READ :  read_stmt(); break;
    case WRITE : write_stmt(); break;
    default : cout<<"unexpected token";
              GetToken();
              break;
  } 
  return ;
}

void init()
{
   keywords["if"]=IF;
   keywords["else"]=ELSE;
   keywords["read"]=READ;
   keywords["write"]=WRITE;

}

void main()
{
  init(); 

  cin.getline(buffer,255);
  pos=0;
  
  GetToken();
  
  stmt_sequence();

  if ( token.ID==ENDFILE) 
      cout<<"The result is correct! " <<endl;
  else cout<<"error input at position: " <<pos+1;

  system("pause");

}