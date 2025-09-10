#include "globals.h"
#include "util.h"
#include "scan.h"
#include <QDebug>
#include <string>

/* states in scanner DFA */
typedef enum
{
    START, INASSIGN, INCOMMENT, INNUM, INID, DONE
}
StateType;

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN + 1];

/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

void reset()
{
    lineno = 0;
    linepos = 0;
    bufsize = 0;
    EOF_flag = FALSE;
}

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
static int getNextChar(void)
{
    if (!(linepos < bufsize))
    {
        lineno++;
        if (fgets(lineBuf, BUFLEN - 1, source))
        {
            qDebug() << lineno<< " : " << lineBuf;
            debugMsg.append(QString::number(lineno)).append(QString::fromStdString(+":" + (std::string)lineBuf));
            if (EchoSource) fprintf(listing, "%4d: %s", lineno, lineBuf);
            bufsize = strlen(lineBuf);
            linepos = 0;
            return lineBuf[linepos++];
        }
        else
        {
            EOF_flag = TRUE;
            return EOF;
        }
    }
    else return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
   in lineBuf */
static void ungetNextChar(void)
{
    if (!EOF_flag) linepos--;
}

/* lookup table of reserved words */
static struct
{
    const char* str;
    TokenType tok;
} reservedWords[MAXRESERVED]
= { {"if",IF},{"then",THEN},{"else",ELSE},
  {"repeat",REPEAT},{"until",UNTIL},
  {"read",READ},{"write",WRITE},
  {"for",FOR}, {"while", WHILE},{"endwhile",ENDWHILE},
  {"to",TO},{"downto",DOWNTO},
  {"do",DO},{"enddo",ENDDO},
  {"and",AND},{"or",OR},{"not",NOT},
  {"end",END}
};

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup(char* s)
{
    int i;
    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str))
            return reservedWords[i].tok;
    return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the
 * next token in source file
 */
TokenType getToken(void)
{  /* index for storing into tokenString */
    int tokenStringIndex = 0;
    /* holds current token to be returned */
    TokenType currentToken;
    /* current state - always begins at START */
    StateType state = START;
    /* flag to indicate save to tokenString */
    int save;
    while (state != DONE)
    {
        int c = getNextChar();
        save = TRUE;
        switch (state)
        {
        case START:
            if (isdigit(c))
                state = INNUM;
            else if (isalpha(c))
                state = INID;
            else if (c == ':')
                state = INASSIGN;
            else if ((c == ' ') || (c == '\t') || (c == '\n'))
                save = FALSE;
            else if (c == '{')
            {
                save = FALSE;
                state = INCOMMENT;
            }
            else
            {
                state = DONE;
                switch (c)
                {
                case EOF:
                    save = FALSE;
                    currentToken = ENDFILE;
                    break;
                case '=':
                    currentToken = EQ;
                    break;
                case '>':
                    c = getNextChar(); // Read the next character
                    if (c == '=')
                    {
                        currentToken = RTEQ; // Token for >=
                    }
                    
                    else
                    {
                        ungetNextChar(); // Backup if it's not >= or <>
                        currentToken = RT; // Token for >
                    }
                    break;
                case '<':
                    c = getNextChar(); // Read the next character
                    if (c == '=')
                    {
                        currentToken = LTEQ; // Token for <=
                    }
                    else if (c == '>')
                    {
                        currentToken = NOTEQ; // Token for <>
                    }
                    else
                    {
                        ungetNextChar(); // Backup if it's not <=
                        currentToken = LT; // Token for <
                    }
                    break;
                case '+':
                    c = getNextChar(); // Read the next character
                    if (c == '=')
                    {
                        currentToken = PLUSEQ; // Token for +=
                    }
                    else if (c == '+')
                    {
                        currentToken = PLUSPLUS; // Token for ++
                    }
                    else
                    {
                        ungetNextChar(); // Backup if it's not += and ++
                        currentToken = PLUS; // Token for +
                    }
                    break;
                case '-':
                    c = getNextChar(); // Read the next character
                    if (c == '-')
                    {
                        currentToken = SUBSUB; // Token for --
                    }
                    else
                    {
                        ungetNextChar(); // Backup if it's not --
                        currentToken = MINUS;   // Token for -
                    }
                    break;
                case '*':
                    currentToken = TIMES;
                    break;
                case '/':
                    currentToken = OVER;
                    break;
                case '(':
                    currentToken = LPAREN;
                    break;
                case ')':
                    currentToken = RPAREN;
                    break;
                case '[':
                    currentToken = LPM;
                    break;
                case ']':
                    currentToken = RPM;
                    break;
                case ';':
                    currentToken = SEMI;
                    break;
                case '%':
                    currentToken = MOD;
                    break;
                case '^':
                    currentToken = POWER;
                    break;
                case '|':
                    currentToken = RGOR;
                    break;
                case '&':
                    currentToken = RGAND;
                    break;
                case '#':
                    currentToken = RGCLOSE;
                    break;
                case '?':
                    currentToken = RGCHOOSE;
                    break;
                default:
                    currentToken = ERROR;
                    break;
                }
            }
            break;
        case INCOMMENT:
            save = FALSE;
            if (c == EOF)
            {
                state = DONE;
                currentToken = ENDFILE;
            }
            else if (c == '}') state = START;
            break;
        case INASSIGN:
            state = DONE;
            if (c == '=')
                currentToken = ASSIGN;
            else if (c == ':')
            {
                c = getNextChar(); // Read the next character
                if (c == '=')
                {
                    currentToken = REGEX; // Token for ::=
                }
                else
                {
                    ungetNextChar();
                    ungetNextChar();
                    save = FALSE;
                    currentToken = ERROR;
                }
            }
            else
            { /* backup in the input */
                ungetNextChar();
                save = FALSE;
                currentToken = ERROR;
            }
            break;
        case INNUM:
            if (!isdigit(c))
            { /* backup in the input */
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = NUM;
            }
            break;
        case INID:
            if (!isalpha(c))
            { /* backup in the input */
                ungetNextChar();
                save = FALSE;
                state = DONE;
                currentToken = ID;
            }
            break;
        case DONE:
        default: /* should never happen */
            fprintf(listing, "Scanner Bug: state= %d\n", state);
            state = DONE;
            currentToken = ERROR;
            break;
        }
        if ((save) && (tokenStringIndex <= MAXTOKENLEN))
            tokenString[tokenStringIndex++] = (char)c;
        if (state == DONE)
        {
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID)
                currentToken = reservedLookup(tokenString);
        }
    }
    if (TraceScan) {
        fprintf(listing, "\t%d: ", lineno);
        printToken(currentToken, tokenString);
    }
    return currentToken;
} /* end getToken */

