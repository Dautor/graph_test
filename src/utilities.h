#pragma once

// INCLUDES //-----------------------------------------------------------------------

#include <stdint.h>
#include <stddef.h>

// MACROS //-------------------------------------------------------------------------

#if __has_builtin(__builtin_offsetof)
#define OffsetOf(x, y) __builtin_offsetof(x, y)
#else
#define OffsetOf(x, y) (u64)(&((x *)nullptr)->y)
#endif

#define ArrayCount(x) (sizeof(x)/sizeof(*(x)))

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

// TYPES //--------------------------------------------------------------------------

typedef  uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef   int8_t  s8;
typedef  int16_t s16;
typedef  int32_t s32;
typedef  int64_t s64;
typedef    float f32;
typedef   double f64;
typedef      s32  fd;

struct file
{
    void  *Data;
    size_t Length;
};

struct source_position
{
    const char *Path;
    u32 Line;
    u32 Column;
};

struct node;

struct node_list
{
    node      *_;
    node_list *Next;
};

struct node
{
    char const *Name;
    node_list  *Provides;
    node_list  *Depends;  // We don't actually need to track this but rather just a count of things it still depends on. It would work fine, but it would not be easy 
    enum struct state
    {
        not_started,
        started,
        done,
        failed,
    } State;
    enum struct check_state
    {
        not_resolved,
        resolving,
        resolved,
    } CheckState;
};

// EXTERNAL VARIABLES //-------------------------------------------------------------
// PROCEDURE DECLARATIONS //---------------------------------------------------------

file FileMap  (char const *Path);
void FileUnmap(file);

node_list *NodeListAlloc(node *, node_list *);
node      *NodeAlloc    (char const *Name);

// INLINE PROCEDURES //--------------------------------------------------------------

