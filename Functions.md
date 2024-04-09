# SimpleScript

## Functions

### Environment

#### array(num: int): array | null
#### array(argv: any...): array

Precondition:   `num >= 1`<br>
Postcondition:  returns an array of null values with count `num`

#### gmt(): string

Precondition:   none<br>
Postcondition:  returns the current GMT time string

#### input(): number | string

Precondition:   none<br>
Postcondition:  returns a number or string from stdin

#### lap(): double

Precondition:   none<br>
Postcondition:  returns the stopwatch time (seconds)

#### local(): string

Precondition:   none<br>
Postcondition:  returns the current local time string

#### rand(): int

Precondition:   none<br>
Postcondition:  returns a pseudorandom, unsigned integer

#### start(): int

Precondition:   none<br>
Postcondition:  starts a new stopwatch and returns its descriptor

#### stop(sw: int): double

Precondition:   `sw >= 0`<br>
Postcondition:  stops the stopwatch; returns its time (seconds)

<!-- -->

### File System

#### closeFile(filedes: int): string

Precondition:   `fildes >= 0`<br>
Postcondition:  closes file; returns undefined

#### delete(filename: string): int

Precondition:   `count filename > 0`<br>
Postcondition:  returns 1 if the file at `filename` is removed, otherwise returns 0

#### exists(filename: string): int

Precondition:   `count filename > 0`<br>
Postcondition:  returns 1 if a file exists at `filename`, otherwise returns 0

#### file(filename: string): int

Precondition:   `filename !== null && count filename`<br>
Postcondition:  opens a new file at `filename`; returns its file descriptor, otherwise returns -1

#### read(filename: string): string
#### read(filename: string, type: string): string | table

Precondition:   `count filename > 0 && array("csv", "tsv", "txt") contains type`<br>
Postcondition:  returns data read from the file at `filename`, otherwise returns `undefined` if the file does not exist

#### rename(old: string, new: string): string

Precondition: `old !== null && count old && new !== 0 && count new`<br>
Postcondition:  renames file `old` to `new`; returns undefined

#### write(filedes: int, data: string): string
#### write(filename: string, data: string): string
#### write(filename: string, data: any, type: string): string

Precondition:   `(filedes >= 0 || count filename > 0) && ((typeOf data === "string" && data !== null) || typeOf data !== "string" && array("csv", "tsv") contains type)`<br>
Postcondition:  writes `data` to file; returns `undefined`

<!-- -->

### Sockets

#### client(src: string, port: int): int
#### client(src: string, port: int, timeout: int): int

Precondition:   `count src > 0 && port >= 0 && timeout >= 0`<br>
Postcondition:  returns a file descriptor identifying the server at `src` and `port`, otherwise returns -1 if the connection failed or throws socket_exception if an error occurs

#### closeSocket(fildes: int): int

Precondition:   `fildes >= 0`<br>
Postcondition:  returns 0 upon successful closure of the socket `fildes`, otherwise returns -1 if no such socket exists or throws socket_exception if an error occurs

#### listen(fildes: int, port: int): int

Precondition:   `fildes >= 0 && port >= 0`<br>
Postcondition:  listens to messages received by the socket `fildes` and forwards them to `port`; returns the file descriptor identifying the listener, otherwise throws socket_exception if an error occurs

#### poll(fildes: int): array | null

Precondition:   `fildes >= 0`<br>
Postcondition:  returns an array of file descriptors identifying clients of the server `fildes`, otherwise returns `null` if the server has no clients or an error occurs

#### recv(fildes: int): string
#### recv(fildes: int, timeout: int): string

Precondition:   `fildes >= 0 && timeout >= 0`<br>
Postcondition:  returns data received from the socket `fildes`, otherwise returns `undefined` if disconnected or an error occurs (blocking)

#### send(fildes: int, data: string): int

Precondition:   `fildes >= 0 && count data > 0`<br>
Postcondition:  returns the number of bytes sent to the socket `fildes`, otherwise returns 0 if disconnected or -1 if an error occurs

#### server(port: int, backlog: int)

Precondition:   `port >= 0 && backlog >= 0`<br>
Postcondition:  starts a server at `port` with the capacity of `backlog`; returns the file descriptor identifying the server, otherwise throws socket_exception if an error occurs

<!-- -->

### SQL

#### connect(host: string, usr: string, pwd: string): int

Precondition:   `count host > 0 && count usr > 0 && count pwd > 0`<br>
Postcondition:  returns the MySQL connection to `hostName` given `userName` and `password`, otherwise throws an exception if an error occurs

#### closeConnection(con: int): int

Precondition:   `con >= 0`<br>
Postcondition:  returns 0 upon successful closure of the MySQL connection, otherwise returns -1 or throws an exception if an error occurs

#### preparedQuery(con: int, sql: string, args...): table

Precondition:   `con >= 0 && count sql > 0`<br>
Postcondition:  returns the result set of a prepared statement query, otherwise returns `undefined` if no such connection exists or throws an exception if an error occurs

#### preparedUpdate(con: int, sql: string, args...): int

Precondition:   `con >= 0 && count sql > 0`<br>
Postcondition:  returns the number of rows updated by a prepared statement update, otherwise returns -1 if no such connection exists or throws an exception if an error occurs

#### query(con: int, sql: string): table

Precondition:   `con >= 0 && count sql > 0`<br>
Postcondition:  returns the result set of `sql` query, otherwise returns `undefined` if no such connection exists or throws an exception if an error occurs

#### setSchema(con: int, schema: string)

Precondition:   `con >= 0 && count schema > 0`<br>
Postcondition:  sets the schema for connection `con`; returns 1 on success, otherwise returns 0 if no such connection exists or throws an exception if an error occurs

#### update(con: int, sql: string)

Precondition:   `con >= 0 && count sql > 0`<br>
Postcondition:  returns the number of rows updated by `sql` query, otherwise returns -1 if no such connection exists or throws an exception if an error occurs
