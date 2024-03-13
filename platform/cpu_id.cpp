

#include "cpu_id.h"

#ifdef MOTOR_COMPILER_MSC
#include <limits.h>
#include <intrin.h>
//typedef unsigned __int32  uint32_t;

#elif MOTOR_COMPILER_GNU
#include <cpuid.h>
#else
//#include <stdint.h>
#endif

using namespace motor::platform ;

// https://wiki.osdev.org/CPUID
// Vendor strings from CPUs.
#define CPUID_VENDOR_AMD           "AuthenticAMD"
#define CPUID_VENDOR_AMD_OLD       "AMDisbetter!" // Early engineering samples of AMD K5 processor
#define CPUID_VENDOR_INTEL         "GenuineIntel"
#define CPUID_VENDOR_VIA           "VIA VIA VIA "
#define CPUID_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPUID_VENDOR_TRANSMETA_OLD "TransmetaCPU"
#define CPUID_VENDOR_CYRIX         "CyrixInstead"
#define CPUID_VENDOR_CENTAUR       "CentaurHauls"
#define CPUID_VENDOR_NEXGEN        "NexGenDriven"
#define CPUID_VENDOR_UMC           "UMC UMC UMC "
#define CPUID_VENDOR_SIS           "SiS SiS SiS "
#define CPUID_VENDOR_NSC           "Geode by NSC"
#define CPUID_VENDOR_RISE          "RiseRiseRise"
#define CPUID_VENDOR_VORTEX        "Vortex86 SoC"
#define CPUID_VENDOR_AO486         "MiSTer AO486"
#define CPUID_VENDOR_AO486_OLD     "GenuineAO486"
#define CPUID_VENDOR_ZHAOXIN       "  Shanghai  "
#define CPUID_VENDOR_HYGON         "HygonGenuine"
#define CPUID_VENDOR_ELBRUS        "E2K MACHINE "
 
// Vendor strings from hypervisors.
#define CPUID_VENDOR_QEMU          "TCGTCGTCGTCG"
#define CPUID_VENDOR_KVM           " KVMKVMKVM  "
#define CPUID_VENDOR_VMWARE        "VMwareVMware"
#define CPUID_VENDOR_VIRTUALBOX    "VBoxVBoxVBox"
#define CPUID_VENDOR_XEN           "XenVMMXenVMM"
#define CPUID_VENDOR_HYPERV        "Microsoft Hv"
#define CPUID_VENDOR_PARALLELS     " prl hyperv "
#define CPUID_VENDOR_PARALLELS_ALT " lrpepyh vr " // Sometimes Parallels incorrectly encodes "prl hyperv" as "lrpepyh vr" due to an endianness mismatch.
#define CPUID_VENDOR_BHYVE         "bhyve bhyve "
#define CPUID_VENDOR_QNX           " QNXQVMBSQG "

// https://wiki.osdev.org/CPUID
enum instuction_sets
{
    CPUID_IA_ECX_SSE3         = 1 << 0, 
    CPUID_IA_ECX_PCLMUL       = 1 << 1,
    CPUID_IA_ECX_DTES64       = 1 << 2,
    CPUID_IA_ECX_MONITOR      = 1 << 3,  
    CPUID_IA_ECX_DS_CPL       = 1 << 4,  
    CPUID_IA_ECX_VMX          = 1 << 5,  
    CPUID_IA_ECX_SMX          = 1 << 6,  
    CPUID_IA_ECX_EST          = 1 << 7,  
    CPUID_IA_ECX_TM2          = 1 << 8,  
    CPUID_IA_ECX_SSSE3        = 1 << 9,  
    CPUID_IA_ECX_CID          = 1 << 10,
    CPUID_IA_ECX_SDBG         = 1 << 11,
    CPUID_IA_ECX_FMA          = 1 << 12,
    CPUID_IA_ECX_CX16         = 1 << 13, 
    CPUID_IA_ECX_XTPR         = 1 << 14, 
    CPUID_IA_ECX_PDCM         = 1 << 15, 
    CPUID_IA_ECX_PCID         = 1 << 17, 
    CPUID_IA_ECX_DCA          = 1 << 18, 
    CPUID_IA_ECX_SSE4_1       = 1 << 19, 
    CPUID_IA_ECX_SSE4_2       = 1 << 20, 
    CPUID_IA_ECX_X2APIC       = 1 << 21, 
    CPUID_IA_ECX_MOVBE        = 1 << 22, 
    CPUID_IA_ECX_POPCNT       = 1 << 23, 
    CPUID_IA_ECX_TSC          = 1 << 24, 
    CPUID_IA_ECX_AES          = 1 << 25, 
    CPUID_IA_ECX_XSAVE        = 1 << 26, 
    CPUID_IA_ECX_OSXSAVE      = 1 << 27, 
    CPUID_IA_ECX_AVX          = 1 << 28,
    CPUID_IA_ECX_F16C         = 1 << 29,
    CPUID_IA_ECX_RDRAND       = 1 << 30,
    CPUID_IA_ECX_HYPERVISOR   = 1 << 31,
 
