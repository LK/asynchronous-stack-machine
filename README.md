# asynchronous-stack-machine
 a processor that implements a stack-based architecture with an expansive enough instruction set to allow arbitrary computation.
 
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
  // in PRSIM:
  > source prsim_file
> irsim scmos30 testing/processor/test_processor.sim testing/processor/test_processor.al
  // in IRSIM:
  > source testing/processor/add.irsim
  > source testing/processor/jmp.irsim
  > source testing/processor/sub.irsim
```
### Test Component
// Create input script in the format as seen in all the components' "test_in.txt"
```
> cd testing/generators
> make
> ./test_writer  input_file  output_file  \[--irsim OR --prsim] \[--random] \[--break-on-warn] \[--reset] \[--\_reset]
```
\* The random and break-on-warn arguments only work for PRSIM tests
** The reset and \_reset options will add toggling those values at the beginning of the test script

### Generate Stack CHP
```
> python generators/gen_stack.py  len_stack  data_width
```
### Generate Opcode Conditions
```
> python generators/gen_opcode_conditions.py > opcode_conditions.act
```

### The instruction set supports the following operations:
HALT: terminate the program.
PUSHC: push a constant onto the stack.
POP: discard the top element of the stack.
POPOUT: pop the top element off of the stack and send it over the OUT channel.
PUSHREG: push the value of a register onto the stack, leaving the register intact.
POPREG: pop the top value off of the stack and store it in the specified register.
ADD: replace the two 2 elements at the top of the stack with one element containing their sum.
SUB: subtract the top-most element of the stack from the element below it, and replace the top two elements of the stack with one element containing the result.
JMP: relative jump, offsetting the program counter by a given amount.
BRZ: conditional branch, if the top element of the stack is a “0” then offset the program counter by the given amount, otherwise fall through to the next instruction. Leaves the stack intact.
BRLZ: conditional branch, if the top element of the stack is strictly less than zero then offset the program counter by the given amount, otherwise fall through to the next instruction. Leaves the stack intact.
