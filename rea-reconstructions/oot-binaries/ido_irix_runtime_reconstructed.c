/*
 * Reconstructed C interface & core logic for SGI IRIX IDO 5.3 / 7.1 Runtime Libraries
 * Evidence source:
 *   - ido53_libmalloc__inspect-native-exports.json (28 exports, MIPS32 big-endian ELF, sha256: cd6cce9583a6...)
 *   - ido71_libmalloc__inspect-native-exports.json (28 exports, identical sha256: cd6cce9583a6...)
 *   - ido53_libexc__inspect-native-exports.json    (25 exports, sha256: b59bf75bcbff...)
 *   - ido71_libexc__inspect-native-exports.json    (28 exports: adds __exc_set_gp_and_resume, unwind_name, find_rpd_name)
 *   - ido53_libgen__inspect-native-exports.json    (95 exports, sha256: bb66dea12420...)
 * Note: Reconstructed from ELF export tables, initializers, and MIPS ABI conventions (no Ghidra decompiler attached).
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

/* =========================================================================
 * 1. libmalloc.so (IDO 5.3 & 7.1 identical binary)
 * Exports: malloc, free, realloc, calloc, recalloc, cfree, memalign,
 *          mallocblksize, mallopt, mallinfo, __checkq, plus _* weak aliases
 * ========================================================================= */

#define M_MXFAST 1
#define M_NLBLKS 2
#define M_GRAIN  3
#define M_KEEP   4

struct mallinfo {
    int arena;
    int ordblks;
    int smblks;
    int hblks;
    int hblkhd;
    int usmblks;
    int fsmblks;
    int uordblks;
    int fordblks;
    int keepcost;
};

typedef struct malloc_hdr {
    size_t size;           /* bit 0: in-use flag, bit 1: small-block flag */
    struct malloc_hdr* next;
    struct malloc_hdr* prev;
} malloc_hdr_t;

static malloc_hdr_t* g_free_list_head = NULL;
static struct mallinfo g_mallinfo_state = {0};
static int g_mxfast = 0;
static int g_nlblks = 100;
static int g_grain  = 4;

int __checkq(void) {
    malloc_hdr_t* cur = g_free_list_head;
    while (cur != NULL) {
        if (cur->next && cur->next->prev != cur) {
            return -1;
        }
        cur = cur->next;
    }
    return 0;
}

size_t mallocblksize(void* ptr) {
    if (!ptr) return 0;
    malloc_hdr_t* hdr = ((malloc_hdr_t*)ptr) - 1;
    return hdr->size & ~(size_t)0x7;
}

int mallopt(int cmd, int value) {
    switch (cmd) {
        case M_MXFAST: g_mxfast = value; return 0;
        case M_NLBLKS: g_nlblks = value; return 0;
        case M_GRAIN:  g_grain  = value; return 0;
        case M_KEEP:   return 0;
        default:       return -1;
    }
}

struct mallinfo mallinfo(void) {
    return g_mallinfo_state;
}

void* _malloc_impl(size_t size);
void  _free_impl(void* ptr);

void* recalloc(void* ptr, size_t nelem, size_t elsize) {
    size_t old_sz = mallocblksize(ptr);
    size_t new_sz = nelem * elsize;
    void* next = realloc(ptr, new_sz);
    if (next && new_sz > old_sz) {
        memset((uint8_t*)next + old_sz, 0, new_sz - old_sz);
    }
    return next;
}

void* memalign(size_t alignment, size_t size) {
    if (alignment < sizeof(void*) || (alignment & (alignment - 1)) != 0) {
        errno = EINVAL;
        return NULL;
    }
    uintptr_t raw = (uintptr_t)_malloc_impl(size + alignment + sizeof(void*));
    if (!raw) return NULL;
    uintptr_t aligned = (raw + sizeof(void*) + alignment - 1) & ~(uintptr_t)(alignment - 1);
    ((void**)aligned)[-1] = (void*)raw;
    return (void*)aligned;
}

void cfree(void* ptr) {
    _free_impl(ptr);
}

/* =========================================================================
 * 2. libexc.so (MIPS IRIX Exception & Stack Unwind Runtime)
 * Exports: exc_setjmp, exc_resume, __exc_set_gp_and_resume (7.1),
 *          find_rpd, find_rpd_name (7.1), unwind, unwind_name (7.1),
 *          fixup_rpt, compare_rpdrs, exception_dispatcher, findinit
 * ========================================================================= */

typedef struct runtime_pdr {
    uint32_t adr;
    int32_t  regmask;
    int32_t  regoffset;
    int32_t  fregmask;
    int32_t  fregoffset;
    int32_t  frameoffset;
    int16_t  framereg;
    int16_t  pcreg;
    int32_t  lnLow;
    int32_t  lnHigh;
    int32_t  cbLineOffset;
} runtime_pdr_t;

int compare_rpdrs(const void* a, const void* b) {
    const runtime_pdr_t* ra = (const runtime_pdr_t*)a;
    const runtime_pdr_t* rb = (const runtime_pdr_t*)b;
    if (ra->adr < rb->adr) return -1;
    if (ra->adr > rb->adr) return 1;
    return 0;
}

const runtime_pdr_t* find_rpd(const runtime_pdr_t* table, size_t count, uint32_t pc) {
    size_t lo = 0, hi = count;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        if (table[mid].adr <= pc) {
            if (mid + 1 == count || table[mid + 1].adr > pc) {
                return &table[mid];
            }
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return NULL;
}

/* =========================================================================
 * 3. libgen.so (Path & Utility Helpers)
 * Exports: basename, dirname, mkdirp, rmdirp, pathfind, bgets, bufsplit
 * ========================================================================= */

char* basename_irix(char* path) {
    if (!path || !*path) return ".";
    size_t len = strlen(path);
    while (len > 1 && path[len - 1] == '/') {
        path[--len] = '\0';
    }
    char* slash = strrchr(path, '/');
    return slash ? (slash[1] ? slash + 1 : "/") : path;
}

char* dirname_irix(char* path) {
    if (!path || !*path) return ".";
    size_t len = strlen(path);
    while (len > 1 && path[len - 1] == '/') {
        path[--len] = '\0';
    }
    char* slash = strrchr(path, '/');
    if (!slash) return ".";
    if (slash == path) {
        path[1] = '\0';
        return path;
    }
    *slash = '\0';
    return path;
}
