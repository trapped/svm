fib: .int=0 #functions can specify arg type and pattern matched value
  ret 0
fib: .int=1,2 #multiple pattern matched values, arg is again ignored
  ret 1

fib: n.int #arg is named (used) and type is specified
  mov n, %R0 #registers %R0 to %R256, can hold any standard value
  mov n, %R1
  sub %R0, 1 #in-place operations
  sub %R1, 2
  call fib %R0, %R0 #in-place call return value: assign to %R0 result of...
  call fib %R1, %R1
  add %R0, %R1
  ret %R0

main: n.int
  mov n, %R0
  call fib %R0, %R0
  write %R0 #high level IO
