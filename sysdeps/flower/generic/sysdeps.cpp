#include "mlibc/tcb.hpp"
#include <abi-bits/errno.h>
#include <bits/ensure.h>
#include <cstdint>
#include <flower/syscall.hpp>
#include <mlibc/all-sysdeps.hpp>
#include <string.h>

#define UNUSED(x) (void)(x)

#define FD_STDOUT 1

#define STUB()                                                                                     \
	({                                                                                             \
		__ensure(!"STUB function was called");                                                     \
		__builtin_unreachable();                                                                   \
	})

namespace mlibc {

void sys_libc_log(const char *msg) { syscall(SYSCALL_WRITE, FD_STDOUT, (long)msg, strlen(msg)); }

[[noreturn]] void sys_libc_panic() {
	sys_libc_log("MLIBC PANIC!");
	sys_exit(1);
	__builtin_trap();
}

int sys_isatty(int fd) {
	if (fd <= 2) {
		return 0;
	}

	return ENOTTY;
}

int sys_open(const char *path, int flags, mode_t mode, int *fd) {
	long ret = syscall(SYSCALL_OPEN, (long)path, flags, mode);
	if (ret < 0) {
		return -ret;
	}
	*fd = (int)ret;
	return 0;
}

int sys_read(int fd, void *buf, size_t count, ssize_t *bytes_read) {
	long out = syscall(SYSCALL_READ, fd, (long)buf, (long)count);
	if (out < 0) {
		return -out;
	}
	*bytes_read = out;
	return 0;
}

int sys_write(int fd, void const *buf, size_t size, ssize_t *ret) {
	long out = syscall(SYSCALL_WRITE, fd, (long)buf, (long)size);
	if (out < 0) {
		return -out;
	}
	*ret = out;
	return 0;
}

int sys_seek(int fd, off_t offset, int whence, off_t *new_offset) {
	long out = syscall(SYSCALL_SEEK, fd, (long)offset, (long)whence);
	if (out < 0) {
		return -out;
	}
	*new_offset = out;
	return 0;
}

int sys_close(int fd) {
	long ret = syscall(SYSCALL_CLOSE, fd);
	if (ret < 0) {
		return -ret;
	}
	return 0;
}

int sys_mkdir(const char *, mode_t) { return -1; }

int sys_tcb_set(void *pointer) {
	syscall(SYSCALL_WRITEFSBASE, (uint64_t)pointer);
	return 0;
}

int sys_anon_allocate(size_t size, void **pointer) {
	return sys_vm_map(
	    NULL, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0, pointer
	);
}

int sys_anon_free(void *pointer, size_t size) {
	uint64_t ret = syscall(SYSCALL_UNMAP, (uint64_t)pointer, size);
	if (ret != 0) {
		return -ret;
	}
	return 0;
}

int sys_vm_map(void *hint, size_t size, int prot, int flags, int fd, off_t offset, void **window) {
	uintptr_t ret = syscall(SYSCALL_MMAP, (uint64_t)hint, size, prot, flags, fd, offset);
	if (ret == 0) {
		return ENOMEM;
	}
	*window = (void *)ret;
	return 0;
}

int sys_vm_unmap(void *addr, size_t size) {
	uint64_t ret = syscall(SYSCALL_UNMAP, (uint64_t)addr, size);
	if (ret != 0) {
		return -ret;
	}
	return 0;
}

int sys_clock_get(int clock_id, time_t *tp, long *nsec) {
	uint64_t millis = 0;

	syscall(SYSCALL_MTIME, (uintptr_t)&millis);

	if (tp) {
		*tp = (time_t)(millis / 1000);
	}

	if (nsec) {
		*nsec = (long)((millis % 1000) * 1000000);
	}

	return 0;
}

int sys_sleep(time_t *sec, long *nsec) {
	uint64_t millis = 0;

	if (sec) {
		millis += (uint64_t)(*sec) * 1000;
	}

	if (nsec) {
		millis += (uint64_t)(*nsec) / 1000000;
	}

	syscall(SYSCALL_MSLEEP, (uintptr_t)&millis);

	return 0;
}

void sys_exit(int status) {
	syscall(SYSCALL_EXIT, status);
	__builtin_unreachable();
}

int sys_futex_wake(int *) { STUB(); }
int sys_futex_wait(int *, int, timespec const *) { STUB(); }

} // namespace mlibc
