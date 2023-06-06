#include <sbi.h>

// Base Extension (EID #0x10)
struct sbiret sbi_get_spec_version(void) {
    struct sbiret r;
    asm("li a7, 0x10");
	asm("li a6, 0x00");
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r.error) :);
    asm("add %0, a1, zero" : "=r"(r.value) :);
    return r;
}

struct sbiret sbi_get_impl_id(void) {
    struct sbiret r;
    asm("li a7, 0x10");
	asm("li a6, 0x01");
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r.error) :);
    asm("add %0, a1, zero" : "=r"(r.value) :);
    return r;
}

struct sbiret sbi_get_impl_version(void) {
    struct sbiret r;
    asm("li a7, 0x10");
	asm("li a6, 0x02");
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r.error) :);
    asm("add %0, a1, zero" : "=r"(r.value) :);
    return r;
}

struct sbiret sbi_probe_extension(long extension_id) {
    struct sbiret r;
    asm("li a7, 0x10");
	asm("li a6, 0x03");
    asm("add a0, %0, zero" : "=r"(extension_id) :);
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r.error) :);
    asm("add %0, a1, zero" : "=r"(r.value) :);
    return r;
}

struct sbiret sbi_get_mvendorid(void) {
    struct sbiret r;
    asm("li a7, 0x10");
	asm("li a6, 0x04");
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r.error) :);
    asm("add %0, a1, zero" : "=r"(r.value) :);
    return r;
}

struct sbiret sbi_get_marchid(void) {
    struct sbiret r;
    asm("li a7, 0x10");
	asm("li a6, 0x05");
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r.error) :);
    asm("add %0, a1, zero" : "=r"(r.value) :);
    return r;
}

struct sbiret sbi_get_mimpid(void) {
    struct sbiret r;
    asm("li a7, 0x10");
	asm("li a6, 0x06");
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r.error) :);
    asm("add %0, a1, zero" : "=r"(r.value) :);
    return r;
}

// Legacy Extensions (EIDs #0x00 - #0x0F)
long sbi_set_timer(uint64_t stime_value) {
    long r;
    asm("li a7, 0x00");
	asm("li a6, 0");
    asm("li a5, 0");
    asm("li a4, 0");
    asm("li a3, 0");
    asm("li a2, 0");
    asm("li a1, 0");
	asm("add a0, %0, zero" : : "r"(stime_value));
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r) :);
    return r;
}

long sbi_console_putchar(int ch) {
    long r;
    asm("li a7, 0x01");
	asm("li a6, 0");
	asm("add a0, %0, zero" : "=r"(ch) :);
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r) :);
    return r;
}

long sbi_console_getchar(void) {
    long r;
	asm("li a7, 0x02");
	asm("li a6, 0");
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r) :);
	return r;
}

long sbi_clear_ipi(void) {
    long r;
	asm("li a7, 0x03");
	asm("li a6, 0");
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r) :);
	return r;
}

long sbi_send_ipi(const unsigned long *hart_mask) {
    long r;
    asm("li a7, 0x04");
	asm("li a6, 0");
    asm("add a0, %0, zero" : "=r"(hart_mask) :);
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r) :);
    return r;
}

long sbi_remote_fence_i(const unsigned long *hart_mask) {
    long r;
    asm("li a7, 0x05");
	asm("li a6, 0");
    asm("add a0, %0, zero" : "=r"(hart_mask) :);
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r) :);
    return r;
}

long sbi_remote_sfence_vma(const unsigned long *hart_mask,
                           unsigned long start,
                           unsigned long size) {
    long r;
    asm("li a7, 0x06");
	asm("li a6, 0");
    asm("add a0, %0, zero" : "=r"(hart_mask) :);
    asm("add a1, %0, zero" : "=r"(start) :);
    asm("add a2, %0, zero" : "=r"(size) :);
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r) :);
    return r;
}

long sbi_remote_sfence_vma_asid(const unsigned long *hart_mask,
                                unsigned long start,
                                unsigned long size,
                                unsigned long asid) {
    long r;
    asm("li a7, 0x07");
	asm("li a6, 0");
    asm("add a0, %0, zero" : "=r"(hart_mask) :);
    asm("add a1, %0, zero" : "=r"(start) :);
    asm("add a2, %0, zero" : "=r"(size) :);
    asm("add a3, %0, zero" : "=r"(asid) :);
	asm("ecall");
    asm("add %0, a0, zero" : "=r"(r) :);
    return r;
}

void sbi_shutdown(void) {
	asm("li a7, 0x08");
	asm("li a6, 0");
	asm("ecall");
}
