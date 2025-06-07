#include <cache_utils.h>
#include <linux/perf_event.h>
#include <pmu_utils.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  int n;
  if (argc > 1)
  {
    n = strtoll(argv[1], NULL, 0);
  }
  else
  {
    n = 1000;
  }

  perf_init();

  long long start_time, end_time;
  long long start_cycle, end_cycle;

  asm volatile("rdtime %0" : "=r"(start_time)::);
  asm volatile("rdcycle %0" : "=r"(start_cycle)::);
  asm volatile("csrr %0, cycle" : "=r"(start_cycle));

  for (volatile int i = 0; i < n; i++)
  {
    asm volatile("nop");
  }
  asm volatile("rdcycle %0" : "=r"(end_cycle)::);
  asm volatile("rdtime %0" : "=r"(end_time)::);

  printf("start cycle: %llu\n", start_cycle);
  printf("  end cycle: %llu\n", end_cycle);
  printf("used cycles: %lld\n", end_cycle - start_cycle);

  printf("start time: %lld\n", start_time);
  printf("  end time: %lld\n", end_time);
  printf("used times: %lld\n", end_time - start_time);
}