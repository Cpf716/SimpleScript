//
//  file_system.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/21/24.
//

#include "file_system.h"

namespace ss {
    //  NON-MEMBER FIELDS

    std::atomic<bool> _is_running = true;

    size_t filec = 0;
    std::vector<std::tuple<int, std::fstream*, size_t>> filev;

    //  NON-MEMBER FUNCTIONS

    int find_file(const int key, const size_t beg = 0, const size_t end = filev.size()) {
        if (beg == end)
            return -1;
        
        size_t len = floor((end - beg) / 2);
        
        if (std::get<0>(filev[beg + len]) == key)
            return (int)(beg + len);
        
        if (std::get<0>(filev[beg + len]) > key)
            return find_file(key, beg, beg + len);
        
        return find_file(key, beg + len + 1, end);
    }

    //  recursively read directory, including directories, themselves
    void _read_dirs(std::vector<std::string>& dst, const std::string src) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::string filename = uuid();
        std::string _filename = uuid();
        std::string abspath = "/tmp/" + filename;
        std::string cmd = "ls -m " + encode(src) +
            " > " + abspath +
            " 2> /tmp/" + _filename;
        
        system(cmd.c_str());
        
        std::ifstream file("/tmp/" + _filename);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        file.close();
        
        if (!ss.str().empty()) {
            remove(abspath.c_str());
            remove(("/tmp/" + _filename).c_str());
            
            throw file_system_exception(ss.str());
        }
        
        ss.str(null());
        ss.clear();
        
        file.open(abspath);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        dst.push_back(src);
        
        std::string line;
        while (getline(file, line))
            ss << line;
        
        if (!ss.str().empty() && ss.str() != src) {
            std::string valuev[ss.str().length() + 1];
            size_t valuec = parse(valuev, ss.str(), ",");
            
            for (size_t i = 0; i < valuec; ++i)
                _read_dirs(dst, src + "/" + valuev[i]);
        }
        
        file.close();
        
