# SimpleScript

## Functions

<!-- -->

### File System

#### `closeFile(filedes: int): int`

Precondition:   none<br>
Postcondition:  closes file; returns `0` on success, otherwise returns `-1` if no such file exists or throws an exception if an error occurs 

#### `exists(src: string): int`

Precondition:   `src`<br>
Postcondition:  returns `1` if a file or directory exists at `src`, otherwise returns `0`

#### `file(filename: string): int`

Precondition:   `filename`<br>
Postcondition:  opens a file at `filename`; returns its file descriptor, otherwise throws an exception if an error occurs

#### `isDir(src: string): int`

Precondition:   `src`<br>
Postcondition:  returns `1` if `src` is a directory; otherwise returns `0`

#### `isFile(src: string): int`

Precondition:   `src`<br>
Postcondition:  returns `1` if `src` is a file; otherwise returns `0`

#### `major(num: int): int`

Precondition:   `num >= 0`<br>
Postcondition:  returns the major number from `num`

#### `makeDir(src: string): null`

Precondition:   `src`<br>
Postcondition:  creates directory `src`; returns `null`, otherwise throws an exception if an error occurs

#### `minor(num: int): int`

Precondition:   `num >= 0`<br>
Postcondition:  returns the minor number from `num`

#### `move(old: string, new: string): null`

Precondition: `old && new`<br>
Postcondition:  renames file or directory `old` to `new`; returns `null`, otherwise throws an exception if an error occurs

#### `readFile(filedes: int): string`

Precondition:   none<br>
Postcondition:  reads file; returns its new contents, otherwise returns `null` if no such file exists 

#### `readFile(filename: string): string`
#### `readFile(filename: string, sep: string): string | table`

Precondition:   `filename && sep !== null`<br>
Postcondition:  returns data read from the file at `filename`, otherwise throws an exception if an error occurs

#### `remove(filename: string): int`

Precondition:   `filename`<br>
Postcondition:  removes the file or directory at `filename`; returns `null`, otherwise throws an exception if an error occurs

#### `stat(filename: string): dictionary`

Precondition:   `filename`<br>
Postcondition:  returns information on the file or directory at `filename`

#### `write(filedes: int, data: string): int`

Precondition:   `data`<br>
Postcondition:  writes `data` to file; returns the number of bytes written, otherwise returns `-1` if no such file exists or throws an exeption if an error occurs

#### `write(filename: string, data: array | string, sep: string = ""): null`

Precondition:   `filename && data !== null && sep !== null`<br>
Postcondition:  writes `data` to file; returns the number of bytes written, otherwise throws an exception if an error occurs

<!-- -->

### MySQL

#### `connect(host: string, usr: string, pwd: string = ""): int`

Precondition:   `host && usr && pwd !== null`<br>
Postcondition:  returns the descriptor of the MySQL connection to `hostName`, otherwise throws an exception if an error occurs

#### `closeConnection(con: int): int`

Precondition:   none<br>
Postcondition:  returns `0` upon successful closure of the MySQL connection `con`, otherwise returns `-1` if no such connection exists or throws an exception if an error occurs

#### `preparedQuery(con: int, sql: string, ...args): table`

Precondition:   `sql`<br>
Postcondition:  returns the result set of a prepared query, otherwise returns `null` if no such connection exists or throws an exception if an error occurs

#### `preparedUpdate(con: int, sql: string, ...args): int`

Precondition:   `sql`<br>
Postcondition:  returns the number of rows updated by a prepared update, otherwise returns `-1` if no such connection exists or throws an exception if an error occurs

#### `query(con: int, sql: string): table`

Precondition:   `sql`<br>
Postcondition:  returns the result set of query `sql`, otherwise returns `null` if no such connection exists or throws an exception if an error occurs

#### `setSchema(con: int, schema: string): int`

Precondition:   `schema`<br>
Postcondition:  sets the schema for connection `con`; returns `0`, otherwise returns `-1` if no such connection exists or throws an exception if an error occurs

#### `update(con: int, sql: string): int`

Precondition:   `sql`<br>
Postcondition:  returns the number of rows updated by query `sql`, otherwise returns `-1` if no such connection exists or throws an exception if an error occurs


