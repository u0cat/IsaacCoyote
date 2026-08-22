//
// Created by TsCat on 2026/7/13.
//

#ifndef ISAACSPY_WATCH_MANAGER_H
#define ISAACSPY_WATCH_MANAGER_H

#include <windows.h>
#include <cstdint>
#include <functional>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace isaac_spy::mem::watch
{
    enum class WatchAccess {
        Read = 1,
        Write = 2,
        Execute = 4
    };

    enum class TriggerCondition {
        Always,
        OnChanged,
        OnEqualTo,
        OnNotEqualTo
    };

    using WatchCallback = std::function<void(uintptr_t address, WatchAccess access,
                                             const void* oldValue, const void* newValue,
                                             size_t valueSize, void* context)>;

    struct PageGuard {
        DWORD originalProtect = 0;
        DWORD watchProtect = 0;
        int refCount = 0;
    };

    struct WatchEntry {
        uint64_t id = 0;
        uintptr_t baseAddress = 0;
        size_t size = 0;
        WatchAccess accessType = WatchAccess::Write;

        TriggerCondition condition = TriggerCondition::Always;

        std::vector<uint8_t> expectedValueBytes;
        WatchCallback callback;
        void* context = nullptr;

        std::vector<uintptr_t> coveredPages;
    };

    class WatchManager {
    public:
        static WatchManager& getInstance() {
            static WatchManager instance;
            return instance;
        }

        bool AddWatch(uintptr_t address, size_t size, WatchAccess access,
                       TriggerCondition condition,
                       const void* expectedValue, size_t expectedValueSize,
                       WatchCallback callback, void* context = nullptr);
        bool AddOnChangeWatch(uintptr_t address, size_t size,
                              WatchCallback callback, void* context = nullptr);
        bool RemoveWatch(uintptr_t address, void* context = nullptr);

    private:
        WatchManager();
        ~WatchManager();

        static LONG CALLBACK VectoredHandler(PEXCEPTION_POINTERS exception_info);

        bool AddPageRef(uintptr_t pageAddr, DWORD currentProtect);
        void ReleasePageRef(uintptr_t pageAddr);

        std::vector<WatchEntry> m_watches;
        std::unordered_map<uintptr_t, PageGuard> m_pageGuards;
        mutable std::shared_mutex m_mutex;
        PVOID m_vehHandle = nullptr;
        DWORD m_pageSize = 0;
        uint64_t m_nextWatchId = 1;
    };
}
#endif //ISAACSPY_WATCH_MANAGER_H
