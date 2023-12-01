# SimpleScript

## Functions

### Environment

#### array(num: int): array | null

Precondition:   `num >= 1`<br>
Postcondition:  returns an array of null values with count `num`

#### gmt(): string

Precondition:   none<br>
Postcondition:  returns the current GMT time string

#### input(): number | string

Precondition:   none<br>
Postcondition:  returns a number or string from stdin

#### local(): string

Precondition:   none<br>
Postcondition:  returns the current local time string

#### rand(): int

Precondition:   none<br>
Postcondition:  returns a pseudorandom, unsigned integer

<!-- -->

### File System

#### exists(path: string): int

Precondition:   `count path > 0`<br>
Postcondition:  returns 1 if a file exists at `path`, otherwise returns 0

#### read(path: string): string
#### read(path: string, type: string): string | table

Precondition:   `count path > 0 && array("csv", "tsv", "txt") contains type`<br>
Postcondition:  returns data read from the file at `path`, otherwise returns `undefined` if the file does not exist

#### remove(path: string): int

Precondition:   `count path > 0`<br>
Postcondition:  returns 1 if the file at `path` is removed, otherwise returns 0

#### write(path: string, data: any): string
#### write(path: string, data: any, type: string): string

Precondition:   `count path >= 0 && ((typeOf data === "string" && data !== null) || typeOf data !== "string" && array("csv", "tsv") contains type)`<br>
Postcondition:  writes `data` to file; returns `undefined`

<!-- -->

### Sockets

#### accept(fildes: int): array | null

Precondition:   `fildes >= 0`<br>
Postcondition:  returns an array of file descriptors identifying clients of the server `fildes`, otherwise returns `null` if the server has no clients or an error occurs

#### client(src: string, port: int): int

Precondition:   `count src > 0 && port >= 0`<br>
Postcondition:  returns a file descriptor identifying the server at `src` and `port`, otherwise throws socket_exception if an error occurs (blocking)

#### close(fildes: int): int

Precondition:   `fildes >= 0`<br>
Postcondition:  returns 0 upon successful closure of the socket `fildes`, otherwise throws socket_exception if an error occurs

#### listen(fildes: int, port: int): int

Precondition:   `fildes >= 0 && port >= 0`<br>
Postcondition:  listens to messages received by the socket `fildes` and forwards them to `port`; returns the file descriptor identifying the listener, otherwise throws socket_exception if an error occurs

#### recv(fildes: int): string

Precondition:   `fildes >= 0`<br>
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

#### close(con: int): int

Precondition:   `con >= 0`<br>
Postcondition:  returns 0 upon successful closure of the MySQL connection, otherwise returns -1 or throws an exception if an error occurs

#### prepareQuery(con: int, sql: string, args...): table

Precondition:   `con >= 0 && count sql > 0`<br>
Postcondition:  returns the result set of a prepared statement query, otherwise returns `undefined` if no such connection exists or throws an exception if an error occurs

#### prepareUpdate(con: int, sql: string, args...): int

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
