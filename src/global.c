#include "global.h"
#include "os_metrics.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

FILE *verboseChannel = 0;
FILE *perfChannel = 0;
ProfilerTimer profileTimers[MAX_PROFILE_TIMERS] = {0};
int currentProfileTimer = 0;

uint64_t cpuFreq = 0;

/**
 * OK so yeah OK so.
 *
 * when we enter a function
 * we set the global "current function"
 * and when we exit, we restore the "current function"
 * and subtract the time from the elapsedNoChildren
 *
 */

unsigned long hash(const char *str1, const char *str2) {
  unsigned long hash = 5381;
  int c;

  while ((c = (unsigned char)*str1++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  while ((c = (unsigned char)*str2++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash & 0xFFF;
}

ProfilerTimer *currentTimer = &profileTimers[0];

unsigned long start(const char *name, uint64_t bytesProcessed) {

  unsigned long parentHash = currentTimer->h;
  unsigned long h = hash(name, "1");

  ProfilerTimer *timer = &profileTimers[h];
  ProfilerTimer *parentTimer = &profileTimers[parentHash];

  // stop the parent timer
  parentTimer->exclusive += ReadCPUTimer() - parentTimer->begin;
  parentTimer->active--;

  // init this timer if we need to
  if (timer->active == 0) {
    strcpy(timer->label, name);
    timer->h = h;
    timer->initTime = ReadCPUTimer();
  }

  // start this timer
  timer->processedBytesCount += bytesProcessed;
  timer->active++;
  timer->hits++;
  timer->begin = ReadCPUTimer();

  currentTimer = timer;

  // pack the two hashes together
  return (parentHash << 12) | h;
}

void stop(unsigned long *hashes) {
  unsigned long parentHash = *hashes >> 12;
  unsigned long h = *hashes & 0xFFF;

  ProfilerTimer *timer = &profileTimers[h];
  ProfilerTimer *parentTimer = &profileTimers[parentHash];

  if (timer->label[0] == 0) {
    fprintf(perfChannel, "tried to update a timer that did not exist\n");
    return;
  }

  // stop this timer
  timer->active--;
  timer->exclusive += ReadCPUTimer() - timer->begin;

  if (timer->active == 0) {
    // record total
    timer->total += ReadCPUTimer() - timer->initTime;
  }

  // start the parent
  parentTimer->active++;
  parentTimer->begin = ReadCPUTimer();

  currentTimer = parentTimer;
}

uint64_t profilerStartTime;

void processArgs(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-v") == 0) {
      verbose = 1;
    }

    if (strcmp(argv[i], "-perf") == 0) {
      perf = 1;
    }

    if (strcmp(argv[i], "-emit") == 0) {
      emit = 1;
    }
  }

  if (verbose) {
    verboseChannel = stderr;
    perfChannel = fopen("/dev/null", "w");
  } else if (perf) {
    perfChannel = stderr;
    verboseChannel = fopen("/dev/null", "w");
  } else {
    verboseChannel = fopen("/dev/null", "w");
    perfChannel = fopen("/dev/null", "w");
  }
}

void beginProfiler() {
  profilerStartTime = ReadCPUTimer();

  if (!perf) {
    return;
  }

  ProfilerTimer *timer = &profileTimers[0];

  strcpy(timer->label, "main");
  timer->active++;
  timer->hits++;
  timer->h = 0;
  timer->begin = ReadCPUTimer();
  timer->initTime = ReadCPUTimer();

  cpuFreq = EstimateCPUTimerFreq();
  currentTimer = timer;
}

void endAndPrintProfiler() {
  ProfilerTimer *timer = &profileTimers[0];

  // stop the root timer
  timer->exclusive += ReadCPUTimer() - timer->begin;
  timer->active--;
  timer->total = ReadCPUTimer() - timer->initTime;

  uint64_t totalElapsed = ReadCPUTimer() - profilerStartTime;

  if (cpuFreq) {
    fprintf(perfChannel, "\nTotal time: %0.4fms (CPU freq %llu)\n\n",
            1000.0 * (double)totalElapsed / (double)cpuFreq, cpuFreq);
  }

#ifdef PROFILER

  int count = 0;
  for (int i = 0; i < MAX_PROFILE_TIMERS; i++) {
    ProfilerTimer timer = profileTimers[i];
    int printed = 0;

    if (timer.active > 0) {
      fprintf(perfChannel, "forgot to end timer: %s\n", timer.label);
    }

    if (timer.exclusive > 0) {
      printed = 1;
      fprintf(perfChannel, "  %d. ", count++);
      PrintTimeElapsed(timer.label, totalElapsed, timer);
    }

    if (timer.processedBytesCount > 0) {
      printed = 1;
      double megabyte = 1024 * 1024;
      double gigabyte = megabyte * 1024;

      double seconds = (double)timer.total / (double)cpuFreq;
      double bytesPerSecond =
          (double)timer.processedBytesCount / (double)seconds;
      double megabytes = (double)timer.processedBytesCount / (double)megabyte;
      double gigabytesPerSecond = bytesPerSecond / (double)gigabyte;

      fprintf(perfChannel, "  %.3fmb at %.2fgb/s", megabytes,
              gigabytesPerSecond);
    }

    if (printed) {
      fprintf(perfChannel, "\n");
    }
  }
#endif
}

char *read_file_no_alloc(const char *filename, char *buffer) {
  FILE *f = fopen(filename, "rb");
  if (!f) {
    perror("fopen");
    return NULL;
  }

  // Seek to end to get file size
  if (fseek(f, 0, SEEK_END) != 0) {
    perror("fseek");
    fclose(f);
    return NULL;
  }

  long size = ftell(f);
  if (size < 0) {
    perror("ftell");
    fclose(f);
    return NULL;
  }

  rewind(f); // Go back to start

  if (!buffer) {
    perror("malloc");
    fclose(f);
    return NULL;
  }

  // Read file into buffer
  size_t read = fread(buffer, 1, size, f);
  if (read != (size_t)size) {
    perror("fread");
    fclose(f);
    return NULL;
  }

  buffer[size] = '\0'; // Optional null terminator

  fclose(f);
  return buffer;
}

char *read_file(const char *filename) {
  FILE *f = fopen(filename, "rb");
  if (!f) {
    perror("fopen");
    return NULL;
  }

  // Seek to end to get file size
  if (fseek(f, 0, SEEK_END) != 0) {
    perror("fseek");
    fclose(f);
    return NULL;
  }

  long size = ftell(f);
  if (size < 0) {
    perror("ftell");
    fclose(f);
    return NULL;
  }

  rewind(f); // Go back to start

  // Allocate buffer (+1 if you want a null terminator)
  char *buffer = malloc(size + 1);
  if (!buffer) {
    perror("malloc");
    fclose(f);
    return NULL;
  }

  // Read file into buffer
  size_t read = fread(buffer, 1, size, f);
  if (read != (size_t)size) {
    perror("fread");
    free(buffer);
    fclose(f);
    return NULL;
  }

  buffer[size] = '\0'; // Optional null terminator

  fclose(f);
  return buffer;
}
