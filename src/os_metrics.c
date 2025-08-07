#include "os_metrics.h"
#include "global.h"
#include <mach/mach_time.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/time.h>

uint64_t GetOSTimerFreq(void) { return 1000000; }

uint64_t ReadOSTimer(void) {
  // NOTE(casey): The "struct" keyword is not necessary here when compiling in
  // C++, but just in case anyone is using this file from C, I include it.
  struct timeval Value;
  gettimeofday(&Value, 0);

  uint64_t Result =
      GetOSTimerFreq() * (uint64_t)Value.tv_sec + (uint64_t)Value.tv_usec;
  return Result;
}

/* NOTE(casey): This does not need to be "inline", it could just be "static"
   because compilers will inline it anyway. But compilers will warn about
   static functions that aren't used. So "inline" is just the simplest way
   to tell them to stop complaining about that. */
inline uint64_t ReadCPUTimer(void) {
  // NOTE(casey): If you were on ARM, you would need to replace __rdtsc
  // with one of their performance counter read instructions, depending
  // on which ones are available on your platform.
  return clock_gettime_nsec_np(CLOCK_UPTIME_RAW);
}

void PrintTimeElapsed(char const *Label, uint64_t TotalTSCElapsed,
                      ProfilerTimer timer) {
  uint64_t Elapsed = timer.total;
  uint64_t ElapsedNoChildren = timer.exclusive;

  double PercentWithChildren =
      100.0 * ((double)Elapsed / (double)TotalTSCElapsed);
  double Percent =
      100.0 * ((double)ElapsedNoChildren / (double)TotalTSCElapsed);

  fprintf(perfChannel, "%-15s[%d] %-16llu (%6.2f%%) (w/ children: %6.2f%%)",
          Label, timer.hits, Elapsed, Percent, PercentWithChildren);
}

uint64_t ReadOSPageFaultCount(void) {
  // NOTE(casey): The course materials are not tested on MacOS/Linux.
  // This code was contributed to the public github. It may or may not work
  // for your system.

  struct rusage Usage = {};
  getrusage(RUSAGE_SELF, &Usage);

  // ru_minflt  the number of page faults serviced without any I/O activity.
  // ru_majflt  the number of page faults serviced that required I/O activity.
  uint64_t Result = Usage.ru_minflt + Usage.ru_majflt;

  return Result;
}

uint64_t EstimateCPUTimerFreq(void) {
  uint64_t MillisecondsToWait = 100;
  uint64_t OSFreq = GetOSTimerFreq();

  uint64_t CPUStart = ReadCPUTimer();
  uint64_t OSStart = ReadOSTimer();
  uint64_t OSEnd = 0;
  uint64_t OSElapsed = 0;
  uint64_t OSWaitTime = OSFreq * MillisecondsToWait / 1000;
  while (OSElapsed < OSWaitTime) {
    OSEnd = ReadOSTimer();
    OSElapsed = OSEnd - OSStart;
  }

  uint64_t CPUEnd = ReadCPUTimer();
  uint64_t CPUElapsed = CPUEnd - CPUStart;

  uint64_t CPUFreq = 0;
  if (OSElapsed) {
    CPUFreq = OSFreq * CPUElapsed / OSElapsed;
  }

  fprintf(verboseChannel, "OS Timer: %llu -> %llu = %llu elapsed\n", OSStart,
          OSEnd, OSElapsed);
  fprintf(verboseChannel, "OS Seconds: %.4f\n",
          (double)OSElapsed / (double)OSFreq);

  fprintf(verboseChannel, "CPU Timer: %llu -> %llu = %llu elapsed\n", CPUStart,
          CPUEnd, CPUElapsed);
  fprintf(verboseChannel, "CPU Freq: %llu (guessed)\n", CPUFreq);

  return CPUFreq;
}
