#ifndef UNITY_H
#define UNITY_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*unity_test_func)(void);

typedef struct {
    unsigned total;
    unsigned passed;
    unsigned failed;
    unsigned ignored;
} unity_stats_t;

extern unity_stats_t Unity;

static inline void UnityBegin(void) { Unity.total = Unity.passed = Unity.failed = Unity.ignored = 0; }
static inline int UnityEnd(void) {
    printf("\n--- Unity Summary ---\n");
    printf("Total: %u, Passed: %u, Failed: %u, Ignored: %u\n", Unity.total, Unity.passed, Unity.failed, Unity.ignored);
    return (Unity.failed == 0) ? 0 : 1;
}

static inline void UnityPrint(const char* s) { fputs(s, stdout); }

#define TEST_IGNORE_MESSAGE(msg) do { UnityPrint("[  IGNORED ] "); UnityPrint(msg); UnityPrint("\n"); ++Unity.total; ++Unity.ignored; return; } while(0)
#define TEST_FAIL_MESSAGE(msg) do { UnityPrint("[  FAILED ] "); UnityPrint(msg); UnityPrint("\n"); ++Unity.total; ++Unity.failed; return; } while(0)
#define TEST_PASS() do { ++Unity.total; ++Unity.passed; return; } while(0)

#define TEST_ASSERT_TRUE(cond) do { if(!(cond)) { TEST_FAIL_MESSAGE("Assertion failed: " #cond); } } while(0)
#define TEST_ASSERT_EQUAL_UINT32(exp, act) do { uint32_t _e=(uint32_t)(exp), _a=(uint32_t)(act); if(_e!=_a){ char _b[128]; snprintf(_b,sizeof(_b),"Expected 0x%08X got 0x%08X", (unsigned)_e, (unsigned)_a); TEST_FAIL_MESSAGE(_b);} } while(0)

#ifdef __cplusplus
}
#endif

#endif /* UNITY_H */
