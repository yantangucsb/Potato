/*
 * Directory entry
 * By Yan
 */

#include "Parameters.h"

typedef struct DirEntry {
    BYTE key[FILE_NAME_LENGTH];
    size_type inodeId;
} DirEntry;
