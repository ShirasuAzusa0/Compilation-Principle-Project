// 1.算术表达式

#include<iostream>
#include<math.h>
using namespace std;

// 单词种类的枚举
enum TokenID {NUMBER,ID,ADD,SUB,MUL,DIV,LBRACKET,RBRACKET,ENDINPUT };

// 单词结构
struct TokenStru
{
  TokenID  ID;
  double val;
  char op;
  char word[20];
};


TokenStru token;
char buffer[255];
int pos=0;

// 词法分析函数：算术表达式的词法分析，

void GetToken() 
{
   int s;

   while (buffer[pos]==32||buffer[pos]==9)  // spacebar or tab 
	   pos++; 

   if((buffer[pos]>='0' )&& (buffer[pos]<='9'))  // digit
   {   
	   s=0;
	   while (buffer[pos]>='0' && buffer[pos]<='9') 
	   {  s=s*10+buffer[pos]-'0';   pos++ ;  } 
       token.ID=NUMBER; token.val=s;  
       if ( buffer[pos]=='.')
		{ 
		  int len=0;
          s=0; pos++;
	      while (buffer[pos]>='0' && buffer[pos]<='9') 
		  {  s=s*10+buffer[pos]-'0'; len++;  pos++ ; } 
		  token.val=token.val+s/pow(10,len);
		}
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
	   token.ID=ID;
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
	   default:  cout<<" Error Input at: "<<pos+1; exit(1);   
	 }
	 pos++;
   }
   else  token.ID=ENDINPUT; 

}


void main()
{
  cin.getline(buffer,255); // 读入一行的算术表达式
  pos=0;
  
  GetToken(); // 获取第一单词
  cout<<"The result is : "<<endl; 
  while  ( token.ID!=ENDINPUT)   
   {
      if (token.ID==NUMBER) cout<<token.ID<<" "<<token.val<<endl;
	   else if (token.ID==ID) cout<<token.ID<<" "<<token.word<<endl;
          else cout<<token.ID<<" "<<token.op<<endl;
	  GetToken(); // 获取下一个单词
  }
  system("pause");

}