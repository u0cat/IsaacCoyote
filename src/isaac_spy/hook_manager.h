//
// Created by TsCat on 2026/7/10.
//

#ifndef ISAACSPY_HOOK_H
#define ISAACSPY_HOOK_H
#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>

#include <safetyhook.hpp>

#include "isaac_spy/hooks/types.h"

namespace isaac_spy::hook
{
    class IHook {
    public:
        virtual ~IHook() = default;
    };

    class HookManager {
    public:
        std::expected<safetyhook::MidHook*, HookError> install_mid(
            std::string name,
            std::string_view pattern,
            std::ptrdiff_t offset,
            safetyhook::MidHookFn callback
        );
        std::expected<void, HookError> remove_mid(std::string_view name);

        std::expected<safetyhook::InlineHook*, HookError> install_inline(
            std::string name,
            std::string_view pattern,
            std::ptrdiff_t offset,
            void* callback
        );
        std::expected<void, HookError> remove_inline(std::string_view name);

        bool contains(std::string_view name);
        void reset();

        template <typename T>
        T& get_hook() {
            const std::type_index key{typeid(T)};
            auto& slot = hook_instances_[key];
            if (!slot) slot = std::make_unique<T>(*this);
            return static_cast<T&>(*slot);
        }

    private:
        std::unordered_map<std::string, SafetyHookMid> mid_hooks_;
        std::unordered_map<std::string, SafetyHookInline> inline_hooks_;
        std::unordered_map<std::type_index, std::unique_ptr<IHook>> hook_instances_;
    };
}
#endif //ISAACSPY_HOOK_H
