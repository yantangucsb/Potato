#include <stdlib.h>
#include "OpenFileTable.h"

ErrorCode getOpenFileEntry(OpenFileTable* open_file_table, char* path_name, OpenFileEntry* open_file_entry){
    open_file_entry = open_file_table->head;
    while(open_file_entry != NULL){
        if(strcmp(open_file_entry->filePath, path_name) == 0)
            break;
        open_file_entry = open_file_entry->next;
    }

    return Success;
}