<!-- -->

### Socket

#### `closeTcp(fildes: int): int`

Precondition:   none<br>
Postcondition:  returns `0` upon successful closure of the socket `fildes`, otherwise returns `-1` if no such socket exists or throws an exception if an error occurs

#### `closeUdp(fildes: int): int`

Precondition:   none<br>
Postcondition:  returns `0` upon successful closure of the socket `fildes`, otherwise returns `-1` if no such socket exists or throws an exception if an error occurs

#### `listen(fildes: int, port: int): int`

Precondition:   `port >= 0`<br>
Postcondition:  listens to messages received by the socket `fildes` and forwards them to `port`; returns `0` on success, otherwise returns `-1` if no such socket exists or throws an exception if an error occurs

#### `poll(fildes: int): array | null`

Precondition:   none<br>
Postcondition:  returns an array of file descriptors identifying clients of the server `fildes`, otherwise returns `null` if no such socket exists, the server has no clients, or an error occurs

#### `recv(fildes: int, timeout: int = 0): string`

Precondition:   none<br>
Postcondition:  returns data received from the socket `fildes`, otherwise returns `null` if disconnected or throws an exception if an error occurs (blocking)

#### `recvFrom(fildes: int, timeout: int = 0): string`

Precondition:   none<br>
Postcondition:  returns data received from the socket `fildes`, otherwise returns `null` if disconnected or throws an exception if an error occurs (blocking)

#### `send(fildes: int, data: string): int`

Precondition:   `data`<br>
Postcondition:  returns the number of bytes sent to the socket `fildes`, otherwise returns `0` if disconnected or throws an exception if an error occurs

#### `sendTo(fildes: int, data: string): int`

Precondition:   none<br>
Postcondition:  returns the number of bytes sent to the socket `fildes`, otherwise returns `0` if disconnected or throws an exception if an error occurs

#### `tcpClient(src: string, port: int): int`

Precondition:   `src && port >= 0`<br>
Postcondition:  returns a file descriptor identifying the server at `src` and `port`, otherwise throws an exception if an error occurs

#### `tcpServer(port: int, backlog: int)`

Precondition:   `port >= 0 && backlog >= 0`<br>
Postcondition:  starts a TCP server at `port` with capacity `backlog`; returns its file descriptor, otherwise throws an exception if an error occurs

#### `udpClient(src: string, port: int): int`

Precondition:   `src && port >= 0`<br>
Postcondition:  returns a file descriptor identifying the server at `src` and `port`, otherwise throws an exception if an error occurs

#### `udpServer(port: int): int`

Precondition:   `port >= 0`<br>
Postcondition:  starts a UDP server at `port`; returns its file descriptor, otherwise throws an exception if an error occurs

### System

#### `array(num: int): array | null`

Precondition:   `num >= 1`<br>
Postcondition:  returns an array of null values with count `num`

#### `array(...args): array`

Precondition:   none<br>
Postcondition:  returns an array from `args`

#### `cmd(cmd: string): string`

Precondition:   `cmd !== null`<br>
Postcondition:  executes `cmd` on the host environment's command processor and returns the output

#### `gmt(): string`

Precondition:   none<br>
Postcondition:  returns the current GMT time string

#### `input(): number | string`

Precondition:   none<br>
Postcondition:  returns a number or string from stdin

#### `lap(): double`

Precondition:   none<br>
Postcondition:  returns the stopwatch time (seconds), otherwise returns `-1` if no such stopwatch exists

#### `local(): string`

Precondition:   none<br>
Postcondition:  returns the current local time string

#### `rand(): int`

Precondition:   none<br>
Postcondition:  returns a pseudorandom, unsigned integer

#### `start(): int`

Precondition:   none<br>
Postcondition:  starts a new stopwatch and returns its descriptor

#### `stop(sw: int): double`

Precondition:   none<br>
Postcondition:  stops the stopwatch; returns its time (seconds), otherwise returns `-1` if no such stopwatch exists

#### `worker(filename: string, ...args): null`

Precondition:   `filename`<br>
Postcondition:  starts a worker thread from the file at `filename` and passes `args` to it; returns `null`
