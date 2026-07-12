#ifndef _FLOWER_LOVE
#define _FLOWER_LOVE

#include <cstdint>

#define SYSCALL_EXIT 1
#define SYSCALL_FORK 2
#define SYSCALL_READ 3
#define SYSCALL_WRITE 4
#define SYSCALL_OPEN 5
#define SYSCALL_CLOSE 6
#define SYSCALL_SEEK 8

#define SYSCALL_WRITEFSBASE 29
#define SYSCALL_GET_THREAD_ID 30
#define SYSCALL_MMAP 31
#define SYSCALL_UNMAP 32
#define SYSCALL_MPROTECT 33

#define SYSCALL_MSLEEP 101
#define SYSCALL_MTIME 102

#define SYSCALL_ARCHCTL 157

#define FLOWER_ASM_CLOBBER "memory", "rcx", "r11"

static inline uint64_t syscall(uint64_t syscall_type, uint64_t arg1) {
	uint64_t ret;
	__asm__ volatile("syscall" : "=a"(ret) : "a"(syscall_type), "D"(arg1) : FLOWER_ASM_CLOBBER);
	return ret;
}

static inline uint64_t syscall(int syscall_type, uint64_t arg1, uint64_t arg2) {
	uint64_t ret;
	__asm__ volatile("syscall"
	                 : "=a"(ret)
	                 : "a"(syscall_type), "D"(arg1), "S"(arg2)
	                 : FLOWER_ASM_CLOBBER);
	return ret;
}

static inline uint64_t syscall(int syscall_type, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	uint64_t ret;
	__asm__ volatile("syscall"
	                 : "=a"(ret)
	                 : "a"(syscall_type), "D"(arg1), "S"(arg2), "d"(arg3)
	                 : FLOWER_ASM_CLOBBER);
	return ret;
}

static inline uint64_t
syscall(int syscall_type, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
	uint64_t ret;
	register uint64_t r10 asm("r10") = arg4; // move arg4 into r10
	__asm__ volatile("syscall"
	                 : "=a"(ret)
	                 : "a"(syscall_type), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
	                 : FLOWER_ASM_CLOBBER);
	return ret;
}

static inline uint64_t syscall(
    int syscall_type, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5
) {
	uint64_t ret;
	register uint64_t r10 asm("r10") = arg4;
	register uint64_t r8 asm("r8") = arg5;
	__asm__ volatile("syscall"
	                 : "=a"(ret)
	                 : "a"(syscall_type), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
	                 : FLOWER_ASM_CLOBBER);
	return ret;
}

static inline uint64_t syscall(
    int syscall_type,
    uint64_t arg1,
    uint64_t arg2,
    uint64_t arg3,
    uint64_t arg4,
    uint64_t arg5,
    uint64_t arg6
) {
	uint64_t ret;
	register uint64_t r10 asm("r10") = arg4;
	register uint64_t r8 asm("r8") = arg5;
	register uint64_t r9 asm("r9") = arg6;
	__asm__ volatile(
	    "syscall"
	    : "=a"(ret)
	    : "a"(syscall_type), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8), "r"(r9)
	    : FLOWER_ASM_CLOBBER
	);
	return ret;
}

#endif
