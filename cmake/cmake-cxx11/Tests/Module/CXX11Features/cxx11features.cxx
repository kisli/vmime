#if defined(HAS_CXX0X_CSTDINT_H)
#include <cstdint>
#endif

#include <sys/types.h>

struct thing {
    unsigned char one;
#if defined(HAS_CXX0X_CSTDINT_H)
    uint32_t four;
#endif
#if defined(HAS_CXX0X_LONG_LONG)
    long long eight;
#endif
};

#include <stdio.h>

int main()
{
#if defined (HAS_CXX0X_NULLPTR)
    void *nix = nullptr;
#else /* HAS_CXX0X_NULLPTR */
    void *nix = 0;
#endif /* HAS_CXX0X_NULLPTR */

#if defined(HAS_CXX0X_STATIC_ASSERT)
    static_assert(1 < 42, "Your C++ compiler is b0rked");
#endif /* HAS_CXX0X_STATIC_ASSERT */

#if defined(HAS_CXX0X_FUNC)
    const char *funcname = __func__;
    printf("the name of main() function is: %s\n", funcname);
#endif /* HAS_CXX0X_FUNC */

#if defined(HAS_CXX0X_SIZEOF_MEMBER)
    size_t onesize = sizeof(thing::one);
#if defined(HAS_CXX0X_STATIC_ASSERT)
    static_assert(sizeof(thing::one) == 1, "Your char is not one byte long");
#endif /* HAS_CXX0X_STATIC_ASSERT */

#if defined(HAS_CXX0X_CSTDINT_H)
    size_t foursize = sizeof(thing::four);
#if defined(HAS_CXX0X_STATIC_ASSERT)
    static_assert(sizeof(thing::four) == 4, "Your uint32_t is not 32 bit long");
#endif /* HAS_CXX0X_STATIC_ASSERT */
#endif /* HAS_CXX0X_CSTDINT_H */
#if defined(HAS_CXX0X_LONG_LONG)
    size_t eightsize = sizeof(thing::eight);
#if defined(HAS_CXX0X_STATIC_ASSERT)
    static_assert(sizeof(thing::eight) == 8, "Your long long is not 64 bit long");
#endif /* HAS_CXX0X_STATIC_ASSERT */
#endif /* HAS_CXX0X_LONG_LONG */
#endif /* HAS_CXX0X_SIZEOF_MEMBER */

    return 0;
}
