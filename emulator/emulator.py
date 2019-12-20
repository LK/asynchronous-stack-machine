import sys
import math
import copy

GEN_PRSIM = True
STACK_SIZE = 2

def compute(op, args, state):
    if op == 'halt':
        return -math.inf
    elif op == 'pushc':
        state['stack'].append(int(args[0]))
    elif op == 'pop':
        state['stack'].pop()
    elif op == 'popout':
        t = state['stack'].pop()
        if not GEN_PRSIM:
            print(t)
    elif op == 'pushreg':
        state['stack'].append(state['regs'][int(args[0])])
    elif op == 'popreg':
        state['regs'][int(args[0])] = state['stack'].pop()
    elif op == 'add':
        state['stack'][-2] += state['stack'][-1]
        state['stack'].pop()
    elif op == 'sub':
        state['stack'][-2] -= state['stack'][-1]
        state['stack'].pop()
    elif op == 'jmp':
        return int(args[0])
    elif op == 'brz':
        return int(args[0]) if state['stack'][-1] == 0 else 1
    elif op == 'brlz':
        return int(args[0]) if state['stack'][-1] < 0 else 1
    else:
        print('cannot parse op ' + op)
        sys.exit(1)

    return 1

def encode(parts):
    op_encoded = 0
    data_encoded = 0
    if parts[0] == 'halt':
        op_encoded = 0b1111
    elif parts[0] == 'pushc':
        op_encoded = 0b0001
        data_encoded = int(parts[1])
    elif parts[0] == 'pop':
        op_encoded = 0b0010
    elif parts[0] == 'popout':
        op_encoded = 0b0011
    elif parts[0] == 'pushreg':
        op_encoded = 0b0100
        data_encoded = int(parts[1])
    elif parts[0] == 'popreg':
        op_encoded = 0b0101
        data_encoded = int(parts[1])
    elif parts[0] == 'add':
        op_encoded = 0b0110
    elif parts[0] == 'sub':
        op_encoded = 0b0111
    elif parts[0] == 'jmp':
        op_encoded = 0b1000
        data_encoded = int(parts[1])
    elif parts[0] == 'brz':
        op_encoded = 0b1001
        data_encoded = int(parts[1])
    elif parts[0] == 'brlz':
        op_encoded = 0b1010
        data_encoded = int(parts[1])
    else:
        print('cannot encode op ' + parts[0])
        sys.exit(1)

    if data_encoded < 0:
      data_encoded = 2**8 + data_encoded

    return (data_encoded << 4) | op_encoded

def run(fname):
    state = { 'regs': [0] * 4, 'stack': [] }
    prsim_preamble()

    with open(fname, 'r') as f:
        cmds = f.readlines()
        pc = 0
        while pc >= 0:
            assert_state(state, pc)
            assert_send('p.chan_PC', 12, pc)
            parts = [x.strip() for x in cmds[pc].split(' ')]
            assert_recv('p.chan_IN', 12, encode(parts))
            if parts[0] == 'popout':
                assert_send('p.chan_OUT', 8, state['stack'][-1])
            incr = compute(parts[0], parts[1:], state)
            if pc + incr < 0:
              break
            pc = pc + incr
            if not GEN_PRSIM:
                print(str(state), ' '.join(parts))

    if not GEN_PRSIM:
        print(state)
    else:
        assert_state(state, pc)

def assert_send(channame, bitwidth, value):
  assert_send_start(channame, bitwidth, value)
  assert_send_end(channame, bitwidth)

def assert_send_start(channame, bitwidth, value):
    if not GEN_PRSIM:
        return

    for i in range(bitwidth):
        t = value & (0b1 << i)
        if t:
            print(f'assert {channame}.d[{i}].t 1')
            print(f'assert {channame}.d[{i}].f 0')
        else:
            print(f'assert {channame}.d[{i}].t 0')
            print(f'assert {channame}.d[{i}].f 1')

    print(f'set {channame}.a 1')
    print('cycle')

def assert_send_end(channame, bitwidth):
    if not GEN_PRSIM:
      return

    for i in range(bitwidth):
        print(f'assert {channame}.d[{i}].t 0')
        print(f'assert {channame}.d[{i}].f 0')

    print(f'set {channame}.a 0')
    print('cycle')

def assert_recv(channame, bitwidth, value):
  assert_recv_start(channame, bitwidth, value)
  assert_recv_end(channame, bitwidth)

def assert_recv_start(channame, bitwidth, value):
    if not GEN_PRSIM:
        return

    for i in range(bitwidth):
        t = value & (0b1 << i)
        if t:
            print(f'set {channame}.d[{i}].t 1')
            print(f'set {channame}.d[{i}].f 0')
        else:
            print(f'set {channame}.d[{i}].t 0')
            print(f'set {channame}.d[{i}].f 1')

    print('cycle')
    print(f'assert {channame}.a 1')

def assert_recv_end(channame, bitwidth):
    if not GEN_PRSIM:
      return

    for i in range(bitwidth):
        print(f'set {channame}.d[{i}].t 0')
        print(f'set {channame}.d[{i}].f 0')

    print('cycle')
    print(f'assert {channame}.a 0')

