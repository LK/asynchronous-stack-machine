# asynchronous-stack-machine

### Compile Processor
> make

### Test Processor
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

### Test Component
// Create input script in the format as seen in all the components' "test_in.txt"
> cd testing/generators
> make
> ./test_writer  input_file  output_file  \[--irsim OR --prsim] \[--random] \[--break-on-warn] \[--reset] \[--\_reset]

* The random and break-on-warn arguments only work for PRSIM tests
** The reset and \_reset options will add toggling those values at the beginning of the test script

### Generate Stack CHP
> python generators/gen_stack.py  len_stack  data_width

### Generate Opcode Conditions
> python generators/gen_opcode_conditions.py > opcode_conditions.act
