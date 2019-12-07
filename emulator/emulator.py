import sys
import math

def compute(op, args, state):
    if op == 'halt':
        return -math.inf
    elif op == 'pushc':
        state['stack'].append(int(args[0]))
    elif op == 'pop':
        state['stack'].pop()
    elif op == 'popout':
        print(state['stack'].pop())
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

    return 1


def run(fname):
    state = { 'regs': [0] * 4, 'stack': [] }
    with open(fname, 'r') as f:
        cmds = f.readlines()
        pc = 0
        while pc >= 0:
            parts = [x.strip() for x in cmds[pc].split(' ')]
            pc = pc + compute(parts[0], parts[1:], state)
            print(str(state), ' '.join(parts))

    print(state)

def main():
    if len(sys.argv) != 2:
        print('Usage: python3 emulator.py filename.cmd')
        sys.exit(1)

    run(sys.argv[1])

if __name__ == '__main__':
    main()
