//
//  file_system_loader.cpp
//  SimpleScript
//
//  Created by Corey Ferguson on 5/4/24.
//

#include "file_system_loader.h"

namespace ss {
    //  NON-MEMBER FIELDS

    void load_file_system(command_processor* cp) {
        cp->set_string("pathSeparator", encode(path_separator()));
        cp->set_read_only("pathSeparator", true);
        cp->consume("pathSeparator");
        
        cp->set_function(new ss::function("closeFile", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));

            return std::to_string(file_close(get_int(argv[0])));
        }));

        cp->set_function(new ss::function("exists", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got 0");

            return std::to_string(exists(decode_raw(get_string(argv[0]))));
        }));

        cp->set_function(new ss::function("file", [](const  size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));

            return std::to_string(file_open(decode_raw(get_string(argv[0]))));
        }));

        cp->set_function(new ss::function("isDir", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));

            return std::to_string(is_dir(decode_raw(get_string(argv[0]))));
        }));

        cp->set_function(new ss::function("isFile", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));

            return std::to_string(is_file(decode_raw(get_string(argv[0]))));
        }));

        // To Do
        cp->set_function(new ss::function("makeDir", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));

            if (argc >= 3)
                expect_error("2 argument(s), got " + std::to_string(argc));

            string filename = decode_raw(get_string(argv[0]));
            bool   is_recursive = false;

            if (argc == 2)
                is_recursive = get_number(argv[0]);

            if (is_recursive)
                mkdirs(filename);

            else if (mkdir(filename.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ))
                throw file_system_exception(std::to_string(errno));

            return null();
        }));

        cp->set_function(new ss::function("major", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));

            return std::to_string(major(get_int(argv[0])));
        }));

        cp->set_function(new ss::function("minor", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));

            return std::to_string(minor(get_int(argv[0])));
        }));

        cp->set_function(new ss::function("move", [](const size_t argc, string* argv) {
            if (argc != 2)
                expect_error("2 argument(s), got " + std::to_string(argc));

            string from_to[2];

            for (size_t i = 0; i < 2; ++i)
                from_to[i] = decode_raw(get_string(argv[i]));

            if (rename(from_to[0].c_str(), from_to[1].c_str()))
                throw file_system_exception(std::to_string(errno));

            return null();
        }));

        // To Do
        cp->set_function(new ss::function("readDir", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));

            if (argc >= 3)
                expect_error("2 argument(s), got " + std::to_string(argc));

            string file_path = decode_raw(get_string(argv[0]));
            bool   is_recursive = false;

            if (argc == 2)
                is_recursive = get_number(argv[1]);

            vector<string> filenames;

            if (is_recursive)
                read_dirs(filenames, file_path);
            else
                read_dir(filenames, file_path);

            ostringstream ss;

            if (filenames.size()) {
                size_t i;
                for (i = 0; i < filenames.size() - 1; ++i)
                    ss << encode(filenames[i]) << separator();

                ss << encode(filenames[i]);
            }

            return ss.str();
        }));

        cp->set_function(new ss::function("readFile", [](const size_t argc, string* argv) {
            if (is_string(argv[0])) {
                if (!argc)
                    expect_error("1 argument(s), got 0");

                if (argc >= 3)
                    expect_error("2 argument(s), got " + std::to_string(argc));

                string filename = decode_raw(get_string(argv[0]));

                if (argc == 2) {
                    string separator = decode_raw(get_string(argv[1]));

                    if (separator.empty())
                        return encode_raw(ss::read(filename));

                    if (separator.length() == tab_length()) {
                        size_t i = 0;
                        while (i < tab_length() && separator[i] == '_')
                            ++i;

                        if (i == tab_length())
                            separator = "\t";
                    }

                    string buffer[1024];

                    int len = ss::read(buffer, filename, separator);

                    if (len == -1)
                        return null();

                    return stringify(len, buffer);
                }

                return encode_raw(ss::read(filename));
            }

            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));

            return encode(read(get_int(argv[0])));
        }));

        // To Do
        cp->set_function(new ss::function("remove", [](const size_t argc, string* argv) {
            if (!argc)
                expect_error("1 argument(s), got " + std::to_string(argc));

            if (argc >= 3)
                expect_error("2 argument(s), got " + std::to_string(argc));

            string filename = decode_raw(get_string(argv[0]));
            bool is_recursive = false;

            if (argc == 2)
                is_recursive = get_number(argv[0]);

            if (is_recursive)
                remove_all(filename);

            else if (remove(filename.c_str()))
                throw file_system_exception(std::to_string(errno));

            return null();
        }));

        cp->set_function(new ss::function("stat", [](const size_t argc, string* argv) {
            if (argc != 1)
                expect_error("1 argument(s), got " + std::to_string(argc));

            string      filename = decode_raw(get_string(argv[0]));
            struct stat fileinfo;

            stat(filename.c_str(), &fileinfo);

            ss::array<string> res;

            res.push(encode("blocks"));
            res.push(std::to_string(fileinfo.st_blocks));

            res.push(encode("deviceId"));
            res.push(std::to_string(fileinfo.st_dev));

            res.push(encode("groupId"));
            res.push(std::to_string(fileinfo.st_gid));

            res.push(encode("links"));
            res.push(std::to_string(fileinfo.st_nlink));

            res.push(encode("mode"));
            res.push(std::to_string(fileinfo.st_mode));

            char buff[100];

            res.push(encode("dateAccessed"));

            strftime(buff, sizeof buff, "%Y-%m-%dT%T", localtime(&fileinfo.st_atimespec.tv_sec));

            res.push(encode(string(buff)));

            res.push(encode("dateModified"));

            strftime(buff, sizeof buff, "%Y-%m-%dT%T", localtime(&fileinfo.st_mtimespec.tv_sec));

            res.push(encode(string(buff)));

            res.push(encode("serialNo"));
            res.push(std::to_string(fileinfo.st_ino));

            res.push(encode("size"));
            res.push(std::to_string(fileinfo.st_size));

            res.push(encode("userId"));
            res.push(std::to_string(fileinfo.st_uid));

            return stringify(res);
        }));

        // To Do
        cp->set_function(new ss::function("write", [](const size_t argc, string* argv) {
            if (argc < 2)
                expect_error("2 argument(s), got " + std::to_string(argc));

            if (ss::is_array(argv[0]))
                type_error(array_t, string_t);

            if (argv[0].empty())
                null_error();

            if (is_string(argv[0])) {
                string filename = decode_raw(argv[0]);

                if (filename.empty())
                    undefined_error(encode(null()));

                string valuev[argv[1].length() + 1];
                size_t valuec = parse(valuev, argv[1]);

                if (valuec == 1) {
                    if (argv[1].empty())
                        null_error();

                    if (is_string(argv[1])) {
                        if (argc >= 3)
                            expect_error("2 argument(s), got " + std::to_string(argc));

                        return std::to_string(write(filename, escape(decode_raw(argv[1]))));
                    } else {
                        double num = stod(argv[1]);

                        if (num != 1)
                            type_error(number_t, string_t);
                            //  double != string

                        if (argc != 3)
                            expect_error("3 argument(s), got " + std::to_string(argc));

                        if (ss::is_array(argv[2]))
                            type_error(array_t, string_t);
                        //  array != string

                        if (argv[2].empty())
                            null_error();

                        if (!is_string(argv[2]))
                            type_error(number_t, string_t);
                        //  double != string

                        string sep = decode_raw(argv[2]);

                        if (sep.empty())
                            undefined_error(encode(null()));

                        if (sep.length() == tab_length()) {
                            size_t i = 0;
                            while (i < tab_length() && sep[i] == '_')
                                ++i;

                            if (i == tab_length())
                                sep = "\t";
                        }

                        return std::to_string(write(filename, valuec, valuev, sep));
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
                        type_error(number_t, string_t);
                    //  double != string

                    string sep = decode_raw(argv[2]);

                    if (sep.empty())
                        undefined_error(encode(null()));

                    if (sep.length() == tab_length()) {
                        size_t i = 0;
                        while (i < tab_length() && sep[i] == '_')
                            ++i;

                        if (i == tab_length())
                            sep = "\t";
                    }

                    return std::to_string(write(filename, valuec, valuev, sep));
                }
            } else {
                if (argc != 2)
                    expect_error("2 argument(s), got " + std::to_string(argc));

                double num = stod(argv[0]);

                if (!is_int(num))
                    type_error(number_t, int_t);
                //  double != int

                if (ss::is_array(argv[1]))
                    type_error(array_t, string_t);
                //  array != string

                if (argv[1].empty())
                    null_error();

                if (!is_string(argv[1]))
                    type_error(number_t, string_t);
                //  double != string

                string data = decode_raw(argv[1]);

                return std::to_string(write((int)num, data));
            }
        }));
    }

     void unload_file_system() {
        file_system_close();
    }
}
