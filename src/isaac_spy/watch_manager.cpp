//
// Created by TsCat on 2026/7/13.
//

#include "isaac_spy/watch_manager.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <mutex>

#include "app/service/log/log_service.h"
#include "isaac_spy/memory.h"

using namespace isaac_spy::mem::watch;

namespace
{
    DWORD GetWriteWatchProtection(DWORD originalProtect) {
        if (originalProtect & PAGE_GUARD) return 0;

        const DWORD modifiers = originalProtect & (PAGE_NOCACHE | PAGE_WRITECOMBINE);
        switch (originalProtect & 0xff) {
        case PAGE_READWRITE:
        case PAGE_WRITECOPY:
            return PAGE_READONLY | modifiers;
        case PAGE_EXECUTE_READWRITE:
        case PAGE_EXECUTE_WRITECOPY:
            return PAGE_EXECUTE_READ | modifiers;
        default:
            return 0;
        }
    }

    struct PendingWatch {
        uint64_t id = 0;
        std::vector<uint8_t> oldValue;
        bool oldValueValid = false;
    };

    struct WritablePage {
        uintptr_t address = 0;
    };

    struct ThreadWatchState {
        std::vector<PendingWatch> watches;
        std::vector<WritablePage> writablePages;
        bool inStep = false;
    };

    thread_local ThreadWatchState tlsState;
}

WatchManager::WatchManager() {
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    m_pageSize = sys_info.dwPageSize;
    m_vehHandle = AddVectoredExceptionHandler(1, VectoredHandler);
    if (!m_vehHandle)
        app::log::get("isaac_spy")->error("failed to register VEH watchpoint handler");
}

WatchManager::~WatchManager() {
    if (m_vehHandle) {
        RemoveVectoredExceptionHandler(m_vehHandle);
        m_vehHandle = nullptr;
    }

    std::unique_lock lock(m_mutex);
    for (auto& watch : m_watches) {
        for (uintptr_t page : watch.coveredPages) {
            ReleasePageRef(page);
        }
    }
    m_watches.clear();
}

bool WatchManager::AddPageRef(uintptr_t pageAddr, DWORD currentProtect) {
    auto it = m_pageGuards.find(pageAddr);
    if (it != m_pageGuards.end()) {
        ++it->second.refCount;
        return true;
    }

    const DWORD watchProtect = GetWriteWatchProtection(currentProtect);
    if (!watchProtect) return false;

    DWORD oldProtect = 0;
    if (!VirtualProtect(reinterpret_cast<LPVOID>(pageAddr), m_pageSize, watchProtect, &oldProtect)) {
        return false;
    }

    m_pageGuards[pageAddr] = {oldProtect, watchProtect, 1};
    return true;
}

void WatchManager::ReleasePageRef(uintptr_t pageAddr) {
    auto it = m_pageGuards.find(pageAddr);
    if (it == m_pageGuards.end()) return;

    --it->second.refCount;
    if (it->second.refCount <= 0) {
        DWORD oldProtect = 0;
        VirtualProtect(reinterpret_cast<LPVOID>(pageAddr), m_pageSize,
                       it->second.originalProtect, &oldProtect);
        m_pageGuards.erase(it);
    }
}

bool WatchManager::AddWatch(uintptr_t address, size_t size, WatchAccess access,
                            TriggerCondition condition,
                            const void* expectedValue, size_t expectedValueSize,
                            WatchCallback callback, void* context) {
    if (!address || !size || access != WatchAccess::Write || !callback ||
        address > (std::numeric_limits<uintptr_t>::max)() - (size - 1)) {
        return false;
    }
    if ((condition == TriggerCondition::OnEqualTo || condition == TriggerCondition::OnNotEqualTo) &&
        (!expectedValue || expectedValueSize != size)) {
        return false;
    }

    const uintptr_t pageMask = ~(static_cast<uintptr_t>(m_pageSize) - 1);
    const uintptr_t pageStart = address & pageMask;
    const uintptr_t pageEnd = (address + size - 1) & pageMask;

    std::unique_lock lock(m_mutex);

    for (auto& watch : m_watches) {
        if (watch.baseAddress == address && watch.size == size &&
            watch.accessType == access && watch.condition == condition &&
            watch.context == context) {
            watch.callback = std::move(callback);
            watch.context = context;
            return true;
        }
    }

    WatchEntry entry;
    entry.id = m_nextWatchId++;
    entry.baseAddress = address;
    entry.size = size;
    entry.accessType = access;
    entry.condition = condition;
    entry.callback = std::move(callback);
    entry.context = context;

    if (condition == TriggerCondition::OnEqualTo || condition == TriggerCondition::OnNotEqualTo) {
        const auto* expectedBytes = static_cast<const uint8_t*>(expectedValue);
        entry.expectedValueBytes.assign(expectedBytes, expectedBytes + size);
    }

    for (uintptr_t page = pageStart;; page += m_pageSize) {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(reinterpret_cast<LPCVOID>(page), &mbi, sizeof(mbi)) ||
            mbi.State != MEM_COMMIT || !AddPageRef(page, mbi.Protect)) {
            for (uintptr_t coveredPage : entry.coveredPages) ReleasePageRef(coveredPage);
            return false;
        }

        entry.coveredPages.push_back(page);
        if (page == pageEnd) break;
    }

    m_watches.push_back(std::move(entry));
    return true;
}

bool WatchManager::AddOnChangeWatch(uintptr_t address, size_t size,
                                    WatchCallback callback, void* context) {
    return AddWatch(address, size, WatchAccess::Write, TriggerCondition::OnChanged,
                    nullptr, 0, std::move(callback), context);
}


