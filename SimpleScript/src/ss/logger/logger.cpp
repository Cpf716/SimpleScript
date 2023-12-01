//
//  logger.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 10/14/23.
//

#include "logger.h"

namespace ss {
    std::ostringstream ss;
        
    void logger_close() {
        time_t now = time(0);
        
        char* dt = ctime(&now);
        
        std::string src = std::string(dt);
        std::string dst[src.length() + 1];
        
        std::size_t beg = 0, len = 0;
        
        for (std::size_t end = 0; end < src.length(); ++end) {
            while (end < src.length() && isspace(src[end]))
                ++end;
            
            beg = end;
            while (end < src.length() && !isspace(src[end]))
                ++end;
            
            if (beg != end)
                dst[len++] = src.substr(beg, end - beg);
        }
        
        beg = 0;
        for (std::size_t end = 0; end < dst[3].length(); ++end) {
            if (dst[3][end] == ':') {
                dst[len] = dst[3].substr(beg, end - beg);
                beg = end + 1;
                
                for (size_t i = len; i > len - 1; --i)
                    std::swap(dst[i], dst[i - 1]);
                
                ++len;
            }
        }
        
        dst[len] = dst[3].substr(beg);
        
        for (size_t i = len; i > len - 1; --i)
            std::swap(dst[i], dst[i - 1]);
        
        for (size_t i = 3; i < len; ++i)
            std::swap(dst[i], dst[i + 1]);
        
        std::string months[12] { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
        
        size_t i = 0;
        while (i < 12 && months[i] != dst[1])
            ++i;
        
        std::ostringstream filename;
        
        filename << "/tmp/";
        
        filename << dst[6] << (i + 1) << dst[2];
        
        for (size_t j = 0; j < 3; ++j)
            filename << dst[j + 3];
        
        filename << ".log";
        
        std::ofstream file;
        
        file.open(filename.str());
        
        file << ss.str();
        
        file.close();
    }

    void logger_write(const std::string str) {
        ss << str;
    }
}
