// 1.算术表达式

#include<iostream>
#include<string>
#include<math.h>
#include<map>
using namespace std;

// 单词种类的枚举
enum TokenID {IF,ELSE,FOR,WHILE,ASSIGN,EQ,LT,GT,LTE,GTE,NOTEQ,NUMBER,ID,ADD,SUB,MUL,DIV,LBRACKET,RBRACKET,SEMICOLON,ENDINPUT };

map<string, TokenID>keywords;

// 单词结构
struct TokenStru
{
  TokenID  ID;
  double val;
  char op[3];
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
       case '+': token.ID=ADD; token.op[0]=buffer[pos];token.op[1]='\0';  break;  // +
	   case '-': token.ID=SUB; token.op[0]=buffer[pos]; token.op[1]='\0'; break; // -
	   case '*': token.ID=MUL; token.op[0]=buffer[pos]; token.op[1]='\0'; break;   // *
	   case '/': token.ID=DIV; token.op[0]=buffer[pos]; token.op[1]='\0'; break;   // /
	   case '(': token.ID=LBRACKET; token.op[0]=buffer[pos]; token.op[1]='\0'; break; 
	   case ')': token.ID=RBRACKET; token.op[0]=buffer[pos];token.op[1]='\0';  break;
       case ';': token.ID=SEMICOLON; token.op[0]=buffer[pos];token.op[1]='\0';  break;
	   case '>': token.op[0]='>';
		         if (buffer[pos+1]=='=' )
				 { pos++; token.ID=GTE; token.op[1]='='; token.op[2]='\0'; break;  }
				 token.op[1]='\0';token.ID=GT; 
				 break;
	   case '<': token.op[0]='<';
		         if (buffer[pos+1]=='=' )
				 { pos++; token.ID=LTE; token.op[1]='='; token.op[2]='\0'; break;  }
				 token.op[1]='\0';token.ID=LT; 
				 break;
       case '=': token.op[0]='=';
		         if (buffer[pos+1]=='=' )
				 { pos++; token.ID=EQ; token.op[1]='='; token.op[2]='\0'; break;  }
				 token.op[1]='\0';token.ID=ASSIGN; 
				 break;
       case '!': token.op[0]='!';
		         if (buffer[pos+1]=='=' )
				 { pos++; token.ID=NOTEQ; token.op[1]='='; token.op[2]='\0'; break;  }
				 token.op[1]='\0';cout<<" !  ERROR"; 
				 break;
	   default:  cout<<" Error Input at: "<<pos+1; exit(1);   
	 }
	 pos++;
   }
   else  token.ID=ENDINPUT; 

}


void init()
{
   keywords["if"]=IF;
   keywords["else"]=ELSE;
   keywords["for"]=FOR;
   keywords["while"]=WHILE;

}


int main()
{
  
  init(); 

  cin.getline(buffer,255); // 读入一行的算术表达式
  pos=0;
  
  GetToken(); // 获取第一单词
  cout<<"The result is : "<<endl; 
  while  ( token.ID!=ENDINPUT)   
   {
	 if (token.ID<4)cout << token.ID << " Keyword"  << endl;
      else if (token.ID==NUMBER) cout<<token.ID<<" "<<token.val<<endl;
	   else if (token.ID==ID) cout<<token.ID<<" "<<token.word<<endl;
          else cout<<token.ID<<" "<<token.op<<endl;
	  GetToken(); // 获取下一个单词
  }

  system("pause");

  return 0;

}