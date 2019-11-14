import sys
if len(sys.argv) < 3:
    print("Usage: python gen_stack.py [num_entries] [bitwidth]")

num_entries = int(sys.argv[1])
bitwidth = int(sys.argv[2])
print('vars {')

for i in range(num_entries + 1):
    print(f'\tint<1> stack{i}_is_full;')
    print(f'\tint<1> stack{i}_op;')
    print(f'\tint<{bitwidth}> stack{i}_data;')
    print(f'\tchan<{bitwidth}> stack{i}_op_chan;')
    print(f'\tchan<{bitwidth}> stack{i}_comm_chan;')
    print()
print('}')

print('chp {')

for i in range(num_entries):
    # 0 = pop, 1 = push
    print(f'\tstack{i}_is_full := 0;')
    print(f'\t*[ stack{i}_op_chan?stack{i}_op;')
    print(f'\t[ ~stack{i}_is_full & stack{i}_op -> stack{i}_comm_chan?stack{i}_data, stack{i}_is_full := 1')
    print(f'\t[] ~stack{i}_is_full & ~stack{i}_op -> stack{i+1}_op_chan!0; stack{i+1}_comm_chan?stack{i}_data; stack{i}_comm_chan!stack{i}_data')
    print(f'\t[] stack{i}_is_full & stack{i}_op -> stack{i+1}_op_chan!1; stack{i+1}_comm_chan!stack{i}_data; stack{i}_comm_chan?stack{i}_data')
    print(f'\t[] stack{i}_is_full & ~stack{i}_op -> stack{i}_comm_chan!stack{i}_data, stack{i}_is_full := 0')
    print(f'\t] ] ||')

print(f'\t*[ stack{num_entries}_op_chan?stack{num_entries}_op;')
print(f'\t[ stack{num_entries}_op -> stack{num_entries}_comm_chan?stack{num_entries}_data')
print(f'\t[] ~stack{num_entries}_op -> stack{num_entries}_comm_chan!stack{num_entries}_data')
print(f'\t] ]')

print('}')