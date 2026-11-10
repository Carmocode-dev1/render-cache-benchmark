#include "fixed_lookup_cache.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    perf::FixedLookupCache<1024> cache;
    std::uintptr_t handle = 0;

    // 1. Fresh entry starts unchecked
    assert(cache.lookup(42, handle) == perf::EntryState::Unchecked);
    assert(handle == 0);

    // 2. Store and retrieve valid handle
    assert(cache.storeValid(42, 0xABCD1234));
    assert(cache.lookup(42, handle) == perf::EntryState::Valid);
    assert(handle == 0xABCD1234);

    // 3. Reject out-of-bounds indices and null handles safely
    assert(!cache.storeValid(1024, 0x1000));
    assert(!cache.storeValid(10, 0));
    assert(cache.lookup(2048, handle) == perf::EntryState::Invalid);
    assert(handle == 0);

    // 4. Negative caching for missing/broken descriptors
    assert(cache.storeInvalid(99));
    assert(cache.lookup(99, handle) == perf::EntryState::Invalid);
    assert(handle == 0);

    // 5. Single-entry invalidation
    cache.invalidateEntry(42);
    assert(cache.lookup(42, handle) == perf::EntryState::Unchecked);

    // 6. O(1) epoch invalidation clears all cached entries without loop
    assert(cache.storeValid(7, 0x2000));
    assert(cache.storeInvalid(8));
    cache.invalidateAll();
    assert(cache.lookup(7, handle) == perf::EntryState::Unchecked);
    assert(cache.lookup(8, handle) == perf::EntryState::Unchecked);

    std::cout << "All FixedLookupCache tests passed.\n";
    return 0;
}
