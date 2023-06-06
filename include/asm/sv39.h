#ifndef _SV39_H_
#define _SV39_H_

// 决定了该页表项的其余部分是否有效（V = 1 时有效）。若 V = 0，则任何遍历到此页表项的虚址转换操作都会导致页错误。
#define PTE_V 0x0000000000000001L

// Valid bit. If 0 any address matching this entry will cause a tlb miss exception (TLBL/TLBS).
#define PTE_R 0x0000000000000002L
#define PTE_W 0x0000000000000004L
#define PTE_X 0x0000000000000008L
#define PTE_U 0x0000000000000010L
#define PTE_G 0x0000000000000020L
#define PTE_A 0x0000000000000040L
#define PTE_D 0x0000000000000080L

// Dirty bit, but really a write-enable bit. 1 to allow writes, 0 and any store using this
// translation will cause a tlb mod exception (TLB Mod).
#define PTE_RSW1 0x0000000000000100L
#define PTE_RSW2 0x0000000000000200L

// Uncacheable bit. 0 to make the access cacheable, 1 for uncacheable.
#define PTE_PERM 0x00000000000003ffL
#define PTE_PPN0 0x000000000007fc00L
#define PTE_PPN1 0x000000000ff80000L
#define PTE_PPN2 0x003ffffff0000000L
#define PTE_PPN  0x003ffffffffffc00L

#define PPN0_SHIFT 10
#define PPN1_SHIFT 19
#define PPN2_SHIFT 28

#define PPN0(va) ((((u_long)(va)) >> PPN0_SHIFT) & 0x01FF)
#define PPN1(va) ((((u_long)(va)) >> PPN1_SHIFT) & 0x01FF)
#define PPN2(va) ((((u_long)(va)) >> PPN2_SHIFT) & 0x01FF)

#define PTE_OFFSET 0x0000000000000fffL
#define PTE_VPN0 0x00000000001ff000L
#define PTE_VPN1 0x000000003fe00000L
#define PTE_VPN2 0x0000007fc0000000L
#define PTE_VPN  0x0000007ffffff000L

#define VPN0_SHIFT 12
#define VPN1_SHIFT 21
#define VPN2_SHIFT 30

#define VPN0(va) ((((u_long)(va)) >> VPN0_SHIFT) & 0x01FF)
#define VPN1(va) ((((u_long)(va)) >> VPN1_SHIFT) & 0x01FF)
#define VPN2(va) ((((u_long)(va)) >> VPN2_SHIFT) & 0x01FF)

#define PTE2PA(x) ((x & PTE_PPN) << 2)
#define PTE2PERM(x) (x & 0x3ffL)
#define PA2PTE(x) ((x >> 2) & PTE_PPN)

// Copy On Write. Reserved for software, used by fork.
#define PTE_COW PTE_RSW1

// Shared memmory. Reserved for software, used by fork.
#define PTE_LIBRARY PTE_RSW2

// #define PDMAP (1 << PDSHIFT)
#define PAGE_SIZE (1 << VPN0_SHIFT)

#define BY2PG  (1 << PPN0_SHIFT)
#define BY2PG1 (1 << PPN1_SHIFT)
#define BY2PG2 (1 << PPN2_SHIFT)

#endif /* !_SV39_H_ */
