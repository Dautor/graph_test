
// INCLUDES //-----------------------------------------------------------------------

#include <stdio.h>
#include "utilities.h"
#include "parser.h"
extern "C"
{
#include <printf.h>
}
#include <string.h>

// MACROS //-------------------------------------------------------------------------
// TYPES //--------------------------------------------------------------------------

struct cli_config
{
    char const *GraphPath;
};

// INTERNAL PROCEDURE DECLARATIONS //------------------------------------------------

static void Depends (node *, node *);
static bool Verify  (node *);
static void Schedule(node *);
static void Provide (node *);
static void NewLine ();
static void Print   (node *);
static bool VerifySpread(node *);

static void DependencyDone(node *, node *);

static s32 Print_p(FILE *, printf_info const *, void const *const *Args);
static s32 PrintFormatSingleArgument(printf_info const *, size_t N, s32 *ArgTypes);

[[noreturn]] static void Usage(char const *Arg0);
static void ParseConfig(cli_config *, s32 ArgCount, char const *Arg[], char const *Env[]);

// INTERNAL VARIABLES //-------------------------------------------------------------

static u32    Indentation = 0;
static const source_position SourcePositionBuiltin = {.Path="<builtin>"};

// VARIABLES //----------------------------------------------------------------------
// INTERNAL PROCEDURES //------------------------------------------------------------

static void
Usage(char const *Arg0)
{
    printf("usage: %s [--graph graph]\n", Arg0);
    exit(1);
}

static inline void
ParseConfig(cli_config *Config, s32 ArgCount, char const *Arg[], char const *Env[])
{
    (void)Env;
    for(s32 i = 1; i < ArgCount; ++i)
    {
        if(strcmp(Arg[i], "--graph") == 0)
        {
            if(i == ArgCount-1) Usage(Arg[0]);
            Config->GraphPath = Arg[++i];
        } else
        {
            Usage(Arg[0]);
        }
    }
}

void
_ExpectToken(token::kind Kind)
{
    if(_MatchToken(Kind) == false)
    {
        FatalErrorHere("Expected '%s', got '%s'", Name(Kind), TokenInfo());
    }
}

static void
Depends(node *A, node *B) // A depends on B
{
    A->Depends  = NodeListAlloc(B, A->Depends);
    B->Provides = NodeListAlloc(A, B->Provides);
}

static bool
Verify(node *A)
{
    ++Indentation;
    switch(A->CheckState)
    {
        case node::check_state::not_resolved:
        {
#if 0
            printf("checking %s", A->Name);
            NewLine();
#endif
            A->CheckState = node::check_state::resolving;
            for(auto i = A->Depends; i; i = i->Next)
            {
                if(Verify(i->_) == false)
                {
                    printf(" -> %s", A->Name);
                    return false;
                }
            }
            A->CheckState = node::check_state::resolved;
        } break;
        case node::check_state::resolving:
        {
            printf("\n%s depends on itself\n", A->Name);
            Indentation = 0;
            return false;
        } break;
        case node::check_state::resolved:;
    }
    --Indentation;
    return true;
}

static bool
VerifySpread(node *A)
{
    if(Verify(A) == false)
    {
        printf("\n");
        return false;
    }
    for(auto i = A->Provides; i; i = i->Next)
    {
        if(VerifySpread(i->_) == false) return false;
    }
    return true;
}

static void
Schedule(node *A)
{
    printf("Scheduling %s:\n", A->Name);
    printf("  \"%s\"\n", A->Rule);
    // actually Provide once we are notified that the rule is done
    Provide(A);
}

static void
Provide(node *A)
{
    for(auto i = A->Provides; i;)
    {
        auto Next = i->Next;
        DependencyDone(i->_, A);
        free(i);
        i = Next;
    }
    A->Provides = nullptr;
}

static void
DependencyDone(node *A, node *B)
{
    for(auto i = &A->Depends; *i;)
    {
        auto D = *i;
        if(D->_ == B)
        {
            auto Next = D->Next;
            free(D);
            *i = Next;
            break;
        }
        i = &(*i)->Next;
    }
    if(A->Depends == nullptr)
    {
        Schedule(A);
    }
}

static void
NewLine()
{
    printf("\n%*.s", Indentation, "");
}

static void
Print(node *A)
{
    printf("%s:", A->Name);
    ++Indentation;
    if(A->Depends != nullptr)
    {
        NewLine();
        printf("Depends  { ");
        for(auto i = A->Depends; i; i = i->Next)
        {
            printf("%s", i->_->Name);
            printf(", ");
        }
        printf("}");
        
    }
    if(A->Provides != nullptr)
    {
        NewLine();
        printf("Provides {");
        ++Indentation;
        for(auto i = A->Provides; i; i = i->Next)
        {
            NewLine();
            Print(i->_);
        }
        --Indentation;
        NewLine();
        printf("}");
    }
    --Indentation;
}

