# SimpleScript

SimpleScript is an interpreted, procedural programming language.

It features three base types (number, string, and array) and seven subtypes (double, int, string, char, array, dictionary, and table) as well as 88 operators and 22 built-in functions.

Additionally, SimpleScript supports MySQL DBMS and TCP socket programming.

## Configuration

1. Clone the repository to your `~/` Home directory

2. Use iODBC Administrator to install the C++ MySQL connector

3. Open a new Terminal window and run `sudo ln -s /usr/local/mysql-connector-c++-8.1.0/lib64/libmysqlcppconn.9.8.1.0.dylib /usr/local/lib` to create a symbolic link to the connector

* If you have macOS Sonoma 14.0+ installed, navigate to `Build Settings/Targets/Linking/Run Search Paths/` and add `/usr/local/lib`.

4. Open the `SimpleScript.xcodeproj` file in Xcode, navigate to `Build Settings/Linking/Other Linker Flags/Debug/`, and add `/usr/local/mysql-connector-c++-8.1.0/lib64/libmysqlcppconn.9.8.1.0.dylib`

5. Similarly, navigate to `Build Settings/Search Paths/Header Search Paths/Debug`, and add `/usr/local/mysql-connector-c++-8.1.0/include/jdbc`

6. Finally, navigate to `Build Settings/Search Paths/Library Search Paths/Debug`, and add `/usr/local/mysql-connector-c++-8.1.0/lib64`

## Building and Running

1. Open Terminal, and run the following commands:
    ```
    cd /Library/Application\ Support
    sudo mkdir SimpleScript
    ln -s /Users/<user>/SimpleScript/SimpleScript/ssl/public/ /Library/Application\ Support/SimpleScript/ssl
    ```

2. Open the `SimpleScript.xcodeproj` file in Xcode 

3. Build the project (only required once)

4. Navigate to `/Users/<user>/Library/Developer/Xcode/DerivedData`

5. Locate the `SimpleScript` directory, and drill down to `Build/Products/Debug/`

6. Open a new Terminal window from the `Debug` directory and run `./SimpleScript`, passing the full path of your SimpleScript file as the first argument

* Additional arguments are passed to the `argv` variable within your SimpleScript file

## Additional Resources

The `/example` directory contains a full suite of sample code to expedite your learning. The SimpleScript Standard Library (SSL) is an excellent resource as well.

Thank you, and happy coding!
