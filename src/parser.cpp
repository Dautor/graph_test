
// INCLUDES //-----------------------------------------------------------------------

#include "parser.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef __FreeBSD__
#include <stdarg.h>
#endif

// MACROS //-------------------------------------------------------------------------
// TYPES //--------------------------------------------------------------------------
// INTERNAL PROCEDURE DECLARATIONS //------------------------------------------------
// INTERNAL VARIABLES //-------------------------------------------------------------
// VARIABLES //----------------------------------------------------------------------

parser Parser;
token const &Token = Parser.Token;

// INTERNAL PROCEDURES //------------------------------------------------------------

static void
ScanStr()
{
    auto &Token     = Parser.Token;
    auto &At        = Parser.At;
    auto &LineStart = Parser.LineStart;
    assert(*At == '"');
    auto Start = ++At;
    while(*At && *At != '"')
    {
        char Val = *At;
        if(Val == '\n')
        {
            FatalErrorHere("[L] String literal cannot contain newline");
        } else if(Val == '\\')
        {
            FatalErrorHere("[L] String escaping not supported yet");
        } else
        {
            ++At;
        }
    }
    if(*At)
    {
        assert(*At == '"');
        ++At;
    } else
    {
        FatalErrorHere("[L] Unexpected end of file within string literal");
    }
    Token.Kind          = token::kind::STRING;
    Token.STRING._      = Start;
    Token.STRING.Length = At-Start-1;
}

// PROCEDURES //---------------------------------------------------------------------

void
Error(char const *fmt, ...)
{
    printf("Error:\n");
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

bool
_MatchToken(token::kind Kind)
{
    if(_IsToken(Kind))
    {
        TokenNext();
        return true;
    } else
    {
        return false;
    }
}

void
TokenNext()
{
    Repeat:
    auto &Token     = Parser.Token;
    auto &At        = Parser.At;
    auto &LineStart = Parser.LineStart;
    Token.Pos.Column = At - LineStart + 1;
    switch(*At)
    {
        case ' ': case '\n': case '\t':
        {
            while(*At == ' ' || *At == '\n' || *At == '\t')
            {
                if(*At == '\n')
                {
                    LineStart = At+1;
                    ++Token.Pos.Line;
                }
                ++At;
            }
            goto Repeat;
        } break;
        
        case '"': { ScanStr(); } break;
        
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '_':
        {
            char const *Start = At;
            while(isalnum(*At) || (*At == '_')) ++At;
            Token.NAME.Length = At-Start;
            Token.NAME._ = strndup(Start, Token.NAME.Length); // We should maybe intern strings
            Token.Kind = token::kind::NAME;
        } break;
        
#define _(c, k) \
case c: \
Token.Kind = token::kind::k; \
++At; \
if(0);
#define __(c, k) \
else if(*At == c) \
{ \
Token.Kind = token::kind::k; \
++At; \
}
#define _1(c1, k1)                 _(c1, k1) break;
#define _2(c1, k1, c2, k2)         _(c1, k1) __(c2, k2) break;
#define _3(c1, k1, c2, k2, c3, k3) _(c1, k1) __(c2, k2) __(c3, k3) break;
        
        _1(0,   NONE);
        _1(',', COMMA);
        _1(':', COLON);
        _1('{', BRACE_LEFT);
        _1('}', BRACE_RIGHT);
        
        default:
        {
            if(isprint(*At))
            {
                printf("Unknown character '%c'\n", *At);
            } else
            {
                printf("Unknown character [%u]\n", *At);
            }
            
            ++At;
            goto Repeat;
        };
        
    }
}

void
Initialize(parser *Parser, char const *Path, file File)
{
    Parser->File      = File;
    Parser->At        = (char const *)File.Data;
    Parser->LineStart = Parser->At;
    Parser->Token.Pos.Path = Path ? Path : "<string>";
    Parser->Token.Pos.Line = 1;
    TokenNext();
}

char const *
Name(token::kind Kind)
{
    if((u32)Kind < ArrayCount(token::KindNames))
    {
        return token::KindNames[(u32)Kind];
    } else
    {
        return nullptr;
    }
}

char const *
TokenInfo(token &Token)
{
    switch(Token.Kind)
    {
        case token::kind::NAME:   { return Token.NAME._;   } break;
        case token::kind::STRING: { return Token.STRING._; } break;
        default: return Name(Token.Kind); break;
    }
}
