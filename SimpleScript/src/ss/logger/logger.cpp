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
        
        for (size_t i = 0; i < len - 1; ++i)
            std::swap(dst[i], dst[i + 1]);
        
        --len;
        
        for (size_t i = len - 1; i > 0; --i)
            std::swap(dst[i], dst[i - 1]);
        
        std::string months[12] { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
        
        std::size_t i = 0;
        while (i < 12 && months[i] != dst[1])
            ++i;
        
        dst[1] = std::to_string(i + 1);
        
        if (dst[1].length() == 1)
            dst[1] = "0" + dst[1];
            
        char str[dst[3].length() + 1];
        
        strcpy(str, dst[3].c_str());
        
        for (i = 0; i < 2; ++i)
            str[i * 3 + 2] = 46;
        
        dst[3] = std::string(str);
        
        std::ostringstream filename;
        
        filename << "/tmp/SimpleScript.";
        
        for (i = 0; i < 2; ++i)
            filename << dst[i] << "-";
        
        filename << dst[i] << " ";
        filename << dst[3] << ".log";
        
        std::ofstream file;
        
        file.open(filename.str());
        
        file << ss.str();
        
        file.close();
    }

    void logger_write(const std::string str) {
        ss << str;
    }
}
