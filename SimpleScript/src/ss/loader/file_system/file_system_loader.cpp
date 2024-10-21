//
//  file_system_loader.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#include "file_system_loader.h"

namespace ss {
    //  NON-MEMBER FIELDS
    
    vector<function_t*> file_systemv;

    //  NON-MEMBER FUNCTIONS

    void load_file_system() {
        if (file_systemv.size())
            return;
        
        file_systemv.push_back(new ss::function("closeFile", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            return std::to_string(file_close((int)num));
        }));
        
        file_systemv.push_back(new ss::function("exists", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got 0");
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
                //  double != string
            
            string src = decode_raw(argv[0]);
            
            if (src.empty())
                undefined_error(encode(null()));
            
            return std::to_string(exists(src));
        }));
        
        file_systemv.push_back(new ss::function("file", [](const  size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
            //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
            //  double != string
            
            string src = decode_raw(argv[0]);
            
            return std::to_string(file_open(src));
        }));
        
        file_systemv.push_back(new ss::function("isDir", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
            //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
            //  double != string
            
            string dir = decode_raw(argv[0]);
            
            if (dir.empty())
                undefined_error(encode(null()));
            
            return std::to_string(is_dir(dir));
        }));
        
        file_systemv.push_back(new ss::function("isFile", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
                //  double != string
            
            string dir = decode_raw(argv[0]);
            
            if (dir.empty())
                undefined_error(encode(null()));
            
            return std::to_string(is_file(dir));
        }));
        
        file_systemv.push_back(new ss::function("makeDir", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc >= 3)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
                //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
                //  double != string
            
            double recursive = 0;
            
            if (argc == 2) {
                if (ss::is_array(argv[1]))
                    type_error(array_t, double_t);
                    //  array != double
                
                if (argv[1].empty() || is_string(argv[1]))
                    type_error(string_t, double_t);
                    //  string != double
                
                recursive = stod(argv[1]);
            }
            
            string src = decode_raw(argv[0]);
            
            if (recursive)
                mkdirs(src);
            
            else if (mkdir(src.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ))
                throw file_system_exception(strerror(errno));
            
            return encode(to_string(undefined_t));
        }));
        
        file_systemv.push_back(new ss::function("major", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
            //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            return std::to_string(major((int)num));
        }));
        
        file_systemv.push_back(new ss::function("minor", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
            //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
            //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
            //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            return std::to_string(minor((int)num));
        }));
        
        file_systemv.push_back(new ss::function("move", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            for (size_t i = 0; i < 2; ++i) {
                if (ss::is_array(argv[i]))
                    type_error(array_t, string_t);
                //  array != string
                
                if (argv[i].empty())
                    null_error();
                
                if (!is_string(argv[i]))
                    type_error(double_t, string_t);
                //  double != string
                
                argv[i] = decode_raw(argv[i]);
            }
            
            if (rename(argv[0].c_str(), argv[1].c_str()))
                throw file_system_exception(strerror(errno));
            
            return encode(to_string(undefined_t));
        }));
        
        file_systemv.push_back(new ss::function("readDir", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc >= 3)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
            //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
            //  double != string
            
            string dir = decode_raw(argv[0]);
            
            if (dir.empty())
                undefined_error(encode(null()));
            
            double recursive = 0;
            
            if (argc == 2) {
                if (ss::is_array(argv[1]))
                    type_error(array_t, double_t);
                //  array != double
                
                if (argv[1].empty() || is_string(argv[1]))
                    type_error(string_t, double_t);
                //  string != double
                
                recursive = stod(argv[1]);
            }
            
            vector<string> dst;
            
            if (recursive)
                read_dirs(dst, dir);
            else
                read_dir(dst, dir);
            
            ostringstream ss;
            
            if (dst.size()) {
                size_t i;
                for (i = 0; i < dst.size() - 1; ++i)
                    ss << encode(dst[i]) << get_sep();
                
                ss << encode(dst[i]);
            }
            
            return ss.str();
        }));
        
        file_systemv.push_back(new ss::function("readFile", [](const size_t argc, string* argv) {
            if (is_string(argv[0])) {
                if (!argc)
                    expect_error("1 argument(s), got 0");
                
                if (argc >= 3)
                    expect_error("2 argument(s), got " + std::to_string(argc));
                
                if (ss::is_array(argv[0]))
                    type_error(array_t, string_t);
                //  array != string
                
                if (argv[0].empty())
                    null_error();
                
                string str = decode_raw(argv[0]);
                
                if (argc >= 2) {
                    if (ss::is_array(argv[1]))
                        type_error(array_t, string_t);
                    //  array != string
                    
                    if (argv[1].empty())
                        null_error();
                    
                    if (!is_string(argv[1]))
                        type_error(double_t, string_t);
                    //  double != string
                    
                    string sep = decode_raw(argv[1]);
                    
                    if (sep.empty())
                        return encode_raw(ss::read(str));
                    
                    if (sep.length() == get_tablen()) {
                        size_t i = 0;
                        while (i < get_tablen() && sep[i] == '_')
                            ++i;
                        
                        if (i == get_tablen())
                            sep = "\t";
                    }
                    
                    string data[1024];
                    
                    int n = ss::read(data, str, sep);
                    
                    if (n == -1)
                        return encode(to_string(undefined_t));
                    //  undefined
                    
                    return stringify(n, data);
                }
                
                return encode_raw(ss::read(str));
            }
            
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, int_t);
                //  array != int
            
            if (argv[0].empty() || is_string(argv[0]))
                type_error(string_t, int_t);
                //  string != int
            
            double num = stod(argv[0]);
            
            if (!is_int(num))
                type_error(double_t, int_t);
                //  double != int
            
            if (num < 0)
                range_error(std::to_string((int)num));
            
            return encode(read((int)num));
        }));
        
        file_systemv.push_back(new ss::function("remove", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (argc >= 3)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
            //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
            //  double != string
            
            double recursive = 0;
            
            if (argc == 2) {
                if (ss::is_array(argv[1]))
                    type_error(array_t, double_t);
                //  array != double
                
                if (argv[1].empty() || is_string(argv[1]))
                    type_error(string_t, double_t);
                //  string != double
                
                recursive = stod(argv[1]);
            }
            
            string src = decode_raw(argv[0]);
            
            if (recursive)
                remove_all(src);
                
            else if (remove(src.c_str()))
                throw file_system_exception(strerror(errno));
            
            return encode(to_string(undefined_t));
        }));
        
        file_systemv.push_back(new ss::function("stat", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
            //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (!is_string(argv[0]))
                type_error(double_t, string_t);
            //  double != string
            
            string src = decode_raw(argv[0]);
            struct stat fileinfo;
            
            stat(src.c_str(), &fileinfo);
            
            ss::array<string> arr;
            
            arr.push(encode("blocks"));
            arr.push(std::to_string(fileinfo.st_blocks));
            
            arr.push(encode("deviceId"));
            arr.push(std::to_string(fileinfo.st_dev));
            
            arr.push(encode("groupId"));
            arr.push(std::to_string(fileinfo.st_gid));
            
            arr.push(encode("links"));
            arr.push(std::to_string(fileinfo.st_nlink));
            
            arr.push(encode("mode"));
            arr.push(std::to_string(fileinfo.st_mode));
            
            char buff[100];
            
            arr.push(encode("dateAccessed"));
            
            strftime(buff, sizeof buff, "%Y-%m-%dT%T", localtime(&fileinfo.st_atimespec.tv_sec));
            
            arr.push(encode(string(buff)));
            
            arr.push(encode("dateModified"));
            
            strftime(buff, sizeof buff, "%Y-%m-%dT%T", localtime(&fileinfo.st_mtimespec.tv_sec));
            
            arr.push(encode(string(buff)));
            
            arr.push(encode("serialNo"));
            arr.push(std::to_string(fileinfo.st_ino));
            
            arr.push(encode("size"));
            arr.push(std::to_string(fileinfo.st_size));
            
            arr.push(encode("userId"));
            arr.push(std::to_string(fileinfo.st_uid));
            
            return stringify(arr);
        }));
        
        file_systemv.push_back(new ss::function("write", [](const size_t argc, string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + std::to_string(argc));
            
            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);
            //  array != string
            
            if (argv[0].empty())
                null_error();
            
            if (is_string(argv[0])) {
                string str = decode_raw(argv[0]);
                
                if (str.empty())
                    undefined_error(encode(null()));
                
                string valuev[argv[1].length() + 1];
                size_t valuec = parse(valuev, argv[1]);
                
                if (valuec == 1) {
                    if (argv[1].empty())
                        null_error();
                    
                    if (is_string(argv[1])) {
                        if (argc >= 3)
                            expect_error("2 argument(s), got " + std::to_string(argc));
                        
                        return std::to_string(write(str, escape(decode_raw(argv[1]))));
                    } else {
                        double num = stod(argv[1]);
                        
                        if (num != 1)
                            type_error(double_t, string_t);
                            //  double != string
                        
                        if (argc != 3)
                            expect_error("3 argument(s), got " + std::to_string(argc));
                        
                        if (ss::is_array(argv[2]))
                            type_error(array_t, string_t);
                        //  array != string
                        
                        if (argv[2].empty())
                            null_error();
                        
                        if (!is_string(argv[2]))
                            type_error(double_t, string_t);
                        //  double != string
                        
                        string sep = decode_raw(argv[2]);
                        
                        if (sep.empty())
                            undefined_error(encode(null()));
                        
                        if (sep.length() == get_tablen()) {
                            size_t i = 0;
                            while (i < get_tablen() && sep[i] == '_')
                                ++i;
                            
                            if (i == get_tablen())
                                sep = "\t";
                        }
                        
                        return std::to_string(write(str, valuec, valuev, sep));
                    }
                } else {
                    if (argc != 3)
                        expect_error("3 argument(s), got " + std::to_string(argc));
                    
                    if (!is_table(valuec, valuev))
                        type_error(array_t, table_t);
                    //  array != table
                    
                    if (ss::is_array(argv[2]))
                        type_error(array_t, string_t);
                    //  array != string
                    
                    if (argv[2].empty())
                        null_error();
                    
                    if (!is_string(argv[2]))
                        type_error(double_t, string_t);
                    //  double != string
                    
                    string sep = decode_raw(argv[2]);
                    
                    if (sep.empty())
                        undefined_error(encode(null()));
                    
                    if (sep.length() == get_tablen()) {
                        size_t i = 0;
                        while (i < get_tablen() && sep[i] == '_')
                            ++i;
                        
                        if (i == get_tablen())
                            sep = "\t";
                    }
                    
                    return std::to_string(write(str, valuec, valuev, sep));
                }
            } else {
                if (argc != 2)
                    expect_error("2 argument(s), got " + std::to_string(argc));
                
                double num = stod(argv[0]);
                
                if (!is_int(num))
                    type_error(double_t, int_t);
                //  double != int
                
                if (ss::is_array(argv[1]))
                    type_error(array_t, string_t);
                //  array != string
                
                if (argv[1].empty())
                    null_error();
                
                if (!is_string(argv[1]))
                    type_error(double_t, string_t);
                //  double != string
                
                string data = decode_raw(argv[1]);
                
                return std::to_string(write((int)num, data));
            }
        }));
        
        file_systemv.shrink_to_fit();
    }

    void set_file_system(command_processor* cp) {
        for (size_t i = 0; i < file_systemv.size(); ++i)
            cp->set_function(file_systemv[i]);
    }

     void unload_file_system() {
        for (size_t i = 0; i < file_systemv.size(); ++i)
            file_systemv[i]->close();
        
        file_system_close();
    }
}
