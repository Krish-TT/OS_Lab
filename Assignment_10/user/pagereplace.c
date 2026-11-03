#include "kernel/types.h"
#include "user/user.h"

#define REFERENCE_COUNT 20
#define MAX_FRAMES 5

static int references[REFERENCE_COUNT] = {
  7, 0, 1, 2, 0, 3, 0, 4, 2, 3,
  0, 3, 2, 1, 2, 0, 1, 7, 0, 1
};

static void
print_frames(int *frames, int frame_count)
{
  printf("[");
  for (int i = 0; i < frame_count; i++)
    printf("%d%s", frames[i], i + 1 == frame_count ? "" : " ");
  printf("]");
}

static int
find_page(int *frames, int frame_count, int page)
{
  for (int i = 0; i < frame_count; i++)
    if (frames[i] == page)
      return i;
  return -1;
}

static int
fifo(int frame_count, int verbose)
{
  int frames[MAX_FRAMES];
  int next = 0;
  int faults = 0;

  for (int i = 0; i < frame_count; i++)
    frames[i] = -1;
  for (int i = 0; i < REFERENCE_COUNT; i++) {
    int page = references[i];
    int hit = find_page(frames, frame_count, page) >= 0;
    if (!hit) {
      frames[next] = page;
      next = (next + 1) % frame_count;
      faults++;
    }
    if (verbose) {
      printf("FIFO ref %d: ", page);
      print_frames(frames, frame_count);
      printf(" %s\n", hit ? "hit" : "fault");
    }
  }
  return faults;
}

static int
lru(int frame_count, int verbose)
{
  int frames[MAX_FRAMES];
  int age[MAX_FRAMES];
  int faults = 0;

  for (int i = 0; i < frame_count; i++) {
    frames[i] = -1;
    age[i] = 0;
  }
  for (int i = 0; i < REFERENCE_COUNT; i++) {
    int page = references[i];
    int slot = find_page(frames, frame_count, page);
    int hit = slot >= 0;
    if (!hit) {
      slot = 0;
      for (int j = 1; j < frame_count; j++)
        if (frames[j] == -1 || age[j] < age[slot])
          slot = j;
      frames[slot] = page;
      faults++;
    }
    age[slot] = i + 1;
    if (verbose) {
      printf("LRU ref %d: ", page);
      print_frames(frames, frame_count);
      printf(" %s\n", hit ? "hit" : "fault");
    }
  }
  return faults;
}

static int
optimal(int frame_count, int verbose)
{
  int frames[MAX_FRAMES];
  int faults = 0;

  for (int i = 0; i < frame_count; i++)
    frames[i] = -1;
  for (int i = 0; i < REFERENCE_COUNT; i++) {
    int page = references[i];
    int slot = find_page(frames, frame_count, page);
    int hit = slot >= 0;
    if (!hit) {
      int farthest = -1;
      slot = 0;
      for (int j = 0; j < frame_count; j++) {
        int next = REFERENCE_COUNT + 1;
        if (frames[j] == -1) {
          slot = j;
          break;
        }
        for (int k = i + 1; k < REFERENCE_COUNT; k++)
          if (references[k] == frames[j]) {
            next = k;
            break;
          }
        if (next > farthest) {
          farthest = next;
          slot = j;
        }
      }
      frames[slot] = page;
      faults++;
    }
    if (verbose) {
      printf("OPT ref %d: ", page);
      print_frames(frames, frame_count);
      printf(" %s\n", hit ? "hit" : "fault");
    }
  }
  return faults;
}

static void
run_algorithm(const char *name, int (*algorithm)(int, int), int frame_count)
{
  int faults = algorithm(frame_count, frame_count == 3);
  int hits = REFERENCE_COUNT - faults;

  printf("%s frames=%d faults=%d hits=%d hit-ratio=%d/%d fault-ratio=%d/%d\n",
      name, frame_count, faults, hits, hits, REFERENCE_COUNT, faults,
      REFERENCE_COUNT);
}

int
main(void)
{
  printf("Reference string: 7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1\n");
  run_algorithm("FIFO", fifo, 3);
  run_algorithm("LRU", lru, 3);
  run_algorithm("OPT", optimal, 3);
  printf("Comparison:\n");
  for (int frames = 3; frames <= 5; frames++) {
    run_algorithm("FIFO", fifo, frames);
    run_algorithm("LRU", lru, frames);
    run_algorithm("OPT", optimal, frames);
  }
  exit(0);
}
