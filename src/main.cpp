
// INCLUDES //-----------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"

// MACROS //-------------------------------------------------------------------------
// TYPES //--------------------------------------------------------------------------

struct node;

struct node_list
{
    node      *_;
    node_list *Next;
};

struct node
{
    char const *Description;
    node_list  *Provides;
    // We don't actually need to track this but rather just a count of things it still depends on. It would work fine, but it would not be easy 
    node_list  *Depends; 
    enum struct state
    {
        not_started,
        started,
        done,
        failed,
    } State;
};

// INTERNAL PROCEDURE DECLARATIONS //------------------------------------------------

static void Depends (node *, node *);
static void Schedule(node *);
static void Provide (node *);

static void DependencyDone(node *, node *);
static node_list *NodeListAlloc(node *, node_list *);

static void PrintDepends (node *);
static void PrintProvides(node *);
static void Print(node *);
static void NewLine();

// INTERNAL VARIABLES //-------------------------------------------------------------

static u32 Indentation = 0;

// VARIABLES //----------------------------------------------------------------------
// INTERNAL PROCEDURES //------------------------------------------------------------

static void
Depends(node *A, node *B) // A depends on B
{
    A->Depends  = NodeListAlloc(B, A->Depends);
    B->Provides = NodeListAlloc(A, B->Provides);
}

static void
Schedule(node *A)
{
    printf("Scheduling %s\n", A->Description);
    // actually schedule A and only "Provide" once we are notified that it is done
    Provide(A);
}

static void
Provide(node *A)
{
    for(node_list *i = A->Provides; i;)
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

static node_list *
NodeListAlloc(node *A, node_list *Next)
{
    auto Result = (node_list *)malloc(sizeof(node_list));
    *Result = {._ = A, .Next = Next};
    return Result;
}

static void
NewLine()
{
    printf("\n%*.s", Indentation, "");
}

static void
Print(node *A)
{
    printf("%s:", A->Description);
    ++Indentation;
    PrintDepends (A);
    PrintProvides(A);
    --Indentation;
}

static void
PrintProvides(node *A)
{
    if(A->Provides == nullptr)
    {
        return;
    }
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

static void
PrintDepends(node *A)
{
    if(A->Depends == nullptr)
    {
        return;
    }
    NewLine();
    printf("Depends  { ");
    for(auto i = A->Depends; i; i = i->Next)
    {
        printf("%s", i->_->Description);
        printf(", ");
    }
    printf("}");
}

// PROCEDURES //---------------------------------------------------------------------

int
main()
{
    // TODO:
    // - Check for cycles
    // - Construct graph from a config
    // - Actually schedule things and add them to an event queue we wait on
    // - Provide errors and explanations of why things were not started as they were specified
    // - Think about nodes that need to be started and stay running - like services
    // - Draw things (maybe use one of these libraries)
    //   - https://github.com/Nelarius/imnodes
    //   - https://github.com/rokups/ImNodes
    
    node Init = {.Description = "Init"};
    node A = {.Description = "A"};
    node B = {.Description = "B"};
    node C = {.Description = "C"};
    node D = {.Description = "D"};
    node E = {.Description = "E"};
    
    Depends(&A, &Init);
    Depends(&B, &A);
    Depends(&C, &A);
    Depends(&E, &A);
    Depends(&C, &B);
    Depends(&D, &C);
    
    Print(&Init);
    printf("\n");
    
    Provide(&Init);
    
    return 0;
}
