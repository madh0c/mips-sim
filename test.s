main:
    li   $t0, 1
    li   $t1, 2
    add  $a0, $t1, $t0
    
    li   $v0, 1
    syscall

    li   $a0, '\n'      # printf("%c", '\n');
    li   $v0, 11
    syscall

