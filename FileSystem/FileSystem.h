//
// Created by user on 08.11.2021.
//

#ifndef APP_FILESYSTEM_H
#define APP_FILESYSTEM_H

#ifdef FILESYSTEM_STD
    #include <filesystem>
    namespace fs = std::filesystem;
#else
    #ifdef FILESYSTEM_EXPER
        #include <experimental/filesystem>
        namespace fs = std::experimental::filesystem
    #else
        #include "OwnFileSystem.h"
    #endif
#endif

#endif //APP_FILESYSTEM_H
