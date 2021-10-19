
// INCLUDES //-----------------------------------------------------------------------

#include "utilities.h"
#include <unistd.h>
#include <err.h>
#include <sys/mman.h>
#include <fcntl.h>

// MACROS //-------------------------------------------------------------------------
// TYPES //--------------------------------------------------------------------------
// INTERNAL PROCEDURE DECLARATIONS //------------------------------------------------
// INTERNAL VARIABLES //-------------------------------------------------------------
// VARIABLES //----------------------------------------------------------------------
// INTERNAL PROCEDURES //------------------------------------------------------------
// PROCEDURES //---------------------------------------------------------------------

file
FileMap(char const *Path)
{
    fd Config = open(Path, O_RDONLY);
    if(Config == -1) err(1, "open");
    size_t Length = 1;
    void *Data = mmap(nullptr, Length, PROT_READ, MAP_PRIVATE, Config, 0);
    if(Data == nullptr) err(1, "mmap");
    close(Config);
    return {.Data = Data, .Length = Length};
}

void
FileUnmap(file File)
{
    munmap(File.Data, File.Length);
}