def assert_state(state, pc):
    def assert_multibit(varname, bitwidth, target):
        for i in range(bitwidth):
            t = target & (0b1 << i)
            if t:
                print(f'assert {varname}[{i}].v.t 1')
                print(f'assert {varname}[{i}].v.f 0')
            else:
                print(f'assert {varname}[{i}].v.t 0')
                print(f'assert {varname}[{i}].v.f 1')

    if not GEN_PRSIM:
        return

    for i in range(STACK_SIZE):
        if len(state['stack']) <= i:
            print(f'assert p.var_stack{i}_is_full.v.t 0')
            print(f'assert p.var_stack{i}_is_full.v.f 1')
        else:
            print(f'assert p.var_stack{i}_is_full.v.t 1')
            print(f'assert p.var_stack{i}_is_full.v.f 0')
            assert_multibit(f'p.var_stack{i}_data', 8, state['stack'][-(i+1)])

    for i in range(4):
        assert_multibit(f'p.var_reg{i}', 8, state['regs'][i])

    assert_multibit(f'p.var_pc_index', 12, pc)

def prsim_preamble():
    def watch_multibit(varname, bitwidth):
        for i in range(bitwidth):
            print(f'watch {varname}[{i}].v.t')
            print(f'watch {varname}[{i}].v.f')
            print(f'set_principal {varname}[{i}].v.t')
            print(f'set_principal {varname}[{i}].v.f')

    def watch_chan(channame, bitwidth):
        for i in range(bitwidth):
            print(f'watch {channame}.d[{i}].t')
            print(f'watch {channame}.d[{i}].f')
            print(f'set_principal {channame}.d[{i}].t')
            print(f'set_principal {channame}.d[{i}].f')
        print(f'watch {channame}.a')
        print(f'set_principal {channame}.a')

    def clear_chan_data(channame, bitwidth):
        for i in range(bitwidth):
                print(f'set {channame}.d[{i}].t 0')
                print(f'set {channame}.d[{i}].f 0')

    if not GEN_PRSIM:
        return

    print('initialize')
    print('break-on-warn')
    print('watch Reset')
    print('watch _Reset')
    print('watch go.r')
    print('watch go.a')
    print('set_principal Reset')
    print('set_principal _Reset')
    print('set_principal go.r')
    print('set_principal go.a')

    for i in range(STACK_SIZE):
        watch_multibit(f'p.var_stack{i}_data', 8)
        print(f'watch p.var_stack{i}_is_full.v.t')
        print(f'watch p.var_stack{i}_is_full.v.f')
        print(f'set_principal p.var_stack{i}_is_full.v.t')
        print(f'set_principal p.var_stack{i}_is_full.v.f')
        print(f'watch p.var_stack{i}_op.v.t')
        print(f'watch p.var_stack{i}_op.v.f')
        print(f'set_principal p.var_stack{i}_op.v.t')
        print(f'set_principal p.var_stack{i}_op.v.f')
        print(f'watch p.chan_stack{i}_op_chan.a')
        print(f'watch p.chan_stack{i}_op_chan.t')
        print(f'watch p.chan_stack{i}_op_chan.f')
        print(f'set_principal p.chan_stack{i}_op_chan.a')
        print(f'set_principal p.chan_stack{i}_op_chan.t')
        print(f'set_principal p.chan_stack{i}_op_chan.f')
        print(f'watch p.chan_stack{i}_full_chan.a')
        print(f'watch p.chan_stack{i}_full_chan.t')
        print(f'watch p.chan_stack{i}_full_chan.f')
        print(f'set_principal p.chan_stack{i}_full_chan.a')
        print(f'set_principal p.chan_stack{i}_full_chan.t')
        print(f'set_principal p.chan_stack{i}_full_chan.f')
        watch_chan(f'p.chan_stack{i}_push_chan', 8)
        watch_chan(f'p.chan_stack{i}_pop_chan', 8)

    for i in range(4):
        watch_multibit(f'p.var_reg{i}', 8)

    watch_multibit('p.var_pc_index', 12)
    watch_multibit('p.var_exec_data', 8)

    watch_chan('p.chan_PC', 12)
    watch_chan('p.chan_IN', 12)
    watch_chan('p.chan_OUT', 12)

    print('watch p.var_dbg.v.t')
    print('watch p.var_dbg.v.f')
    print('set_principal p.var_dbg.v.t')
    print('set_principal p.var_dbg.v.f')

    print('mode reset')
    print('set Reset 1')
    print('set _Reset 0')
    print('set go.r 0')
    print('set p.chan_PC.a 0')
    print('set p.chan_OUT.a 0')
    clear_chan_data('p.chan_IN', 12)
    clear_chan_data('p.chan_stack2_pop_chan', 8)
    print('set p.chan_stack2_push_chan.a 0')
    print('set p.chan_stack2_op_chan.a 0')
    print('set p.chan_stack2_full_chan.t 0')
    print('set p.chan_stack2_full_chan.f 0')
    print('cycle')
    print('set Reset 0')
    print('set _Reset 1')
    print('cycle')

    print('mode run')
    print('set go.r 1')
    print('cycle')

def main():
    if len(sys.argv) != 2:
        print('Usage: python3 emulator.py filename.cmd')
        sys.exit(1)

    run(sys.argv[1])

if __name__ == '__main__':
    main()
