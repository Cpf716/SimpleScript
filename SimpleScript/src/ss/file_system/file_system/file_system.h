//
//  file_system.h
//  SimpleScript
//
//  Created by Corey Ferguson on 5/21/24.
//

#ifndef file_system_h
#define file_system_h

#include "preferences.h"
#include <random>
#include <sys/stat.h>       //  mkdir

namespace ss {
    // NON-MEMBER FUNCTIONS

    std::string basename(const std::string src);

    int file_close(const int src);

    int file_open(const std::string src);

    void file_system_close();

    bool is_dir(const std::string src);

    bool is_file(const std::string src);

    void mkdirs(const std::string src);

    std::string read(const int src);

    std::string read(const std::string src);

    void read_dir(std::vector<std::string>& dst, const std::string src);

    void read_dirs(std::vector<std::string>& dst, const std::string src);

    void remove_all(const std::string src);

    std::string uuid();

    int write(const int dst, const std::string src);

    size_t write(const std::string dst, const std::string src);
}

#endif /* file_system_h */
