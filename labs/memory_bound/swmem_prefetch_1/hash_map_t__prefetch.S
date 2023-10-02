       │     000000000000f9b0 <solution(hash_map_t const*, std::vector<int, std::allocator<int> > const&)>:
       │     solution(hash_map_t const*, std::vector<int, std::allocator<int> > const&):
       │       push       %rbp
       │       push       %r14
       │       push       %rbx
       │       mov        (%rsi),%r9
       │       mov        0x8(%rsi),%rax
       │       sub        %r9,%rax
       │     ↓ je         cd
       │       mov        (%r9),%ebx
       │       mov        0x18(%rdi),%r14
       │       mov        (%rdi),%r11
       │       xor        %r8d,%r8d
       │       mov        %ebx,%ecx
       │       and        %r14d,%ecx
       │       cmp        $0x5,%rax
       │       movslq     %ecx,%rcx
       │       prefetcht2 (%r11,%rcx,4)
       │     ↓ jb         d2
       │       sar        $0x2,%rax
       │       mov        $0x2,%r10d
       │       mov        $0x1,%ecx
       │       cmp        $0x3,%rax
       │       cmovae     %rax,%r10
       │       xor        %r8d,%r8d
       │     ↓ jmp        6e
       │       cs         nopw 0x0(%rax,%rax,1)
       │       xchg       %ax,%ax
     4 │ 60:   add        %eax,%r8d
    80 │ 63:   add        $0x1,%rcx
       │       mov        %esi,%ebx
    30 │       cmp        %r10,%rcx
       │     ↓ je         d4
    24 │ 6e:   mov        (%r9,%rcx,4),%esi
       │       xor        %edx,%edx
       │       mov        %esi,%eax
    24 │       and        %r14d,%eax
    11 │       cltq
    81 │       prefetcht2 (%r11,%rax,4)
       │       movslq     %ebx,%rax
   100 │       div        %r14
     6 │       movslq     %edx,%rax
  3219 │       cmpl       $0x7fffffff,(%r11,%rax,4)
    82 │     ↑ je         63
     4 │       xor        %eax,%eax
    11 │       test       %ebx,%ebx
       │     ↑ je         60
       │       nop
    21 │ a0:   movslq     %ebx,%rdx
     5 │       imul       $0x66666667,%rdx,%rbx
       │       mov        %edx,%ebp
   125 │       add        $0x9,%edx
    14 │       mov        %rbx,%rdi
     3 │       sar        $0x22,%rbx
     1 │       shr        $0x3f,%rdi
   113 │       add        %edi,%ebx
    15 │       lea        (%rbx,%rbx,1),%edi
    38 │       lea        (%rdi,%rdi,4),%edi
    17 │       sub        %edi,%ebp
   142 │       add        %ebp,%eax
     2 │       cmp        $0x12,%edx
   130 │     ↑ ja         a0
    23 │     ↑ jmp        60
       │ cd:   xor        %r8d,%r8d
       │     ↓ jmp        131
       │ d2:   mov        %ebx,%esi
       │ d4:   movslq     %esi,%rax
       │       xor        %edx,%edx
       │       xor        %ecx,%ecx
       │       div        %r14
       │       movslq     %edx,%rax
       │       cmpl       $0x7fffffff,(%r11,%rax,4)
       │     ↓ je         131
       │       test       %esi,%esi
       │     ↓ je         12b
       │       xor        %ecx,%ecx
       │       cs         nopw 0x0(%rax,%rax,1)
       │       nop
       │100:   movslq     %esi,%rax
       │       imul       $0x66666667,%rax,%rsi
       │       mov        %eax,%edi
       │       add        $0x9,%eax
       │       mov        %rsi,%rdx
       │       sar        $0x22,%rsi
       │       shr        $0x3f,%rdx
       │       add        %edx,%esi
       │       lea        (%rsi,%rsi,1),%edx
       │       lea        (%rdx,%rdx,4),%edx
       │       sub        %edx,%edi
       │       add        %edi,%ecx
       │       cmp        $0x12,%eax
       │     ↑ ja         100
       │12b:   add        %r8d,%ecx
       │       mov        %ecx,%r8d
       │131:   mov        %r8d,%eax
       │       pop        %rbx
       │       pop        %r14
       │       pop        %rbp
       │     ← ret
