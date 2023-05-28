#ifndef _SBI_H_
#define _SBI_H_

#include <stdint.h>

struct sbiret {
    long error;
    long value;
};

// Base Extension (EID #0x10)
struct sbiret sbi_get_spec_version(void);
struct sbiret sbi_get_impl_id(void);
struct sbiret sbi_get_impl_version(void);
struct sbiret sbi_probe_extension(long extension_id);
struct sbiret sbi_get_mvendorid(void);
struct sbiret sbi_get_marchid(void);
struct sbiret sbi_get_mimpid(void);

// Legacy Extensions (EIDs #0x00 - #0x0F)
long sbi_set_timer(uint64_t stime_value);
long sbi_console_putchar(int ch);
long sbi_console_getchar(void);
long sbi_clear_ipi(void);
long sbi_send_ipi(const unsigned long *hart_mask);
long sbi_remote_fence_i(const unsigned long *hart_mask);
long sbi_remote_sfence_vma(const unsigned long *hart_mask,
                           unsigned long start,
                           unsigned long size);
long sbi_remote_sfence_vma_asid(const unsigned long *hart_mask,
                                unsigned long start,
                                unsigned long size,
                                unsigned long asid);
void sbi_shutdown(void);

#endif /* !_SBI_H_ */
