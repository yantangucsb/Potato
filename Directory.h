/*
 * Directory entry
 * By Yan
 */

#include "Parameters.h"

typedef struct DirENtry {
    BYTE key[];
    size_type InodeId;
} DirEntry
