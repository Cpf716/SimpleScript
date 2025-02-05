//
//  file_system.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/21/24.
//

#include "file_system.h"

namespace ss {
    //  NON-MEMBER FIELDS

    // Begin Enhancement 1-1 - Thread Safety - 2025-02-05
    std::mutex                                          file_mutex;
    // End Enhancement 1-1
    size_t                                              filec = 0;
    std::vector<std::tuple<int, std::fstream*, size_t>> filev;

    //  NON-MEMBER FUNCTIONS

    //  recursively read directory, including directories, themselves
    void _read_dirs(std::vector<std::string>& dst, const std::string src) {
    #if DEBUG_LEVEL
        assert(src.length());
    #endif
        std::string filename = uuid();
        std::string _filename = uuid();
        std::string abspath = path(temporary(), filename);
        std::string cmd = "ls -m " + encode(src) +
            " > " + abspath +
            " 2> " + path(temporary(), _filename);
        
        system(cmd.c_str());
        
        std::ifstream file(path(temporary(), _filename));
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        file.close();
        
        if (!ss.str().empty()) {
            remove(abspath.c_str());
            remove(path(temporary(), _filename).c_str());
            
            throw file_system_exception(ss.str());
        }
        
        ss.str(null());
        ss.clear();
        
        file.open(abspath);
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
        dst.push_back(src);
        
        std::string line;
        while (getline(file, line))
            ss << line;
        
        if (!ss.str().empty() && ss.str() != src) {
            std::string valuev[ss.str().length() + 1];
            size_t valuec = parse(valuev, ss.str(), ",");
            
            for (size_t i = 0; i < valuec; ++i)
                _read_dirs(dst, src + path_separator() + valuev[i]);
        }
        
        file.close();
        
        remove(abspath.c_str());
        remove(path(temporary(), _filename).c_str());
    }

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

    bool exists(const std::string src) {
    #if DEBUG_LEVEL
        assert(src.length());
    #endif
        std::ifstream file;
        
        file.open(src);
        
        bool exists = !!file;
        
        file.close();
        
        return exists;
    }

    int file_close(const int src) {
        // Begin Enhancement 1-1 - Thread Safety - 2025-02-05
        file_mutex.lock();
        
        int i = find_file(src);
        
        if (i == -1) {
            file_mutex.unlock();
            
            return -1;
        }

        std::get<1>(filev[i])->close();
        
        if (std::get<1>(filev[i])->fail()) {
            file_mutex.unlock();
            
            throw file_system_exception(std::to_string(errno));
        }

        delete std::get<1>(filev[i]);

        filev.erase(filev.begin() + i);
        
        file_mutex.unlock();
        // End Enhancement 1-1
        
        return 0;
    }

    int file_open(const std::string src) {
        std::fstream* file = new std::fstream(src);
        
        if (!file->is_open()) {
            std::ofstream _file(src);
            
            _file.close();
            
            file->open(src);
            
            if (!file->is_open())
                throw file_system_exception(std::to_string(errno));
        }
        
        // Begin Enhancement 1-1 - Thread Safety - 2025-02-05
        file_mutex.lock();
        
        filev.push_back({ filec, file, 0 });
        
        int result = (int)filec++;
        
        file_mutex.unlock();
        
        return result;
        // End Enhancement 1-1
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
        std::string abspath = path(temporary(), uuid());
        std::string cmd = "cd " + src +
            " 2> " + abspath;
        
        system(cmd.c_str());
        
        std::ifstream file(abspath);
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
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
        std::string abspath = path(temporary(), filename);
        std::string cmd = "cd " + src +
            " 2> " + abspath;
        
        system(cmd.c_str());
        
        file.open(abspath);
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        file.close();
        
        remove(abspath.c_str());
        
        return !ss.str().empty();
    }

    void mkdirs(const std::string file_path) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::string dst[file_path.length() + 1];
        size_t len = split(dst, file_path, path_separator());
        
        for (size_t i = 0; i < len - 1; ++i)
            swap(dst[i], dst[i + 1]);
        
        --len;
        
        std::ostringstream ss;
        
