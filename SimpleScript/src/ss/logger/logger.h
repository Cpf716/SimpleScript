//
//  logger.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/14/23.
//

#ifndef logger_h
#define logger_h

#include "file_system_exception.h"
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

namespace ss {

    //  NON-MEMBER FUNCTIONS

    void logger_close();

    void logger_open(const std::string filename);

    void logger_write(const std::string message);
}

#endif /* logger_h */
