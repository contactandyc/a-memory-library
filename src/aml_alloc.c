// SPDX-FileCopyrightText: 2019–2025 Andy Curtis <contactandyc@gmail.com>
// SPDX-FileCopyrightText: 2024–2025 Knode.ai — technical questions: contact Andy (above)
// SPDX-License-Identifier: Apache-2.0

#include "a-memory-library/aml_alloc.h"
#include <pthread.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

struct aml_allocator_s;
typedef struct aml_allocator_s aml_allocator_t;

void aml_allocator_init();
void aml_allocator_destroy();
void aml_dump_global_allocations(aml_allocator_t *a, FILE *out);

typedef struct aml_allocator_node_s {
  const char *caller;
  ssize_t length;
  struct aml_allocator_node_s *next;
  struct aml_allocator_node_s *previous;
  aml_allocator_t *a;
} aml_allocator_node_t;

struct aml_allocator_s {
  aml_allocator_node_t *head;
  aml_allocator_node_t *tail;
  size_t total_bytes_allocated;
  size_t total_allocations;
  char *logfile;
  pthread_t thread;
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  int done;
};

static void print_node(FILE *out, const char *caller, ssize_t len,
                       aml_allocator_node_t *n) {
  if (len >= 0)
    fprintf(out, "%s: %ld ", caller, len);
  else {
    aml_allocator_dump_t *d = (aml_allocator_dump_t *)(n + 1);
    ssize_t num = -len;
    size_t length = num;
    d->dump(out, caller, d, length);
  }
}

aml_allocator_t *global_allocator = NULL;

static void _aml_dump_global_allocations(aml_allocator_t *a, FILE *out) {
  if (a->head) {
    fprintf(out,
            "%lu byte(s) allocated in %lu allocations (%lu byte(s) overhead)\n",
            a->total_bytes_allocated, a->total_allocations,
            a->total_allocations * sizeof(aml_allocator_node_t));
    aml_allocator_node_t *n = a->head;
    while (n) {
      print_node(out, n->caller, n->length, n);
      fprintf(out, "\n");
      n = n->next;
    }
  }
}

void aml_dump_global_allocations(aml_allocator_t *a, FILE *out) {
  pthread_mutex_lock(&a->mutex);
  _aml_dump_global_allocations(a, out);
  pthread_mutex_unlock(&a->mutex);
}

void _aml_dump(FILE *out) {
    aml_dump_global_allocations(global_allocator, out);
}

void save_old_log(aml_allocator_t *a, size_t saves, char *tmp) {
  int num = 0;
  int s = saves;
  for (int i = 32; i > 0; i--) {
    uint32_t ix = i - 1;
    uint32_t v = 1 << ix;
    v--;
    if ((saves & v) == v) {
      num = ix;
      break;
    }
  }
  // printf("%lu => %d\n", saves, num);
  while (num) {
    num--;
    char *p = tmp;
    char *old_name = p;
    if (num) {
      snprintf(old_name, strlen(a->logfile)+20, "%s.%d", a->logfile, num);
      p += strlen(old_name) + 1;
    } else
      old_name = (char *)a->logfile;

    char *new_name = p;
    snprintf(new_name, strlen(a->logfile)+20, "%s.%d", a->logfile, num + 1);
    // printf("rename: %s => %s\n", old_name, new_name);
    rename(old_name, new_name);
  }
}

void *dump_global_allocations_thread(void *arg) {
  aml_allocator_t *a = (aml_allocator_t *)arg;
  // save every 60 seconds, back off now, 60 seconds ago, 240 seconds ago, 240*4
  // seconds ago
  size_t save = 0;
  struct timespec ts;

  char *tmp = (char *)malloc((strlen(a->logfile) * 2) * 100);
  int done = 0;
  while (!done) {
    save_old_log(a, save, tmp);
    pthread_mutex_lock(&a->mutex);
    time_t t = time(NULL);
    FILE *out = fopen(a->logfile, "wb");
    fprintf(out, "%s", ctime(&t));
    _aml_dump_global_allocations(a, out);
    fclose(out);
    save++;
    if (a->done)
      done = 1;
    else {
      clock_gettime(CLOCK_REALTIME, &ts);
      ts.tv_sec += 15;
      pthread_cond_timedwait(&a->cond, &a->mutex, &ts);
    }
    pthread_mutex_unlock(&a->mutex);
  }
  free(tmp);
  return NULL;
}

void aml_allocator_init() {
  if(global_allocator)
    return;

  aml_allocator_t *a = (aml_allocator_t *)malloc(sizeof(aml_allocator_t));
  a->head = NULL;
  a->tail = NULL;
  a->total_bytes_allocated = 0;
  a->total_allocations = 0;
  a->logfile = NULL;
  a->done = 0;
  pthread_mutex_init(&a->mutex, NULL);
  global_allocator = a;
}

void _aml_alloc_log(const char *filename) {
  aml_allocator_t *a = global_allocator;
  if (a->logfile) { free(a->logfile); a->logfile = NULL; }
  if (!filename) return;
  a->logfile = strdup(filename);
  if (filename) {
    pthread_cond_init(&a->cond, NULL);
    pthread_create(&(a->thread), NULL, dump_global_allocations_thread, a);
  }
}