    CPUID_IA_EDX_FPU          = 1 << 0,  
    CPUID_IA_EDX_VME          = 1 << 1,  
    CPUID_IA_EDX_DE           = 1 << 2,  
    CPUID_IA_EDX_PSE          = 1 << 3,  
    CPUID_IA_EDX_TSC          = 1 << 4,  
    CPUID_IA_EDX_MSR          = 1 << 5,  
    CPUID_IA_EDX_PAE          = 1 << 6,  
    CPUID_IA_EDX_MCE          = 1 << 7,  
    CPUID_IA_EDX_CX8          = 1 << 8,  
    CPUID_IA_EDX_APIC         = 1 << 9,  
    CPUID_IA_EDX_SEP          = 1 << 11, 
    CPUID_IA_EDX_MTRR         = 1 << 12, 
    CPUID_IA_EDX_PGE          = 1 << 13, 
    CPUID_IA_EDX_MCA          = 1 << 14, 
    CPUID_IA_EDX_CMOV         = 1 << 15, 
    CPUID_IA_EDX_PAT          = 1 << 16, 
    CPUID_IA_EDX_PSE36        = 1 << 17, 
    CPUID_IA_EDX_PSN          = 1 << 18, 
    CPUID_IA_EDX_CLFLUSH      = 1 << 19, 
    CPUID_IA_EDX_DS           = 1 << 21, 
    CPUID_IA_EDX_ACPI         = 1 << 22, 
    CPUID_IA_EDX_MMX          = 1 << 23, 
    CPUID_IA_EDX_FXSR         = 1 << 24, 
    CPUID_IA_EDX_SSE          = 1 << 25, 
    CPUID_IA_EDX_SSE2         = 1 << 26, 
    CPUID_IA_EDX_SS           = 1 << 27, 
    CPUID_IA_EDX_HTT          = 1 << 28, 
    CPUID_IA_EDX_TM           = 1 << 29, 
    CPUID_IA_EDX_IA64         = 1 << 30,
    CPUID_IA_EDX_PBE          = 1 << 31
};

enum extended_instuction_sets
{
    CPUID_IA_ECX_AVX512_VBM_1       = 1 << 1,   // vector bit manipulation 1
    CPUID_IA_ECX_AVX512_VBM_2       = 1 << 6,   // vector bit manipulation 1

    CPUID_IA_EBX_AVX_2              = 1 << 5,   // AVX 2
    CPUID_IA_EBX_AVX512_F           = 1 << 16,  // AVX 512 Foundation
    CPUID_IA_EBX_AVX512_DQ          = 1 << 17,  // AVX 512 DOUBLE/QUAD Word
    CPUID_IA_EBX_AVX512_IFMA        = 1 << 21,  // AVX 512 integer fma
} ;

//*********************************************************************************
cpu_id::cpu_id( unsigned const i ) noexcept
{
#ifdef MOTOR_COMPILER_MSC

    memset( regs, 0, sizeof( regs ) ) ;
    __cpuid( (int *)regs, (int)i ) ;

#elif MOTOR_COMPILER_GNU

    __cpuid( (int)i, (int *)regs ) ;

#else
    asm volatile
        ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
        : "a" (i), "c" (0));
    // ECX is set to zero for CPUID function 4
#endif
}