        remove(abspath.c_str());
        remove(("/tmp/" + _filename).c_str());
    }

    std::string basename(const std::string src) {
        size_t e = src.length();
        while (e > 0 && src[e - 1] != '.')
            --e;
        
        if (!e)
            e = src.length();
        
        if (src[e - 1] == '.')
            --e;
        
        size_t s = e;
        while (s > 0 && src[s - 1] != '/')
            --s;
        
        return src.substr(s, e - s);
    }

    int file_close(const int src) {
        int i = find_file(src);
        if (i == -1)
            return -1;

        std::get<1>(filev[i])->close();
        
        if (std::get<1>(filev[i])->fail())
            throw file_system_exception(strerror(errno));

        delete std::get<1>(filev[i]);

        filev.erase(filev.begin() + i);
        
        return 0;
    }

    int file_open(const std::string src) {
        std::fstream* file = new std::fstream(src);
        
        if (!file->is_open()) {
            std::ofstream _file(src);
            
            _file.close();
            
            file->open(src);
            
            if (!file->is_open())
                throw file_system_exception(strerror(errno));
        }
        
        filev.push_back(std::tuple<int, std::fstream*, size_t>(filec, file, 0));
        
        return (int)filec++;
    }

    void file_system_close() {
        for (size_t i = 0; i < filev.size(); ++i) {
            std::get<1>(filev[i])->close();
            
            delete std::get<1>(filev[i]);
        }
    }

    bool is_dir(const std::string src) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::string filename = uuid();
        std::string abspath = "/tmp/" + filename;
        std::string cmd = "cd " + src +
            " 2> " + abspath;
        
        system(cmd.c_str());
        
        std::ifstream file(abspath);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        file.close();
        
        remove(abspath.c_str());
        
        return ss.str().empty();
    }

    bool is_file(const std::string src) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::ifstream file(src);
        
        bool flag = !file;
        
        file.close();
        
        if (flag)
            return false;
        
        std::string filename = uuid();
        std::string abspath = "/tmp/" + filename;
        std::string cmd = "cd " + src +
            " 2> " + abspath;
        
        system(cmd.c_str());
        
        file.open(abspath);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        file.close();
        
        remove(abspath.c_str());
        
        return !ss.str().empty();
    }

    bool is_running() {
        return _is_running.load();
    }

    void mkdirs(const std::string src) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::string dst[src.length() + 1];
        size_t len = split(dst, src, "/");
        
        for (size_t i = 0; i < len - 1; ++i)
            swap(dst[i], dst[i + 1]);
        
        --len;
        
        std::ostringstream ss;
        
        for (size_t i = 0; i < len; ++i) {
            ss << "/" << dst[i];
            
            std::ifstream file(ss.str());
            
            if (file) {
                if (!is_dir(ss.str()))
                    throw file_system_exception(ss.str() + " is not a directory");
                
            } else if (mkdir(ss.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ))
                throw file_system_exception(strerror(errno));
            
            file.close();
        }
    }

    std::string read(const int src) {
        int i = find_file(src);
        
        if (i == -1)
            return null();
        
        while (true) {
            if (!is_running())
                return null();
            
            std::get<1>(filev[i])->seekg(0, std::ios::end);
            
            long tellg = std::get<2>(filev[i]);
            long _tellg = std::get<1>(filev[i])->tellg();
            
            if (std::get<1>(filev[i])->fail())
                throw file_system_exception(strerror(errno));
            
            if (tellg < _tellg) {
                size_t len = _tellg - tellg;
                char str[len + 1];
                
                for (size_t j = 0; j < len; ++j) {
                    std::get<1>(filev[i])->seekg(j + tellg, std::ios::beg);
                    
                    str[j] = std::get<1>(filev[i])->get();
                }
                
                if (std::get<1>(filev[i])->fail())
                    throw file_system_exception(strerror(errno));
                
                str[len] = '\0';
                
                std::get<2>(filev[i]) = _tellg;
                
                return std::string(str);
            }
        }
        
        return null();
    }

    std::string read(const std::string src) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::ifstream file;
        
        file.open(src);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        return ss.str();
    }

    void read_dir(std::vector<std::string>& dst, const std::string src) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::string filename = uuid();
        std::string _filename = uuid();
        std::string abspath = "/tmp/" + filename;
        
        std::string cmd = "ls -m " + encode(src) +
            " > " + abspath +
            " 2> /tmp/" + _filename;
        
        system(cmd.c_str());
        
        std::ifstream file("/tmp/" + _filename);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        file.close();
        
        if (!ss.str().empty()) {
            remove(abspath.c_str());
            remove(("/tmp/" + _filename).c_str());
            
            throw file_system_exception(ss.str());
        }
        
        ss.str(null());
        ss.clear();
        
        file.open(abspath);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        std::string line;
        while (getline(file, line))
            ss << line;
        
        std::string valuev[ss.str().length() + 1];
        size_t valuec = parse(valuev, ss.str(), ",");
        
        for (size_t i = 0; i < valuec; ++i)
            dst.push_back(valuev[i]);
        
        file.close();
         
        remove(abspath.c_str());
        remove(("/tmp/" + _filename).c_str());
    }

    //  recursively read directory
    void read_dirs(std::vector<std::string>& dst, const std::string src) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::string filename = uuid();
        std::string _filename = uuid();
        std::string abspath = "/tmp/" + filename;
        std::string cmd = "ls -m " + encode(src) +
            " > " + abspath +
            " 2> /tmp/" + _filename;
        
        system(cmd.c_str());
        
        std::ifstream file("/tmp/" + _filename);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        file.close();
        
        if (!ss.str().empty()) {
            remove(abspath.c_str());
            remove(("/tmp/" + _filename).c_str());
            
            throw file_system_exception(ss.str());
        }
        
        ss.str(null());
        ss.clear();
        
        file.open(abspath);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        std::string line;
        while (getline(file, line))
            ss << line;
        
        if (ss.str() == src)
            dst.push_back(src);
        
        else if (!ss.str().empty()) {
            std::string valuev[ss.str().length() + 1];
            size_t valuec = parse(valuev, ss.str(), ",");
            
            for (size_t i = 0; i < valuec; ++i)
                read_dirs(dst, src + "/" + valuev[i]);
        }
        
        file.close();
        
        remove(abspath.c_str());
        remove(("/tmp/" + _filename).c_str());
    }

    //  recursively remove file or directory
    void remove_all(const std::string src) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::vector<std::string> dst;
        
        _read_dirs(dst, src);
        
        for (int i = 1; i < dst.size(); ++i)
            for (size_t j = i - 1; j >= 0 && dst[j] > dst[j + 1]; --j)
                swap(dst[j], dst[j - 1]);
        
        for (size_t i = 0; i < floor(dst.size() / 2); ++i)
            swap(dst[i], dst[dst.size() - i - 1]);
        
        for (size_t i = 0; i < dst.size(); ++i)
            if (remove(dst[i].c_str()))
                throw file_system_exception(strerror(errno));
    }

    void set_is_running(const bool is_running) {
        _is_running.store(is_running);
    }

    std::string uuid() {
        std::ostringstream ss;
        
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<> uid;
        
        for (size_t i = 0; i < 8; ++i)
            ss << std::hex << uid(gen) % 16;
        
        ss << '-';
        
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 4; ++j)
                ss << uid(gen) % 16;
            
            ss << '-';
        }
        
        for (size_t i = 0; i < 12; ++i)
            ss << std::hex << uid(gen) % 16;
        
        return ss.str();
    }

    int write(const int dst, const std::string src) {
        int i = find_file(dst);

        if (i == -1)
            return -1;

        (* std::get<1>(filev[i])) << src;
        (* std::get<1>(filev[i])).flush();
        
        if (std::get<1>(filev[i])->fail())
            throw file_system_exception(strerror(errno));

        std::get<2>(filev[i]) = std::get<1>(filev[i])->tellg();

        return (int)src.length();
    }

    size_t write(const std::string dst, const std::string src) {
#if DEBUG_LEVEL
        assert(dst.length());
#endif
        std::ofstream file;
        
        file.open(dst);
        
        if (!file.is_open())
            throw file_system_exception(strerror(errno));
        
        file << src;
        
        if (file.fail())
            throw file_system_exception(strerror(errno));
        
        file.close();
        
        if (file.fail())
            throw file_system_exception(strerror(errno));
        
        return src.length();
    }
}
