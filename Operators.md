# SimpleScript

## Operator Precedence

### Unary Operators

| Opcode      | Parameters            | Return Value                                                            |
| ----------- | --------------------- | ----------------------------------------------------------------------- |
| abs         | num                   | absolute value of `num` (double)                                        |
| cbrt        | num                   | cube root of `num` (double)                                             |
| ceil        | num                   | ceiling of `num` (int)                                                  |
| floor       | num                   | floor of `num` (int)                                                    |
| log         | num                   | natural log of `num` (double)                                           |
| sqrt        | num                   | square root of `num` (double)                                           |
| !           | val                   | negation of `val` (int)                                                 |
| count       | val (array \| string) | number of elements or characters in `val` (int)                         |
| first       | arr                   | first element in `arr` (number \| string)                               |
| inverse     | tab                   | `tab` with its rows and columns swapped (table)                         |
| isAlpha     | str                   | 1 if `str` is comprised only of [A-Z][a-z], otherwise 0 (string)        |
| isAlnum     | str                   | 1 if `str` is comprised only of [0-9][A-Z][a-z], otherwise 0 (string)   |
| isDigits    | str                   | 1 if `str` is comprised only of [0-9], otherwise 0 (string)             |
| isLower     | str                   | 1 if `str` is comprised only of lowercase letters, otherwise 0 (string) |
| isSpace     | str                   | 1 if `str` is comprised only of spaces, otherwise 0 (string)            |
| isUpper     | str                   | 1 if `str` is comprised only of uppercase letters, otherwise 0 (string) |
| keys        | dict                  | `dict`'s keys (array \| string)                                         |
| last        | arr                   | last element in `arr` (number \| string)                                |
| parse       | str                   | number parsed from `str`, otherwise `nan` (number)                      |
| shrink      | arr                   | previous `sizeOf arr` (int)                                             |
| sizeOf      | arr                   | capacity of `arr` (int)                                                 |
| subtypeOf   | val                   | subtype of `val` (string)                                               |
| toCharArray | str (string)          | `str`'s character array (array \| char)                                 |
| toChar      | int                   | character representation of `int` (char)                                |
| toCharCode  | char                  | integer representation of `char` (int)                                  |
| toLower     | str                   | `str` converted to lowercase (string)                                   |
| toUpper     | str                   | `str` converted to uppercase (string)                                   |
| typeOf      | val                   | base type of `val` (string)                                             |
| values      | dict                  | `dict`'s values (array \| string)                                       |

### Binary Operators

#### Dot Operator

| Level | Opcode | Description |
| ----- | ------ | ----------- |
| 1.    | .      | Indexer     |

#### Arithmetic Operators

| Level | Opcode | Description                       |
| ----- | ------ | --------------------------------- |
| 2.    | ^^     | Exponentiation                    |
| 3.    | \* / % | Multiplication, Division, Modulus |
| 4.    | \- +   | Subtraction, Addition

#### Bitwise Operators

| Level | Opcode     | Description             |
| ----- | ---------- | ----------------------- |
| 5.    | \>> <<     | Right Shift, Left Shift |

#### Max/Min Operators

| Level | Opcode     | Description      |
| ----- | ---------- | ---------------- |
| 6.    | max min    | Maximum, Minimum |

#### Functional Operators

| Level | Opcode       | Type             |
| ----- | ----------   | ---------------- |
| 7.    | aggregate    | Ternary (Lambda) |
|       | cell         | Ternary          |
|       | col          | Binary           |
|       | contains     | Binary           |
|       | fill         | Binary           |
|       | filter       | Ternary (Lambda) |
|       | find         | Ternary (Lambda) |
|       | findIndex    | Ternary (Lambda) |
|       | format       | Binary           | //
|       | indexOf      | Binary           |
|       | insert       | Ternary          |
|       | join         | Binary           |
|       | lastIndexOf  | Binary           | 
|       | map          | Ternary (Lambda) |
|       | reserve      | Binary           |
|       | row          | Binary           |
|       | setCount     | Binary           |
|       | slice        | Binary/Ternary   |
|       | splice       | Binary/Ternary   |
|       | substr       | Binary/Ternary   |
|       | toSpliced    | Binary/Ternary   |

#### Relational Operators

| Level | Opcode      | Description                                                      |
| ----- | ----------- | ---------------------------------------------------------------- |
| 8.    | <= \>= < \> | Less Than (Incl.), Greater Than (Incl.), Less Than, Greater Than |

#### Equality Operators

| Level | Opcode        | Description                                              |
| ----- | ------------- | -------------------------------------------------------- |
| 9.    | === !== == != | Triple Equality, Triple Inequality, Equality, Inequality |

#### Bitwise Operators

| Level | Opcode     | Description |
| ----- | ---------- | ----------- |
| 10.   | &          | Bitwise AND |
| 11.   | ^          | Bitwise XOR |
| 12.   | \|         | Bitwise OR  |

#### Logical Operators

| Level | Opcode     | Description |
| ----- | ---------- | ----------- |
| 13.   | &&         | Logical AND |
| 14.   | \|\|       | Logical OR  |

#### Assignment Operators   //

| Level | Opcode     | Description                 |
| ----- | ---------- | --------------------------- |
| 15.   | \*=        | Multiplicative Assignment   |    //
|       | /=         | Division Assignment         |    //
|       | %=         | Modulus Assignment          |    //
|       | -=         | Subtraction Assignment      |    //
|       | +=         | Addition Assignment         |    //
|       | \>>=       | Right Shift Assignment      |    //
|       | <<=        | Left Shift Shift Assignment |    //
|       | &=         | Bitwise AND Assignment      |    //
|       | ^=         | Bitwise XOR Assignment      |    //
|       | \|=        | Bitwise OR Assignment       |    //
|       | =          | Direct Assignment           |    //

#### Conditional Operator

| Level | Opcode     | Description |
| ----- | ---------- | ----------- |
|       | ?          | Conditional |

#### Coalescing Operator

| Level | Opcode     | Description |
| ----- | ---------- | ----------- |
|       | ??         | Coalescing  |

#### Sequencer Operator

| Level | Opcode     | Description |
| ----- | ---------- | ----------- |
| 16.   | ,          | Sequencer   |
