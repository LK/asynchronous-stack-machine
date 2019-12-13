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
    print(f'\tchan<1> stack{i}_op_chan;')
    print(f'\tchan<{bitwidth}> stack{i}_push_chan;')
    print(f'\tchan<{bitwidth}> stack{i}_pop_chan;')
    print()
print('}')

print('chp {')

for i in range(num_entries):
    # 0 = pop, 1 = push
    print(f'''
  stack{i}_is_full := 0;
  *[ stack{i}_op_chan?stack{i}_op;
    [
      (~stack{i}_is_full & ~stack{i}_op) | (stack{i}_is_full & stack{i}_op) -> stack{i+1}_op_chan!stack{i}_op
      [] else -> skip
    ];

    [
      (~stack{i}_is_full & ~stack{i}_op) -> stack{i+1}_pop_chan?stack{i}_data
      [] else -> skip
    ];

    [ ~stack{i}_is_full & stack{i}_op -> stack{i}_push_chan?stack{i}_data, stack{i}_is_full := 1
    [] (~stack{i}_is_full & ~stack{i}_op) | (stack{i}_is_full & ~stack{i}_op) -> stack{i}_pop_chan!stack{i}_data
    [] stack{i}_is_full & stack{i}_op -> stack{i+1}_push_chan!stack{i}_data; stack{i}_push_chan?stack{i}_data
    ];

    [
      stack{i}_is_full & ~stack{i}_op -> stack{i}_is_full := 0
      [] else -> skip
    ]
  ] ||''')

print(f'\t*[ stack{num_entries}_op_chan?stack{num_entries}_op;')
print(f'\t[ stack{num_entries}_op -> stack{num_entries}_push_chan?stack{num_entries}_data')
print(f'\t[] ~stack{num_entries}_op -> stack{num_entries}_pop_chan!stack{num_entries}_data')
print(f'\t] ]')

print('}')
