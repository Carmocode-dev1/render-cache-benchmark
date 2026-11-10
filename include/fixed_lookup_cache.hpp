#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace perf {

enum class EntryState : std::uint8_t {
    Unchecked = 0,
    Valid = 1,
    Invalid = 2
};

template <std::size_t MaxEntries>
class FixedLookupCache {
    static_assert(MaxEntries > 0, "MaxEntries must be greater than zero");

public:
    struct Slot {
        std::uint32_t epoch = 0;
        EntryState state = EntryState::Unchecked;
        std::uintptr_t resourceHandle = 0;
    };

    FixedLookupCache() = default;

    EntryState lookup(std::size_t index, std::uintptr_t& outHandle) const noexcept {
        if (index >= MaxEntries) {
            outHandle = 0;
            return EntryState::Invalid;
        }
        const Slot& slot = slots_[index];
        if (slot.epoch != currentEpoch_ || slot.state == EntryState::Unchecked) {
            outHandle = 0;
            return EntryState::Unchecked;
        }
        outHandle = slot.resourceHandle;
        return slot.state;
    }

    bool storeValid(std::size_t index, std::uintptr_t handle) noexcept {
        if (index >= MaxEntries || handle == 0) {
            return false;
        }
        slots_[index] = Slot{currentEpoch_, EntryState::Valid, handle};
        return true;
    }

    bool storeInvalid(std::size_t index) noexcept {
        if (index >= MaxEntries) {
            return false;
        }
        slots_[index] = Slot{currentEpoch_, EntryState::Invalid, 0};
        return true;
    }

    void invalidateEntry(std::size_t index) noexcept {
        if (index < MaxEntries) {
            slots_[index] = Slot{0, EntryState::Unchecked, 0};
        }
    }

    void invalidateAll() noexcept {
        ++currentEpoch_;
        if (currentEpoch_ == 0) {
            // Handle 32-bit epoch wraparound safely once every 4 billion full resets.
            for (auto& slot : slots_) {
                slot = Slot{0, EntryState::Unchecked, 0};
            }
            currentEpoch_ = 1;
        }
    }

    constexpr std::size_t capacity() const noexcept { return MaxEntries; }
    std::uint32_t epoch() const noexcept { return currentEpoch_; }

private:
    std::array<Slot, MaxEntries> slots_{};
    std::uint32_t currentEpoch_ = 1;
};

}  // namespace perf