static s32
Print_p(FILE *Stream, printf_info const *Info, void const *const *Args)
{
    source_position const *P = *(source_position const *const *)Args[0];
    if(P->Path == nullptr)
    {
        P = &SourcePositionBuiltin;
    }
    s32 Length = fprintf(Stream, "%s:%u:%u", P->Path, P->Line, P->Column);
    return Length;
}

static s32
PrintFormatSingleArgument(printf_info const *Info, size_t N, s32 *ArgTypes)
{
    if(N > 0)
    {
        ArgTypes[0] = PA_POINTER;
    }
    return 1;
}

// PROCEDURES //---------------------------------------------------------------------

node_list *
NodeListAlloc(node *A, node_list *Next)
{
    auto Result = (node_list *)malloc(sizeof(node_list));
    *Result = {._ = A, .Next = Next};
    return Result;
}

node *
NodeAlloc(char const *Name)
{
    auto N = (node *)calloc(1, sizeof(node));
    N->Name = Name;
    return N;
}

node_list *
Find(node_list *L, char const *Name, size_t Length)
{
    for(auto i = L; i; i = i->Next)
    {
        if(strncmp(i->_->Name, Name, Length) == 0)
        {
            return i;
        }
    }
    return nullptr;
}

node_list *
Add(node_list *L, char const *Name)
{
    return NodeListAlloc(NodeAlloc(Name), L);
}

node_list *
ParseConfig()
{
    node_list *Result = nullptr;
    while(Token.Kind != token::kind::NONE)
    {
        token Name = Token;
        ExpectToken(NAME);
        auto NL = Find(Result, Token.NAME._, Token.NAME.Length);
        if(NL == nullptr) NL = Result = Add(Result, strndup(Token.NAME._, Token.NAME.Length));
        auto N = NL->_;
        ExpectToken(BRACE_LEFT);
        while(IsToken(NAME))
        {
            NL = Find(Result, Token.NAME._, Token.NAME.Length);
            if(NL == nullptr) NL = Result = Add(Result, strndup(Token.NAME._, Token.NAME.Length));
            Depends(N, NL->_);
            TokenNext();
            ExpectToken(COMMA);
        }
        ExpectToken(BRACE_RIGHT);
        if(MatchToken(COLON))
        {
            auto Rule = Token;
            ExpectToken(STRING);
            N->Rule = strndup(Rule.STRING._, Rule.STRING.Length);
        }
    }
    return Result;
}

s32
main(s32 ArgCount, char const *Arg[], char const *Env[])
{
    // TODO:
    // + Check for cycles
    //   - We should improve this once we are drawing things
    // + Construct graphs read from files
    // + Parse rules
    //   - Improve rule syntax
    //   - Make it be just files or some other custom things and not like a custom shell language
    //   - pipe + fork + close + dup2 + exec + wait
    // - Actually schedule things and add them to an event queue we wait on
    //   - kqueue
    //   + Dummy nodes that have no build rules but rather serve as a shorthand for multiple dependencies (you can depend on it instead of depending on all of the things it depends on
    //     Useful for something like "all jails up"
    //     - Give a warning if there exists such a node but which does not provide a rule
    // - Provide errors and explanations of why things were not started as they were specified
    // - Think about nodes that need to be started and stay running - like services
    // - Draw things (maybe use one of these libraries)
    //   - https://github.com/Nelarius/imnodes
    //   - https://github.com/rokups/ImNodes
    
    cli_config CliConfig =
    {
        .GraphPath = "test_0.graph",
    };
    ParseConfig(&CliConfig, ArgCount, Arg, Env);
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if(register_printf_function('P', Print_p, PrintFormatSingleArgument) == -1)
    {
        fprintf(stderr, "FATAL: printf custom type register failed\n");
        exit(1);
    }
#pragma clang diagnostic pop
    
    node Init = {.Name = "Init"};
    
    char const *ConfigPath = CliConfig.GraphPath;
    auto Config = FileMap(ConfigPath);
    Initialize(&Parser, ConfigPath, Config);
    
    auto Graph = ParseConfig();
    for(auto i = Graph; i; i = i->Next)
    {
        auto N = i->_;
        if(N->Depends == nullptr)
        {
            Depends(N, &Init);
        }
    }
    
    if(VerifySpread(&Init) == false)
    {
        printf("Exiting...\n");
        exit(1);
    }
    printf("\n");
    for(auto i = Graph; i; i = i->Next)
    {
        if(i->_->CheckState == node::check_state::not_resolved)
        {
            printf("%s is a part of a disconnected cycle\n", i->_->Name);
        }
    }
    
#if 0
    Print(&Init);
    printf("\n");
#endif
    
    Provide(&Init);
    
    FileUnmap(Config);
    
    return 0;
}
