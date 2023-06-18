#include <lib.h>

int pageref(u_long va) {
	u_long pte;

    if (is_mapped(va)) {
        pte = pt0[va >> VPN0_SHIFT];
        return pages[(PTE2PA(pte) - KERNBASE) >> VPN0_SHIFT].pp_ref;
    }

    return 0;
}