void aml_allocator_destroy() {
  aml_allocator_t *a = global_allocator;
  if(!a) return;
  if (a->logfile) {
    /* broadcast that we are done to the main thread */
    pthread_mutex_lock(&a->mutex);
    a->done = 1;
    pthread_cond_signal(&a->cond);
    pthread_mutex_unlock(&a->mutex);
    pthread_join(a->thread, NULL);
  } else {
    if (a->logfile) {
      FILE *out = fopen(a->logfile, "wb");
      aml_dump_global_allocations(a, stderr);
      fclose(out);
      free(a->logfile);
    } else
      aml_dump_global_allocations(a, stderr);
  }
  free(a);
  global_allocator = NULL;
}

#ifdef _AML_DEBUG_
void amlStartupFun(void) __attribute__((constructor));
void amlCleanupFun(void) __attribute__((destructor));

void amlStartupFun(void) {
  aml_allocator_init();
}

void amlCleanupFun(void) { aml_allocator_destroy(); }
#endif

void *_aml_malloc_d(const char *caller, size_t len, bool custom) {
  if (!len)
    return NULL;

    if(len > 24*1024*1024)
        printf("allocating %lu bytes\n", len);

  aml_allocator_t *a = global_allocator;

  aml_allocator_node_t *n =
      (aml_allocator_node_t *)malloc(sizeof(aml_allocator_node_t) + len);
  if (!n) {
    pthread_mutex_lock(&a->mutex);
    print_node(stderr, caller, len, NULL);
    fprintf(stderr, "malloc failed\n");
    pthread_mutex_unlock(&a->mutex);
    abort();
  }
  ssize_t l = len;
  if (custom)
    l = -l;

  n->caller = caller;
  n->length = l;
  n->next = NULL;
  n->a = a;

  pthread_mutex_lock(&a->mutex);
  a->total_bytes_allocated += len;
  a->total_allocations++;
  n->previous = a->tail;
  if (n->previous)
    n->previous->next = n;
  else
    a->head = n;
  a->tail = n;
  pthread_mutex_unlock(&a->mutex);
  return (void *)(n + 1);
}

void *_aml_calloc_d(const char *caller, size_t len, bool custom) {
  void *m = _aml_malloc_d(caller, len, custom);
  if (m)
    memset(m, 0, len);
  return m;
}

char *_aml_strdup_d(const char *caller, const char *p) {
  size_t len = strlen(p) + 1;
  char *m = (char *)_aml_malloc_d(caller, len, false);
  strcpy(m, p);
  return m;
}

char *_aml_strdupvf(const char *fmt, va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);
  char tmp[32];
  char *tp = (char *)(&tmp);
  int n = vsnprintf(tp, 32, fmt, args_copy);
  if (n < 0)
    abort();
  va_end(args_copy);
  if (n < 32)
    return strdup(tp);

  char *r = (char *)malloc(n + 1);
  va_copy(args_copy, args);
  int n2 = vsnprintf(r, n + 1, fmt, args_copy);
  if (n != n2)
    abort(); // should never happen!
  va_end(args_copy);
  return r;
}

char *_aml_strdupvf_d(const char *caller, const char *fmt, va_list args) {
  va_list args_copy;
  va_copy(args_copy, args);
  char tmp[32];
  char *tp = (char *)(&tmp);
  int n = vsnprintf(tp, 32, fmt, args_copy);
  if (n < 0)
    abort();
  va_end(args_copy);
  if (n < 32)
    return _aml_strdup_d(caller, tp);

  char *r = (char *)_aml_malloc_d(caller, n + 1, false);
  va_copy(args_copy, args);
  int n2 = vsnprintf(r, n + 1, fmt, args_copy);
  if (n != n2)
    abort(); // should never happen!
  va_end(args_copy);
  return r;
}

char *_aml_strdupf_d(const char *caller, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *r = _aml_strdupvf_d(caller, fmt, args);
  va_end(args);
  return r;
}

char *_aml_strdupf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char *r = _aml_strdupvf(fmt, args);
  va_end(args);
  return r;
}

static size_t count_bytes_in_array(char **a, size_t *n) {
  size_t len = sizeof(char *);
  size_t num = 1;
  while (*a) {
    len += strlen(*a) + sizeof(char *) + 1;
    num++;
    a++;
  }
  *n = num;
  return len;
}

static size_t count_bytes_in_arrayn(char **a, size_t num) {
    /* Reserve space for exactly num pointers + one final NULL */
    size_t len = sizeof(char *) * (num + 1);
    for (size_t i = 0; i < num; i++) {
        if (a[i]) {
            len += strlen(a[i]) + 1; /* bytes for this string including '\0' */
        }
    }
    return len;
}

char **_aml_strdupa2_d(const char *caller, char **a) {
  if (!a)
    return NULL;

  char **p = a;
  while (*p)
    p++;

  p++;
  return (char **)_aml_dup_d(caller, a, (p - a) * sizeof(char *));
}

