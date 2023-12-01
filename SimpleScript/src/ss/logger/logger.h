//
//  logger.h
//  SimpleScript
//
//  Created by Corey Ferguson on 10/14/23.
//

#ifndef logger_h
#define logger_h

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

namespace ss {
    //  NON-MEMBER FIELDS

    const bool ENABLE_LOGGING = false;

    //  NON-MEMBER FUNCTIONS

    void logger_close();

    void logger_write(const std::string str);
}

#endif /* logger_h */
