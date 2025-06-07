/**
 * @url https://stackoverflow.com/questions/35923834
 */

#include <asm/unistd.h>
#include <inttypes.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags)
{
  int ret;

  ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
  return ret;
}

int main(int argc, char **argv)
{
  long long start_count, end_count;

  int n;
  if (argc > 1)
  {
    n = strtoll(argv[1], NULL, 0);
  }
  else
  {
    n = 1000;
  }

  int fd;
  struct perf_event_attr pe;
  memset(&pe, 0, sizeof(struct perf_event_attr));
  pe.type = PERF_TYPE_HARDWARE;
  pe.config = PERF_COUNT_HW_CPU_CYCLES;
  pe.size = sizeof(struct perf_event_attr);
  pe.disabled = 1;
  pe.exclude_kernel = 1;
  pe.exclude_hv = 1; // Don't count hypervisor events.

  fd = perf_event_open(&pe, 0, -1, -1, 0);
  if (fd == -1)
  {
    fprintf(stderr, "Error opening leader %llx\n", pe.config);
    exit(EXIT_FAILURE);
  }

  ioctl(fd, PERF_EVENT_IOC_RESET, 0);
  ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
  read(fd, &start_count, sizeof(long long));

  for (volatile int i = 0; i < n; i++)
  {
    asm volatile("nop");
  }

  read(fd, &end_count, sizeof(long long));
  ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);

  printf("  start cycle: %lld\n", start_count);
  printf("    end cycle: %lld\n", end_count);
  printf("hw cpu cycles: %lld\n", end_count - start_count);

  close(fd);
}