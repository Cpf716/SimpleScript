//
//  logger.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/14/23.
//

#ifndef logger_h
#define logger_h

#include "file_system.h"           // mkdirs
#include "file_system_exception.h" // file_system_exception
#include "functions.h"             // pad_start
#include "properties.h"            // path_separator

namespace ss {
    //  NON-MEMBER FUNCTIONS

    void logger_close();

    void logger_open(const std::string file_path);

    void logger_write(const std::string message);
}

#endif /* logger_h */
