//
// Created by TsCat on 2026/7/13.
//

#ifndef ISAACSPY_WATCHER_MANAGER_H
#define ISAACSPY_WATCHER_MANAGER_H
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>

#include "memory.h"
#include "watch_manager.h"

namespace isaac_spy::mem
{
    template <typename T>
    class MemoryRef {
    public:
        using RefCallback = std::function<void(MemoryRef& memory_ref, const T& old_value, const T& new_value)>;

        MemoryRef(uintptr_t ptr);
        ~MemoryRef();


        MemoryRef(const MemoryRef&) = delete;
        MemoryRef& operator=(const MemoryRef&) = delete;
        MemoryRef(MemoryRef&&) = delete;
        MemoryRef& operator=(MemoryRef&&) = delete;

        T get() const;
        bool set(T value);
        bool watch(RefCallback callback);

        operator T() const {
            return get();
        }

        MemoryRef& operator=(const T& new_value) {
            set(new_value);
            return *this;
        }

    private:
        mutable T value_;
        std::vector<RefCallback> callbacks;

        bool init_watch();
        static void watch_callback(uintptr_t address, watch::WatchAccess access,
                                   const void* oldValue, const void* newValue,
                                   size_t valueSize, void* context);
        bool watch_initialized = false;
        bool write_flag = false;
        uintptr_t ptr_;
    };

    template <typename T>
    MemoryRef<T>::MemoryRef(uintptr_t ptr) : ptr_(ptr) {
        value_ = mem::read_value<T>(ptr_);
    };

    template <typename T>
    MemoryRef<T>::~MemoryRef() {
        if (watch_initialized) {
            watch::WatchManager::getInstance().RemoveWatch(ptr_, this);
        }
    }

    template <typename T>
    T MemoryRef<T>::get() const {
        T value{};
        safe_read_raw(&value, ptr_, sizeof(T));

        value_ = value;
        return value;
    }

    template <typename T>
    bool MemoryRef<T>::set(T value) {
        write_flag = true;
        const bool succeeded = mem::write<T>(ptr_, value);
        write_flag = false;

        if (succeeded) {
            value_ = value;
        }
        return succeeded;
    }

    template <typename T>
    bool MemoryRef<T>::watch(RefCallback callback) {
        if (!watch_initialized && !init_watch()) {
            return false;
        }

        callbacks.push_back(std::move(callback));
        return true;
    }

    template <typename T>
    bool MemoryRef<T>::init_watch() {
        watch_initialized = watch::WatchManager::getInstance().AddOnChangeWatch(
            ptr_, sizeof(T), watch_callback, this);
        return watch_initialized;
    }

    template <typename T>
    void MemoryRef<T>::watch_callback(uintptr_t address, watch::WatchAccess access, const void* oldValue,
                                      const void* newValue, size_t valueSize, void* context) {
        if (!context || !oldValue || !newValue || valueSize != sizeof(T)) return;

        auto* self = static_cast<MemoryRef*>(context);

        T old_value{};
        T new_value{};
        std::memcpy(&old_value, oldValue, sizeof(T));
        std::memcpy(&new_value, newValue, sizeof(T));
        self->value_ = new_value;

        if (self->write_flag) {
            self->write_flag = false;
            return;
        }

        for (const auto& callback : self->callbacks) {
            callback(*self, old_value, new_value);
        }
    }
}
#endif //ISAACSPY_WATCHER_MANAGER_H
