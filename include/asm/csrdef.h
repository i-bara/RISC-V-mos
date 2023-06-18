#ifndef _CSRDEF_H_
#define _CSRDEF_H_

#ifdef RISCV32

#define SIE
#define SIE_WPRI 0xfffffccc
#define SIE_SEIE 0x00000200
#define SIE_UEIE 0x00000100
#define SIE_STIE 0x00000020
#define SIE_UTIE 0x00000010
#define SIE_SSIE 0x00000002
#define SIE_USIE 0x00000001

#define SIP
#define SIP_WIRI 0xfffffccc
#define SIP_SEIP 0x00000200
#define SIP_UEIP 0x00000100
#define SIP_STIP 0x00000020
#define SIP_UTIP 0x00000010
#define SIP_SSIP 0x00000002
#define SIP_USIP 0x00000001

#define STVEC
#define STVEC_BASE 0xfffffffc
#define STVEC_MODE 0x00000003

#define SCAUSE
#define SCAUSE_INT 0x80000000
#define SCAUSE_EXC 0x7fffffff

#define STVAL

#define SSTATUS
#define SSTATUS_SD 0x80000000
#define SSTATUS_MXR 0x00080000
#define SSTATUS_SUM 0x00040000
#define SSTATUS_XS 0x00018000
#define SSTATUS_FS 0x00006000
#define SSTATUS_SPP 0x00000100
#define SSTATUS_SPIE 0x00000020
#define SSTATUS_UPIE 0x00000010
#define SSTATUS_SIE 0x00000002
#define SSTATUS_UIE 0x00000001

#define SSCRATCH

#define SATP
#define SATP_MODE 0x80000000
#define SATP_MODE_BARE 0x00000000
#define SATP_MODE_SV32 0x80000000
#define SATP_ASID 0x7fc00000
#define SATP_PPN 0x003fffff

#else // riscv64

#define SIE
#define SIE_WPRI 0xfffffffffffffccc
#define SIE_SEIE 0x0000000000000200
#define SIE_UEIE 0x0000000000000100
#define SIE_STIE 0x0000000000000020
#define SIE_UTIE 0x0000000000000010
#define SIE_SSIE 0x0000000000000002
#define SIE_USIE 0x0000000000000001

#define SIP
#define SIP_WIRI 0xfffffffffffffccc
#define SIP_SEIP 0x0000000000000200
#define SIP_UEIP 0x0000000000000100
#define SIP_STIP 0x0000000000000020
#define SIP_UTIP 0x0000000000000010
#define SIP_SSIP 0x0000000000000002
#define SIP_USIP 0x0000000000000001

#define STVEC
#define STVEC_BASE 0xfffffffffffffffc
#define STVEC_MODE 0x0000000000000003

#define SCAUSE
#define SCAUSE_INT 0x8000000000000000
#define SCAUSE_EXC 0x7fffffffffffffff

#define STVAL

#define SSTATUS
#define SSTATUS_SD 0x8000000000000000
#define SSTATUS_MXR 0x0000000000080000
#define SSTATUS_SUM 0x0000000000040000
#define SSTATUS_XS 0x0000000000018000
#define SSTATUS_FS 0x0000000000006000
#define SSTATUS_SPP 0x0000000000000100
#define SSTATUS_SPIE 0x0000000000000020
#define SSTATUS_UPIE 0x0000000000000010
#define SSTATUS_SIE 0x0000000000000002
#define SSTATUS_UIE 0x0000000000000001

#define SSCRATCH

#define SATP
#define SATP_MODE 0xf000000000000000
#define SATP_MODE_BARE 0x0000000000000000
#define SATP_MODE_SV39 0x8000000000000000
#define SATP_MODE_SV48 0x9000000000000000
#define SATP_ASID 0x0ffff00000000000
#define SATP_PPN 0x00000fffffffffff

#endif

#endif /* !_CSRDEF_H_ */
