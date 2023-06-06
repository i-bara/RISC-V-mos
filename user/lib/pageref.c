#include <lib.h>

int pageref(u_long va) {
	u_long pte;

    if (pt2[va >> VPN2_SHIFT] & PTE_V) {			// 一旦进入必须写时复制，但是必须进入才能写时复制，矛盾
        if (pt1[va >> VPN1_SHIFT] & PTE_V) {
            if (pt0[va >> VPN0_SHIFT] & PTE_V) {	// 内核在 exofork 内 duppage，这样能够实现快速的映射
                pte = pt0[va >> VPN0_SHIFT];
                return pages[(PTE2PA(pte) - KERNBASE) >> VPN0_SHIFT].pp_ref;
            }
        }
    }

    return 0;
}
