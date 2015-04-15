loop: n.int
  add n, 1 #args are copies
  neqc [loop n] n, 1000 #embedded call [loop n]

_loop: n.int
  add n, 1
  call loop, 0 #omitted call return value storage
  neqc [_loop n] n, 1000

main:
  call _loop, 0