//*********************************************************************************
motor::string_t cpu_id::vendor_string( void_t ) noexcept 
{
    this_t const cid( 0 ) ;

    motor::string_t vendor ;

    vendor += motor::string_t( (const char *)&cid.ebx(), 4 ) ;
    vendor += motor::string_t( (const char *)&cid.edx(), 4 ) ;
    vendor += motor::string_t( (const char *)&cid.ecx(), 4 ) ;

    return vendor ;
}

//*********************************************************************************
// https://en.wikipedia.org/wiki/CPUID#EAX%3D80000002h%2C80000003h%2C80000004h%3A_Processor_Brand_String
motor::string_t cpu_id::brand_string( void_t ) noexcept 
{
    motor::string_t brand ;

    {
        this_t const cid( 0x80000000 ) ;

        if( cid.eax() < 0x80000004 ) return "Unknown" ;
    }

    {
        this_t const cid( 0x80000002 ) ;

        brand += motor::string_t( (const char *)&cid.eax(), 4 ) ;
        brand += motor::string_t( (const char *)&cid.ebx(), 4 ) ;
        brand += motor::string_t( (const char *)&cid.ecx(), 4 ) ;
        brand += motor::string_t( (const char *)&cid.edx(), 4 ) ;
    }

    {
        this_t const cid( 0x80000003 ) ;

        brand += motor::string_t( (const char *)&cid.eax(), 4 ) ;
        brand += motor::string_t( (const char *)&cid.ebx(), 4 ) ;
        brand += motor::string_t( (const char *)&cid.ecx(), 4 ) ;
        brand += motor::string_t( (const char *)&cid.edx(), 4 ) ;
    }

    {
        this_t const cid( 0x80000004 ) ;

        brand += motor::string_t( (const char *)&cid.eax(), 4 ) ;
        brand += motor::string_t( (const char *)&cid.ebx(), 4 ) ;
        brand += motor::string_t( (const char *)&cid.ecx(), 4 ) ;
        brand += motor::string_t( (const char *)&cid.edx(), 4 ) ;
    }

    return brand ;
}

//*********************************************************************************
// https://en.wikipedia.org/wiki/CPUID
motor::string_t cpu_id::instruction_sets_string( void_t ) noexcept 
{
    motor::string_t instruction_sets ;

    // instruction sets
    {
        this_t const cid( 1 ) ;

        if( cid.ecx() & CPUID_IA_ECX_SSE3 )
        {
            instruction_sets += "SSE3," ;
        }

        if( cid.ecx() & CPUID_IA_ECX_SSSE3 )
        {
            instruction_sets += " SSSE3," ;
        }

        if( cid.edx() & CPUID_IA_EDX_SSE2 )
        {
            instruction_sets += " SSE2," ;
        }

        if( cid.edx() & CPUID_IA_EDX_SSE )
        {
            instruction_sets += " SSE2," ;
        }

        if( cid.ecx() & CPUID_IA_ECX_FMA )
        {
            instruction_sets += " FMA," ;
        }

        if( cid.ecx() & CPUID_IA_ECX_SSE4_1 )
        {
            instruction_sets += " SSE 4.1," ;
        }

        if( cid.ecx() & CPUID_IA_ECX_SSE4_2 )
        {
            instruction_sets += " SSE 4.2," ;
        }

        if( cid.ecx() & CPUID_IA_ECX_AVX )
        {
            instruction_sets += " AVX_1," ;
        }
    }

    // extended instruction sets
    {
        this_t const cid( 7 ) ;

        if( cid.ecx() & CPUID_IA_ECX_AVX512_VBM_1 )
        {
            instruction_sets += " AVX512_VBM_1," ;
        }

        if( cid.ecx() & CPUID_IA_ECX_AVX512_VBM_2 )
        {
            instruction_sets += " AVX512_VBM_2," ;
        }

        if( cid.ebx() & CPUID_IA_EBX_AVX_2 )
        {
            instruction_sets += " AVX_2," ;
        }

        if( cid.ebx() & CPUID_IA_EBX_AVX512_F )
        {
            instruction_sets += " AVX_512_F," ;
        }
        
    }

    return instruction_sets ;
}