        for (size_t i = 0; i < len; ++i) {
            ss << path_separator() << dst[i];
            
            std::ifstream file(ss.str());
            
            if (file) {
                if (!is_dir(ss.str()))
                    throw file_system_exception(ss.str() + " is not a directory");
                
            } else if (mkdir(ss.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ))
                throw file_system_exception(std::to_string(errno));
            
            file.close();
        }
    }

    std::string path(const std::string a, const std::string b) {
        std::function<size_t(std::string*, const std::string)> tokens = [](std::string* target, const std::string source) {
            size_t len = split(target, source, path_separator()),
                   i = 0;
            
            while (i < len) {
                if (target[i].empty()) {
                    for (size_t j = i; j < len - 1; ++j)
                        swap(target[j], target[j + 1]);
                    
                    --len;
                } else
                    ++i;
            }
            
            
            return len;
        };
        
        std::string av[a.length() + 1];
        size_t      ac = tokens(av, a);
        
        std::string bv[b.length() + 1];
        size_t      bc = tokens(bv, b);
        
        std::ostringstream ss;
        
        ss << path_separator();
        
        for (size_t i = 0; i < ac; ++i)
            ss << av[i] << path_separator();
        
        if (bc) {
            size_t i;
            for (i = 0; i < bc - 1; ++i)
                ss << bv[i] << path_separator();
            
            ss << bv[i];
        }
        
        return ss.str();
    }

    std::string read(const int src) {
        // Begin Enhancement 1-1 - Thread Safety - 2025-02-05
        file_mutex.lock();
        
        int pos = find_file(src);
        
        if (pos == -1) {
            file_mutex.unlock();
            
            return null();
        }
        
        std::tuple<int, std::fstream*, size_t> file = filev[pos];
        
        file_mutex.unlock();
        // End Enhancement 1-1
        
        while (true) {
            std::get<1>(file)->seekg(0, std::ios::end);
            
            long tellg = std::get<2>(file);
            long _tellg = std::get<1>(file)->tellg();
            
            if (std::get<1>(file)->fail())
                throw file_system_exception(std::to_string(errno));
            
            if (tellg < _tellg) {
                size_t len = _tellg - tellg;
                char str[len + 1];
                
                for (size_t j = 0; j < len; ++j) {
                    std::get<1>(file)->seekg(j + tellg, std::ios::beg);
                    
                    str[j] = std::get<1>(file)->get();
                }
                
                if (std::get<1>(file)->fail())
                    throw file_system_exception(std::to_string(errno));
                
                str[len] = '\0';
                
                std::get<2>(file) = _tellg;
                
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
            throw file_system_exception(std::to_string(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        return ss.str();
    }

    int read(std::string* dst, const std::string src, const std::string sep) {
    #if DEBUG_LEVEL
        assert(dst != NULL);
        assert(!src.empty());
        assert(!sep.empty());
    #endif
        std::ifstream file;
        
        file.open(src);
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
        int n = 0;
        
        std::string line;
        while (getline(file, line))
            dst[n++] = line;
        
        file.close();
        
        size_t p = n;
        
        n = (int)merge(n, dst, "\n");
        
        size_t i = 0, m = 0;
        while (i < n) {
            std::string valuev[dst[i].length() + 1];
            size_t valuec = parse(valuev, dst[i], sep);
            
            if (valuec > m)
                m = valuec;
            
            dst[n] = std::to_string(valuec);
            
            for (size_t j = n; j > i; --j)
                std::swap(dst[j], dst[j - 1]);
            
            ++n;
            ++i;
            
            dst[i] = raw(valuev[0], sep);
            
            for (size_t j = 1; j < valuec; ++j) {
                dst[n] = raw(valuev[j], sep);
                
                for (size_t k = n; k > i + j; --k)
                    std::swap(dst[k], dst[k - 1]);
                
                ++n;
            }
            
            i += valuec;
        }
        
        ++m;
        
        for (size_t i = 0; i < p; ++i) {
            int j = stoi(dst[i * m]);
            for (size_t k = 0; k < m - j - 1; ++k) {
                dst[n] = null();
                
                for (size_t l = n; l > i * m + j + k + 1; --l)
                    std::swap(dst[l], dst[l - 1]);
                
                n++;
            }
        }
        
        dst[n] = std::to_string(m);
        
        for (size_t i = n; i > 0; --i)
            std::swap(dst[i], dst[i - 1]);
        
        ++n;
        
        return n;
    }

    void read_dir(std::vector<std::string>& dst, const std::string src) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::string filename = uuid();
        std::string _filename = uuid();
        std::string abspath = path(temporary(), filename);
        
        std::string cmd = "ls -m " + encode(src) +
            " > " + abspath +
            " 2> " + path(temporary(), _filename);;
        
        system(cmd.c_str());
        
        std::ifstream file(path(temporary(), _filename));
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        file.close();
        
        if (!ss.str().empty()) {
            remove(abspath.c_str());
            remove(path(temporary(), _filename).c_str());
            
            throw file_system_exception(ss.str());
        }
        
        ss.str(null());
        ss.clear();
        
        file.open(abspath);
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
        std::string line;
        while (getline(file, line))
            ss << line;
        
        std::string valuev[ss.str().length() + 1];
        size_t valuec = parse(valuev, ss.str(), ",");
        
        for (size_t i = 0; i < valuec; ++i)
            dst.push_back(valuev[i]);
        
        file.close();
         
        remove(abspath.c_str());
        remove(path(temporary(), _filename).c_str());
    }

    //  recursively read directory
    void read_dirs(std::vector<std::string>& dst, const std::string src) {
#if DEBUG_LEVEL
        assert(src.length());
#endif
        std::string filename = uuid();
        std::string _filename = uuid();
        std::string abspath = path(temporary(), filename);
        std::string cmd = "ls -m " + encode(src) +
            " > " + abspath +
            " 2> " + path(temporary(), _filename);
        
        system(cmd.c_str());
        
        std::ifstream file(path(temporary(), _filename));
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
        std::ostringstream ss;
        
        ss << file.rdbuf();
        
        file.close();
        
        if (!ss.str().empty()) {
            remove(abspath.c_str());
            remove(path(temporary(), _filename).c_str());
            
            throw file_system_exception(ss.str());
        }
        
        ss.str(null());
        ss.clear();
        
        file.open(abspath);
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
        std::string line;
        while (getline(file, line))
            ss << line;
        
        if (ss.str() == src)
            dst.push_back(src);
        
        else if (!ss.str().empty()) {
            std::string valuev[ss.str().length() + 1];
            size_t valuec = parse(valuev, ss.str(), ",");
            
            for (size_t i = 0; i < valuec; ++i)
                read_dirs(dst, src + path_separator() + valuev[i]);
        }
        
        file.close();
        
        remove(abspath.c_str());
        remove(path(temporary(), _filename).c_str());
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
                throw file_system_exception(std::to_string(errno));
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
        // Begin Enhancement 1-1 - Thread Safety - 2025-02-05
        file_mutex.lock();
        
        int pos = find_file(dst);

        if (pos == -1) {
            file_mutex.unlock();
            
            return -1;
        }

        (* std::get<1>(filev[pos])) << src;
        (* std::get<1>(filev[pos])).flush();
        
        if (std::get<1>(filev[pos])->fail())
            throw file_system_exception(std::to_string(errno));

        std::get<2>(filev[pos]) = std::get<1>(filev[pos])->tellg();
        
        file_mutex.unlock();
        // End Enhancement 1-1

        return (int)src.length();
    }

    size_t write(const std::string dst, const std::string src) {
#if DEBUG_LEVEL
        assert(dst.length());
#endif
        std::ofstream file;
        
        file.open(dst);
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
        file << src;
        
        if (file.fail())
            throw file_system_exception(std::to_string(errno));
        
        file.close();
        
        if (file.fail())
            throw file_system_exception(std::to_string(errno));
        
        return src.length();
    }

    size_t write(const std::string filename, const size_t len, std::string* arr, const std::string sep) {
    #if DEBUG_LEVEL
        assert(!filename.empty());
        assert(len);
        assert(arr != NULL);
    #endif
        std::ostringstream ss;
        
        for (size_t i = 0; i < (len - 1) / stoi(arr[0]); ++i) {
            for (size_t j = 0; j < stoi(arr[i * stoi(arr[0]) + 1]) - 1; ++j) {
                std::string value = arr[i * stoi(arr[0]) + j + 2];
                
                value = escape(decode(value));
                
                size_t k;
                for (k = 0; k < value.length(); ++k) {
                    if (value[k] == '\"')
                        break;
                    
                    if (k > value.length() - sep.length())
                        continue;
                    
                    size_t l = 0;
                    while (l < sep.length() && sep[l] == value[k + l])
                        ++l;
                    
                    if (l == sep.length())
                        break;
                }
                
                if (k != value.length())
                    value = encode(value);
                
                ss << value << sep;
            }
            
            std::string value = arr[i * stoi(arr[0]) + stoi(arr[i * stoi(arr[0]) + 1]) + 1];
            
            value = escape(decode(value));
            
            size_t j;
            for (j = 0; j < value.length(); ++j) {
                if (value[j] == '\"')
                    break;
                
                if (j > value.length() - sep.length())
                    continue;
                
                size_t k = 0;
                while (k < sep.length() && sep[k] == value[j + k])
                    ++k;
                
                if (k == sep.length())
                    break;
            }
            
            if (j != value.length())
                value = encode(value);
                
            ss << value << "\n";
        }
        
        std::ofstream file;
        
        file.open(filename);
        
        if (!file.is_open())
            throw file_system_exception(std::to_string(errno));
        
        file << ss.str();
        
        if (file.fail())
            throw file_system_exception(std::to_string(errno));
        
        file.close();
        
        if (file.fail())
            throw file_system_exception(std::to_string(errno));
        
        return ss.str().length();
    }
}
