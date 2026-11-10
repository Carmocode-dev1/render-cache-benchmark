#include "fixed_lookup_cache.hpp"

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace {

// Simulates an uncached per-frame validation cost (e.g., repeated OS/table verification).
std::uintptr_t slowValidateHandle(std::size_t id) noexcept {
    std::uint64_t x = static_cast<std::uint64_t>(id) * 6364136223846793005ULL + 1442695040888963407ULL;
    for (int i = 0; i < 40; ++i) {
        x ^= (x >> 13);
        x *= 0xff51afd7ed558ccdULL;
        x ^= (x >> 17);
    }
    return static_cast<std::uintptr_t>((x & 0xFFFFULL) | 0x1000ULL);
}

}  // namespace

int main() {
    constexpr std::size_t kTableSize = 3072;
    constexpr std::size_t kFrames = 1500;
    constexpr std::size_t kVisibleObjects = 512;

    perf::FixedLookupCache<kTableSize> cache;

    volatile std::uintptr_t sinkUncached = 0;
    const auto t0 = std::chrono::steady_clock::now();
    for (std::size_t f = 0; f < kFrames; ++f) {
        for (std::size_t i = 0; i < kVisibleObjects; ++i) {
            sinkUncached ^= slowValidateHandle((i * 7) % 2048);
        }
    }
    const auto t1 = std::chrono::steady_clock::now();

    volatile std::uintptr_t sinkCached = 0;
    const auto t2 = std::chrono::steady_clock::now();
    for (std::size_t f = 0; f < kFrames; ++f) {
        for (std::size_t i = 0; i < kVisibleObjects; ++i) {
            const std::size_t id = (i * 7) % 2048;
            std::uintptr_t handle = 0;
            const auto state = cache.lookup(id, handle);
            if (state == perf::EntryState::Unchecked) {
                handle = slowValidateHandle(id);
                cache.storeValid(id, handle);
            }
            if (state != perf::EntryState::Invalid) {
                sinkCached ^= handle;
            }
        }
    }
    const auto t3 = std::chrono::steady_clock::now();

    const double uncachedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    const double cachedMs = std::chrono::duration<double, std::milli>(t3 - t2).count();
    const double reductionPct = uncachedMs > 0.0 ? ((uncachedMs - cachedMs) / uncachedMs) * 100.0 : 0.0;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "Lookups:      " << (kFrames * kVisibleObjects) << "\n";
    std::cout << "Uncached:     " << uncachedMs << " ms\n";
    std::cout << "Fixed Cache:  " << cachedMs << " ms\n";
    std::cout << "CPU savings:  " << reductionPct << " %\n";
    std::cout << "Checksums:    " << sinkUncached << " vs " << sinkCached << "\n";

    return (sinkUncached == sinkCached) ? 0 : 1;
}
