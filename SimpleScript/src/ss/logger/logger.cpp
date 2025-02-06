//
//  logger.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 10/14/23.
//

#include "logger.h"

namespace ss {
    // NON-MEMBER FIELDS

    std::ofstream file;

    // NON-MEMBER FUNCTIONS
        
    void logger_close() {
        file.close();
        
        if (file.fail())
            throw file_system_exception(strerror(errno));
    }

    void logger_open(const std::string file_path) {
        std::ostringstream ss;
        
        ss << file_path << path_separator();
        
        time_t now = time(0);
        char*  dt = ctime(&now);
        
        std::string tokens[strlen(dt) + 1];
        
        ss::_tokens(tokens, std::string(dt));
        
        // index abbreviated month
        std::string months[12];
        
        ss::months(months);
        
        size_t month;
        for (month = 0; month < 12; month++) {
            size_t pos = 0;
            while (pos < 3 && tokens[1][pos] == months[month][pos])
                ++pos;
            
            if (pos == 3)
                break;
        }
        
        tokens[1] = std::to_string(++month);
        
        ss << tokens[4];
        
        for (size_t i = 1; i < 3; i++)
            ss << pad_start(tokens[i], 2, "0");
        
        if (!exists(ss.str())) {
            try {
                mkdirs(ss.str());
            } catch (file_system_exception& e) {
                throw file_system_exception(strerror(std::stoi(e.what())));
            }
        }
        
        ss << path_separator();

        std::string time[3];
        
        split(time, tokens[3], ":");
        
        for (size_t i = 0; i < 3; ++i)
            ss << time[i];
        
        ss << ".log";
        
        file.open(ss.str());
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
    }

    void logger_write(const std::string message) {
        file << message << std::endl;
        file.flush();
        
        if (file.fail())
            throw file_system_exception(strerror(errno));
    }
}
