# Welcome
Hi there, welcome to Quartz! This is an experimental programming language, meaning that its underlying systems and how it handles things like memory and pointers can and will absolutely change with time.

![Version](https://img.shields.io/badge/version-0.0.0-ffffff?style=for-the-badge)  ![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![C++](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
## How it works
In short, this language is split into two parts. The compiler, which you would run on your personal computer, and the interpreter, which you could run just about anywhere else, given that you have a method of transporting the compiled result there.
Now, in a little more detail. The compiler is made up of the lexer, parser, and bytecode emitter. The lexer converts source into tokens, which goes into the parser. The parser will rearrange these tokens into instructions, which can then be directly converted into bytes.

The interpreter takes those bytes and runs it through its own virtual machine. The VM loops through ALL provided bytes, where each one is interpreted and a function is ran for whatever that instruction may describe. For example, a NUMBER instruction tells the interpreter to read the next four bytes, turn them into a float from those bytes, and then push the resulting number into the stack. All it is really doing is figuring out what values to put onto the top of the stack, which can be thought of as a sort of "pile". When a byte needs a value, it simply takes from the top of the stack, in which the byte can perform its respective function.

## Bytes/Opcodes
To what byte an instruction to is assigned is decided compile-time.
### NUMBER
Reads the next four bytes and converts them into a float(from little endian).
### STRING
Continues to read a byte, convert it into a character, and push it into a string, until the **STRING_END** opcode is encountered.
### STRING_END
Marks the end of a string definition.
### NIL
Pushes a nil value to the stack.
### TABLE
Reads and pushes values into a table stack, rather than the global stack, until the **TABLE_END** opcode is encountered.
### TABLE_END
Marks the end of a table definition.
### ADD
Pops two values from the stack, adds them together, and pushes the result back into the stack. If at least one value is a string, then concatenation occurs.
### SUB
Ditto, but with subtraction. Cannot subtract strings.
### MUL
Ditto, but with multiplication.
### DIV
Ditto, but with division.
### PRINT
Pops one value from the stack and prints its value.
### SET
Pops two values from the stack, the variable ID and its value, and registers it as a variable.
### FETCH
Pops one value from the stack, being the variable ID, and finds the value tied to it. It pushes that value to the stack.
### TABLE_FETCH
Fetches a table. Because tables are stored as pointers, this operation is required and distinct from **FETCH**.
### INDEX_ACCESS
Accesses an item from a table via index.
> [!WARNING]
> Table methods do not work properly. Because there is a distinction between the fetching of normal values and pointer values, assigning a variable to an existing pointer completely wipes out its pointer status, meaning any operations performed on that new variable will be non-pointer related. This distinction may be eliminated entirely moving forward.
### START_PGM
Marks when to start writing the program to memory.
### END_PGM
Marks when to stop writing the program to memory.
> [!NOTE]
> Start and end program markers are always appended to the start and end of the output bytecode respectively.