char **_aml_strdupa2(char **a) {
  if (!a)
    return NULL;

  char **p = a;
  while (*p)
    p++;

  p++;
  return (char **)_aml_dup(a, (p - a) * sizeof(char *));
}

char **_aml_strdupa_d(const char *caller, char **a) {
  if (!a)
    return NULL;

  size_t n = 0;
  size_t len = count_bytes_in_array(a, &n);
  char **r = (char **)_aml_malloc_d(caller, len, false);
  char *m = (char *)(r + n);
  char **rp = r;
  while (*a) {
    *rp++ = m;
    char *s = *a;
    while (*s)
      *m++ = *s++;
    *m++ = 0;
    a++;
  }
  *rp = NULL;
  return r;
}


char **_aml_strdupan_d(const char *caller, char **a, size_t n) {
    if (!a)
        return NULL;

    size_t len = count_bytes_in_arrayn(a, n);
    char **r = (char **)_aml_malloc_d(caller, len, false);
    char *m = (char *)(r + n + 1);
    char **rp = r;

    for (size_t i = 0; i < n; i++) {
        if (a[i]) {
            *rp++ = m;
            const char *s = a[i];
            while (*s) *m++ = *s++;
            *m++ = '\0';
        } else {
            *rp++ = NULL;
        }
    }
    *rp = NULL; /* always terminate pointer array */
    return r;
}


char **_aml_strdupa(char **a) {
  if (!a)
    return NULL;

  size_t n = 0;
  size_t len = count_bytes_in_array(a, &n);
  char **r = (char **)malloc(len);
  char *m = (char *)(r + n);
  char **rp = r;
  while (*a) {
    *rp++ = m;
    char *s = *a;
    while (*s)
      *m++ = *s++;
    *m++ = 0;
    a++;
  }
  *rp = NULL;
  return r;
}

char **_aml_strdupan(char **a, size_t n) {
    if (!a)
        return NULL;

    size_t len = count_bytes_in_arrayn(a, n);
    char **r = (char **)malloc(len);
    char *m = (char *)(r + n + 1);
    char **rp = r;

    for (size_t i = 0; i < n; i++) {
        if (a[i]) {
            *rp++ = m;
            const char *s = a[i];
            while (*s) *m++ = *s++;
            *m++ = '\0';
        } else {
            *rp++ = NULL;
        }
    }
    *rp = NULL;
    return r;
}


static aml_allocator_node_t *get_aml_node(const char *caller, void *p, const char *message) {
  aml_allocator_t* a = global_allocator;

  aml_allocator_node_t *n = (aml_allocator_node_t *)p;
  n--;
  if (n->a == a)
    return n;

  pthread_mutex_lock(&a->mutex);
  fprintf(stderr, "Bad pointer passed to %s: %s\n", caller, message);
  pthread_mutex_unlock(&a->mutex);
  abort();

  aml_allocator_node_t *n2 = a->head;
  char *c = (char *)p;
  aml_allocator_node_t *closest = NULL;
  size_t closest_abs_dist;
  ssize_t closest_dist;
  while (n2) {
    char *p2 = (char *)(n2 + 1);
    size_t dist = p2 < c ? c - p2 : p2 - c;
    if (!closest || dist < closest_abs_dist) {
      closest_abs_dist = dist;
      closest_dist = c - p2;
      closest = n2;
    }
    n2 = n2->next;
  }
  pthread_mutex_lock(&a->mutex);
  if (closest) {
    print_node(stderr, closest->caller, closest->length, closest);
    fprintf(
        stderr,
        "is closest allocation and is %lu bytes %s of original allocation\n",
        closest_abs_dist, closest_dist < 0 ? "behind" : "ahead");
  }
  print_node(stderr, caller, 0, NULL);
  fprintf(stderr, "%s\n", message);
  pthread_mutex_unlock(&a->mutex);
  abort();
}

void *_aml_realloc_d(const char *caller, void *p, size_t len, bool custom) {
  if (!p)
    return _aml_malloc_d(caller, len, custom);

  aml_allocator_node_t *n =
      get_aml_node(caller, p, "aml_realloc is invalid (p is not allocated?)");

  void *m = _aml_malloc_d(caller, len, custom);
  size_t len2;
  if (n->length > 0)
    len2 = n->length;
  else {
    ssize_t num = -n->length;
    len2 = num;
  }
  if (len2 < len)
    memcpy(m, p, len2);
  else
    memcpy(m, p, len);

  _aml_free_d(caller, p);
  return m;
}

void _aml_free_d(const char *caller, void *p) {
  if (!p)
    return;

  aml_allocator_t *a = global_allocator;
  aml_allocator_node_t *n =
      get_aml_node(caller, p, "aml_free is invalid (double free?)");
  pthread_mutex_lock(&a->mutex);
  if (n->previous)
    n->previous->next = n->next;
  else
    a->head = n->next;
  if (n->next)
    n->next->previous = n->previous;
  else
    a->tail = n->previous;
  a->total_allocations--;
  if (n->length > 0)
    a->total_bytes_allocated -= n->length;
  else
    a->total_bytes_allocated += n->length;
  pthread_mutex_unlock(&a->mutex);
  n->a--; // to try and protect against double free
  free(n);
}
