#include "mlibc/tcb.hpp"
#include <abi-bits/errno.h>
#include <bits/ensure.h>
#include <cstdint>
#include <mlibc/all-sysdeps.hpp>
#include <string.h>

#define UNUSED(x) (void)(x)

#define FD_STDOUT 1

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

#define SYSCALL_MSLEEP 101
#define SYSCALL_MTIME 102

uint64_t syscall1(uint64_t syscall_type, uint64_t arg1) {
	uint64_t ret;
	__asm__ volatile("syscall" : "=a"(ret) : "a"(syscall_type), "D"(arg1) : "rcx", "r11", "memory");
	return ret;
}

uint64_t syscall2(int syscall_type, uint64_t arg1, uint64_t arg2) {
	uint64_t ret;
	__asm__ volatile("syscall"
	                 : "=a"(ret)
	                 : "a"(syscall_type), "D"(arg1), "S"(arg2)
	                 : "rcx", "r11", "memory");
	return ret;
}

uint64_t syscall3(int syscall_type, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
	uint64_t ret;
	__asm__ volatile("syscall"
	                 : "=a"(ret)
	                 : "a"(syscall_type), "D"(arg1), "S"(arg2), "d"(arg3)
	                 : "rcx", "r11", "memory");
	return ret;
}

uint64_t syscall4(int syscall_type, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
	uint64_t ret;
	register uint64_t r10 asm("r10") = arg4; // move arg4 into r10
	__asm__ volatile("syscall"
	                 : "=a"(ret)
	                 : "a"(syscall_type), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10)
	                 : "rcx", "r11", "memory");
	return ret;
}

uint64_t syscall5(
    int syscall_type, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5
) {
	uint64_t ret;
	register uint64_t r10 asm("r10") = arg4;
	register uint64_t r8 asm("r8") = arg5;
	__asm__ volatile("syscall"
	                 : "=a"(ret)
	                 : "a"(syscall_type), "D"(arg1), "S"(arg2), "d"(arg3), "r"(r10), "r"(r8)
	                 : "rcx", "r11", "memory");
	return ret;
}

uint64_t syscall6(
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
	    : "rcx", "r11", "memory"
	);
	return ret;
}

void syscall_print(const char *s, size_t len) {
	syscall3(SYSCALL_WRITE, FD_STDOUT, (long)s, (long)len);
}

void syscall_write_fsbase(long fsbase) {
	syscall_print("syscall_write_fsbase called\n", 29);
	syscall1(SYSCALL_WRITEFSBASE, fsbase);
}

#define STUB()                                                                                     \
	({                                                                                             \
		__ensure(!"STUB function was called");                                                     \
		__builtin_unreachable();                                                                   \
	})

namespace mlibc {

void sys_libc_panic() {
	sys_libc_log("MLIBC PANIC\n");
	sys_exit(-1);
	__builtin_trap();
}

void sys_libc_log(const char *msg) { syscall_print(msg, strlen(msg)); }

int sys_isatty(int fd) {
	if (fd <= 2) {
		return 0;
	}

	return ENOTTY;
}

int sys_open(const char *path, int flags, mode_t mode, int *fd) {
	long ret = syscall3(SYSCALL_OPEN, (long)path, flags, mode);
	if (ret < 0) {
		return -ret;
	}
	*fd = (int)ret;
	return 0;
}

int sys_read(int fd, void *buf, size_t count, ssize_t *bytes_read) {
	long out = syscall3(SYSCALL_READ, fd, (long)buf, (long)count);
	if (out < 0) {
		return -out;
	}
	*bytes_read = out;
	return 0;
}

int sys_write(int fd, void const *buf, size_t size, ssize_t *ret) {
	long out = syscall3(SYSCALL_WRITE, fd, (long)buf, (long)size);
	if (out < 0) {
		return -out;
	}
	*ret = out;
	return 0;
}

int sys_seek(int fd, off_t offset, int whence, off_t *new_offset) {
	long out = syscall3(SYSCALL_SEEK, fd, (long)offset, (long)whence);
	if (out < 0) {
		return -out;
	}
	*new_offset = out;
	return 0;
}

int sys_close(int fd) {
	long ret = syscall1(SYSCALL_CLOSE, fd);
	if (ret < 0) {
		return -ret;
	}
	return 0;
}

int sys_mkdir(const char *, mode_t) { return -1; }

int sys_tcb_set(void *pointer) {
	syscall_write_fsbase((uint64_t)pointer);
	return 0;
}

int sys_anon_allocate(size_t size, void **pointer) {
	return sys_vm_map(
	    NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0, pointer
	);
}

int sys_vm_map(void *hint, size_t size, int prot, int flags, int fd, off_t offset, void **window) {
	uintptr_t ret = syscall6(SYSCALL_MMAP, (uint64_t)hint, size, prot, flags, fd, offset);
	if (ret == 0) {
		return ENOMEM;
	}
	*window = (void *)ret;
	return 0;
}

int sys_vm_unmap(void *addr, size_t size) {
	uint64_t ret = syscall2(SYSCALL_UNMAP, (uint64_t)addr, size);
	if (ret != 0) {
		return -ret;
	}
	return 0;
}

int sys_anon_free(void *pointer, size_t size) {
	uint64_t ret = syscall2(SYSCALL_UNMAP, (uint64_t)pointer, size);
	if (ret != 0) {
		return -ret;
	}
	return 0;
}

int sys_clock_get(int clock_id, time_t *tp, long *nsec) {
	uint64_t millis = 0;

	syscall1(SYSCALL_MTIME, (uintptr_t)&millis);

	if (tp) {
		*tp = (time_t)(millis / 1000);
	}

	if (nsec) {
		*nsec = (long)((millis % 1000) * 1000000);
	}

	return 0;
}

int sys_sleep(time_t *sec, long *nsec)
{
	uint64_t millis = 0;

	if (sec) {
		millis += (uint64_t)(*sec) * 1000;
	}

	if (nsec) {
		millis += (uint64_t)(*nsec) / 1000000;
	}

	syscall1(SYSCALL_MSLEEP, (uintptr_t)&millis);

	return 0;
}

void sys_exit(int status) {
	syscall1(SYSCALL_EXIT, status);
	__builtin_unreachable();
}

int sys_futex_wake(int *) { STUB(); }
int sys_futex_wait(int *, int, timespec const *) { STUB(); }

} // namespace mlibc
