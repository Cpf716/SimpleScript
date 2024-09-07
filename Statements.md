# SimpleScript

## Statements

#### echo : any

#### define : symbol, any

#### suppress : symbol

#### sleep : int

### Jump

#### assert : string

#### break

#### continue

#### exit

#### return : any

#### throw : string

## Control Statements

### Function

#### func-end func : symbol

func foobar<br>
&nbsp;&nbsp;&nbsp;&nbsp;echo ""<br>
end func

### Jump

#### try-catch-end try : symbol

try<br>
&nbsp;&nbsp;&nbsp;&nbsp;throw ""<br>
catch exc<br>
end try

### Repetition

#### do while-end while

do while true<br>
&nbsp;&nbsp;&nbsp;&nbsp;continue<br>
end while

#### for-end for

for ,,<br>
&nbsp;&nbsp;&nbsp;&nbsp;continue<br>
end for

#### while-end while

while true<br>
&nbsp;&nbsp;&nbsp;&nbsp;continue<br>
end while

### Selection

#### if-end if

if true<br>
&nbsp;&nbsp;&nbsp;&nbsp;echo ""<br>
end if
