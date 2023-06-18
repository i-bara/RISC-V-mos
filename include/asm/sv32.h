#ifndef _SV32_H_
#define _SV32_H_

// 决定了该页表项的其余部分是否有效（V = 1 时有效）。若 V = 0，则任何遍历到此页表项的虚址转换操作都会导致页错误。
#define PTE_V 0x00000001

// Valid bit. If 0 any address matching this entry will cause a tlb miss exception (TLBL/TLBS).
#define PTE_R 0x00000002
#define PTE_W 0x00000004
#define PTE_X 0x00000008
#define PTE_U 0x00000010
#define PTE_G 0x00000020
#define PTE_A 0x00000040
#define PTE_D 0x00000080

// Dirty bit, but really a write-enable bit. 1 to allow writes, 0 and any store using this
// translation will cause a tlb mod exception (TLB Mod).
#define PTE_RSW1 0x00000100
#define PTE_RSW2 0x00000200

// Uncacheable bit. 0 to make the access cacheable, 1 for uncacheable.
#define PTE_PERM 0x000003ff
#define PTE_PPN0 0x000ffc00
#define PTE_PPN1 0xfff00000
#define PTE_PPN  0xfffffc00

#define PN_SHIFT 10

#define PPN0_SHIFT 10
#define PPN1_SHIFT 20

#define PPN0(va) ((((u_long)(va)) >> PPN0_SHIFT) & 0x03FF)
#define PPN1(va) ((((u_long)(va)) >> PPN1_SHIFT) & 0x03FF)

#define PTE_OFFSET 0x00000fff
#define PTE_VPN0 0x003ff000
#define PTE_VPN1 0xffc00000
#define PTE_VPN  0xfffff000

#define VPN0_SHIFT 12
#define VPN1_SHIFT 22

#define VPN0(va) ((((u_long)(va)) >> VPN0_SHIFT) & 0x03FF)
#define VPN1(va) ((((u_long)(va)) >> VPN1_SHIFT) & 0x03FF)

#define PTE2PA(x) ((x & PTE_PPN) << 2)
#define PTE2PERM(x) (x & 0x3ff)
#define PA2PTE(x) ((x >> 2) & PTE_PPN)

// Copy On Write. Reserved for software, used by fork.
#define PTE_COW PTE_RSW1

// Shared memmory. Reserved for software, used by fork.
#define PTE_LIBRARY PTE_RSW2

// #define PDMAP (1 << PDSHIFT)
#define PAGE_SIZE (1 << VPN0_SHIFT)
#define LARGE_PAGE_SIZE (1 << VPN1_SHIFT)

#define BY2PG PAGE_SIZE
// #define BY2PG1 (1 << PPN1_SHIFT)
// #define BY2PG2 (1 << PPN2_SHIFT)

#endif /* !_SV32_H_ */
