//
//  logger.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 10/14/23.
//

#include "logger.h"

namespace ss {
    std::ofstream file;
        
    void logger_close() {
        file.close();
        
        if (file.fail())
            throw file_system_exception(strerror(errno));
    }

    void logger_open(const std::string filename) {
        if (file.is_open())
            throw file_system_exception("file.is_open()");
        
        file.open(filename);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
    }

    void logger_write(const std::string message) {
        file << message;
        file.flush();
        
        if (file.fail())
            throw file_system_exception(std::strerror(errno));
    }
}