bool WatchManager::RemoveWatch(uintptr_t address, void* context) {
    std::unique_lock lock(m_mutex);
    const auto it = std::find_if(m_watches.begin(), m_watches.end(),
                                 [address, context](const WatchEntry& watch) {
                                     return watch.baseAddress == address &&
                                         (!context || watch.context == context);
                                 });
    if (it == m_watches.end()) return false;

    for (uintptr_t page : it->coveredPages) ReleasePageRef(page);
    m_watches.erase(it);
    return true;
}

LONG CALLBACK WatchManager::VectoredHandler(PEXCEPTION_POINTERS exception_info) {
    PEXCEPTION_RECORD record = exception_info->ExceptionRecord;
    PCONTEXT context = exception_info->ContextRecord;

    if (record->ExceptionCode == EXCEPTION_SINGLE_STEP) {
        if (!tlsState.inStep) return EXCEPTION_CONTINUE_SEARCH;

        WatchManager& manager = getInstance();
        struct CallbackInvocation {
            WatchCallback callback;
            uintptr_t address = 0;
            std::vector<uint8_t> oldValue;
            std::vector<uint8_t> newValue;
            void* context = nullptr;
        };
        std::vector<CallbackInvocation> invocations;

        {
            std::shared_lock lock(manager.m_mutex);

            for (const auto& page : tlsState.writablePages) {
                const auto pageIt = manager.m_pageGuards.find(page.address);
                if (pageIt == manager.m_pageGuards.end()) continue;

                DWORD oldProtect = 0;
                VirtualProtect(reinterpret_cast<LPVOID>(page.address), manager.m_pageSize,
                               pageIt->second.watchProtect, &oldProtect);
            }

            for (auto& pending : tlsState.watches) {
                const auto watchIt = std::find_if(manager.m_watches.begin(), manager.m_watches.end(),
                                                  [&pending](const WatchEntry& watch)
                                                  {
                                                      return watch.id == pending.id;
                                                  });
                if (watchIt == manager.m_watches.end() || !pending.oldValueValid) continue;

                std::vector<uint8_t> newValue(watchIt->size);
                if (!safe_read_raw(newValue.data(), watchIt->baseAddress, watchIt->size)) continue;

                const bool changed = pending.oldValue.size() == newValue.size() &&
                    std::memcmp(pending.oldValue.data(), newValue.data(), newValue.size()) != 0;
                if (!changed) continue;

                bool trigger = false;
                switch (watchIt->condition) {
                case TriggerCondition::Always:
                case TriggerCondition::OnChanged:
                    trigger = true;
                    break;
                case TriggerCondition::OnEqualTo:
                    trigger = watchIt->expectedValueBytes.size() == newValue.size() &&
                        std::memcmp(newValue.data(), watchIt->expectedValueBytes.data(), newValue.size()) == 0;
                    break;
                case TriggerCondition::OnNotEqualTo:
                    trigger = watchIt->expectedValueBytes.size() != newValue.size() ||
                        std::memcmp(newValue.data(), watchIt->expectedValueBytes.data(), newValue.size()) != 0;
                    break;
                }

                if (trigger && watchIt->callback) {
                    invocations.push_back({
                        watchIt->callback, watchIt->baseAddress,
                        std::move(pending.oldValue), std::move(newValue), watchIt->context
                    });
                }
            }
        }

        context->EFlags &= ~0x100;
        tlsState.inStep = false;
        tlsState.watches.clear();
        tlsState.writablePages.clear();

        for (const auto& invocation : invocations) {
            invocation.callback(invocation.address, WatchAccess::Write,
                                invocation.oldValue.data(), invocation.newValue.data(),
                                invocation.newValue.size(), invocation.context);
        }

        return EXCEPTION_CONTINUE_EXECUTION;
    }

    if (record->ExceptionCode != EXCEPTION_ACCESS_VIOLATION ||
        record->NumberParameters < 2 || record->ExceptionInformation[0] != 1) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    WatchManager& manager = getInstance();
    const uintptr_t faultAddress = record->ExceptionInformation[1];
    const uintptr_t pageMask = ~(static_cast<uintptr_t>(manager.m_pageSize) - 1);
    const uintptr_t pageAddress = faultAddress & pageMask;

    std::shared_lock lock(manager.m_mutex);
    const auto pageIt = manager.m_pageGuards.find(pageAddress);
    if (pageIt == manager.m_pageGuards.end()) return EXCEPTION_CONTINUE_SEARCH;

    const bool alreadyWritable = std::any_of(
        tlsState.writablePages.begin(), tlsState.writablePages.end(),
        [pageAddress](const WritablePage& page) { return page.address == pageAddress; });

    if (!alreadyWritable) {
        for (const auto& watch : manager.m_watches) {
            if (std::find(watch.coveredPages.begin(), watch.coveredPages.end(), pageAddress) ==
                watch.coveredPages.end()) {
                continue;
            }
            if (std::any_of(tlsState.watches.begin(), tlsState.watches.end(),
                            [&watch](const PendingWatch& pending) { return pending.id == watch.id; })) {
                continue;
            }

            PendingWatch pending;
            pending.id = watch.id;
            pending.oldValue.resize(watch.size);
            pending.oldValueValid = safe_read_raw(pending.oldValue.data(), watch.baseAddress, watch.size);
            tlsState.watches.push_back(std::move(pending));
        }

        DWORD oldProtect = 0;
        if (!VirtualProtect(reinterpret_cast<LPVOID>(pageAddress), manager.m_pageSize,
                            pageIt->second.originalProtect, &oldProtect)) {
            return EXCEPTION_CONTINUE_SEARCH;
        }
        tlsState.writablePages.push_back({pageAddress});
    }

    tlsState.inStep = true;
    context->EFlags |= 0x100;
    return EXCEPTION_CONTINUE_EXECUTION;
}
