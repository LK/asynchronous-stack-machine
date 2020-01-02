<div align="center">
 <h1>An asynchronous stack machine microprocessor.</h1>
 <h4>An <a href="http://csl.yale.edu/~rajit/classes/eeng426/index.php">EENG426</a> final project by <a href="https://github.com/LK">Lenny Khazan</a> and <a href="https://github.com/ajoann">Amanda Hansen</a></h4>
</div>

### What is this?
We designed a stack machine using asynchronous circuit design techniques, with the [ACT](https://github.com/asyncvlsi/act) toolchain. Our stack machine supports 11 instructions to perform basic arithmetic and control flow logic. It includes four on-chip registers and two on-chip stack elements to store operands and outputs, with further elements overflowing to off-chip memory. Data values are stored in eight bits, instruction addresses in 12 bits, and instructions are encoded as a 4-bit opcode and 8-bit operand (additional operands must be passed via the stack).

### Why?
Unlike traditional clocked chips, our processor is able to execute instructions without relying on any kind of clock signal (see [quasi-delay-insensitive circuits](https://en.wikipedia.org/wiki/Quasi-delay-insensitive_circuit)). Given the right tooling, this greatly simplifies the design and verification process when designing and manufacturing chips.
 
### Compile Processor
```
> make
```
### Test Processor
```
> make 
> python emulator/emulator.py emulator/tests/add.cmd > prsim_file
> ./testing/generators/prsim2irsim prsim_file testing/processor/add.irsim
> prsim testing/processor/test_processor.prs
> irsim scmos30 testing/processor/test_processor.sim testing/processor/test_processor.al
```

You can then `source` the prsim and irsim files from within the respective tools.

### Test Component
Create input script in the format as seen in all the components' "test_in.txt"
```
> cd testing/generators
> make
> ./test_writer  input_file  output_file  [--irsim OR --prsim] [--random] [--break-on-warn] [--reset] [--_reset]
```

*The random and break-on-warn arguments only work for PRSIM tests*

*The reset and \_reset options will add toggling those values at the beginning of the test script*

### Generate Stack CHP
```
> python generators/gen_stack.py  len_stack  data_width
```
### Generate Opcode Conditions
```
> python generators/gen_opcode_conditions.py > opcode_conditions.act
```

### The instruction set supports the following operations:
* HALT: terminate the program.
* PUSHC: push a constant onto the stack.
* POP: discard the top element of the stack.
* POPOUT: pop the top element off of the stack and send it over the OUT channel.
* PUSHREG: push the value of a register onto the stack, leaving the register intact.
* POPREG: pop the top value off of the stack and store it in the specified register.
* ADD: replace the two 2 elements at the top of the stack with one element containing their sum.
* SUB: subtract the top-most element of the stack from the element below it, and replace the top two elements of the stack with one element containing the result.
* JMP: relative jump, offsetting the program counter by a given amount.
* BRZ: conditional branch, if the top element of the stack is a “0” then offset the program counter by the given amount, otherwise fall through to the next instruction. Leaves the stack intact.
* BRLZ: conditional branch, if the top element of the stack is strictly less than zero then offset the program counter by the given amount, otherwise fall through to the next instruction. Leaves the stack intact.
