BITWIDTH = 4
OPCODES = {
  'halt':       0b0000,
  'pushc':      0b0001,
  'pop':        0b0010,
  'popout':     0b0011,
  'pushreg':    0b0100,
  'popreg':     0b0101,
  'add':        0b0110,
  'sub':        0b0111,
  'jmp':        0b1000,
  'brz':        0b1001,
  'brlz':       0b1010
}

def compute_truths_for_code(code):
  truths = [False] * BITWIDTH
  for i in range(BITWIDTH):
    truths[BITWIDTH-i-1] = (code & (1 << i)) > 0
  return truths

def gen_guard_condition(bit_idx, truth, inverted):
    return ('~' if inverted else '') + f'v0[{bit_idx}].' + ('t' if (not truth and inverted) or
            (truth and not inverted) else 'f')

for name, code in OPCODES.items():
  truths = compute_truths_for_code(code)
  print(f'defproc bundled_is_{name}_1(bool go_r; dualrail v0[4]; dualrail out) {{')
  print('\tbool _gor;')
  print('\tprs {')
  print('\t\tgo_r => _gor-')
  print('\t\t' + ' & '.join([gen_guard_condition(i, t, True) for i, t in enumerate(truths)]) + ' & ~_gor -> out.t+')
  print('\t\t' + ' | '.join([gen_guard_condition(i, not t, True) for i, t in enumerate(truths)]) + ' & ~_gor -> out.f+')
  print('\t\t_gor -> out.t-')
  print('\t\t_gor -> out.f-')
  print('\t}')
  print('}')
  print()

print('defproc bundled_is_branch_1(bool go_r; dualrail v0[4]; dualrail out) {')
print('\tbool _gor;')
print('\tprs {')
print('\t\tgo_r => _gor-')
print('\t\t~v0[3].f & ~_gor -> out.t+')
print('\t\t~v0[3].t & ~_gor -> out.f+')
print('\t\t_gor -> out.t-')
print('\t\t_gor -> out.f-')
print('\t}')
print('}')
print()
