#ifndef __PERF_H__
#define __PERF_H__
#include <pmu_utils.h>

int test_pref()
{
  perf_init();

  long long start_count, end_count;
  read(perf_fd, &start_count, sizeof(long long));

  read(perf_fd, &end_count, sizeof(long long));
  printf("Start count: %lld\n", start_count);
  printf("End count: %lld\n", end_count);
  printf("Delta: %lld\n", end_count - start_count);

  perf_deinit();
  return 0;
}

#endif // __PERF_H__
