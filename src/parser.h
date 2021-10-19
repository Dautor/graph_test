#pragma once

// INCLUDES //-----------------------------------------------------------------------

#include "utilities.h"

// MACROS //-------------------------------------------------------------------------

#define     FatalError(fmt, ...) (Error(fmt, ##__VA_ARGS__), fflush(stdout), exit(1))
#define      ErrorHere(fmt, ...) { Error("%P " fmt, &Token.Pos, ##__VA_ARGS__);      }
#define FatalErrorHere(fmt, ...) { FatalError("%P " fmt, &Token.Pos, ##__VA_ARGS__); }
#define     IsToken(x)     _IsToken(token::kind::x)
#define  MatchToken(x)  _MatchToken(token::kind::x)
#define ExpectToken(x) _ExpectToken(token::kind::x)

// TYPES //--------------------------------------------------------------------------

struct token
{
    source_position Pos;
    
    enum struct kind
    {
        NONE,
        NAME,
        STRING,
        COMMA,
        COLON,
        BRACE_LEFT,
        BRACE_RIGHT,
        Count,
    } Kind;
    
    static constexpr char const *const KindNames[(u32)kind::Count] =
    {
#define _(x) [(u32)kind::x]
        _(NONE)        = "EOF",
        _(NAME)        = "NAME",
        _(STRING)      = "STRING",
        _(COMMA)       = ",",
        _(COLON)       = ":",
        _(BRACE_LEFT)  = "{",
        _(BRACE_RIGHT) = "}",
#undef _
    };
    
    union
    {
        struct { char const *_; s32 Length; } NAME;
        struct { char const *_; s32 Length; } STRING;
    };
};

struct parser
{
    file File;
    char const *At;
    char const *LineStart;
    token Token;
};

// EXTERNAL VARIABLES //-------------------------------------------------------------

extern parser Parser;
extern token const &Token;

// PROCEDURE DECLARATIONS //---------------------------------------------------------

bool  _MatchToken(token::kind);
void _ExpectToken(token::kind);
void Error(char const *fmt, ...);

void TokenNext ();
void Initialize(parser *, char const *, file);

node_list *ParseConfig();
char const *TokenInfo(token &Token = Parser.Token);
char const *Name(token::kind);

// INLINE PROCEDURES //--------------------------------------------------------------

inline bool _IsToken(token::kind Kind) { return Token.Kind == Kind; }
