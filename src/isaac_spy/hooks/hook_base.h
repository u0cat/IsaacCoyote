// Created by TsCat on 2026/8/19.

#ifndef ISAACCOYOTE_HOOK_BASE_H
#define ISAACCOYOTE_HOOK_BASE_H

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <safetyhook.hpp>

#include "isaac_spy/hook_manager.h"
#include "isaac_spy/hooks/types.h"

namespace isaac_spy::hook
{
    struct HookSpec {
        std::string_view name;
        std::string_view pattern;
        std::ptrdiff_t offset = 0;
    };

    namespace detail
    {
        // Deduces the single argument type of any one-argument callable
        // (lambda, function pointer, std::function).
        template <typename T>
        struct callable_argument;

        template <typename T>
        struct callable_argument : callable_argument<decltype(&T::operator())> {};

        template <typename R, typename Arg>
        struct callable_argument<R(Arg)> { using type = Arg; };
        template <typename R, typename Arg>
        struct callable_argument<R (*)(Arg)> { using type = Arg; };
        template <typename R, typename Arg>
        struct callable_argument<R (&)(Arg)> { using type = Arg; };
        template <typename R, typename Arg>
        struct callable_argument<std::function<R(Arg)>> { using type = Arg; };
        template <typename C, typename R, typename Arg>
        struct callable_argument<R (C::*)(Arg) const> { using type = Arg; };
        template <typename C, typename R, typename Arg>
        struct callable_argument<R (C::*)(Arg)> { using type = Arg; };
        template <typename C, typename R, typename Arg>
        struct callable_argument<R (C::*)(Arg) const noexcept> { using type = Arg; };
        template <typename C, typename R, typename Arg>
        struct callable_argument<R (C::*)(Arg) noexcept> { using type = Arg; };

        template <typename Fn>
        using callable_argument_t = typename callable_argument<
            std::remove_cv_t<std::remove_reference_t<Fn>>>::type;
    }

    // Shared lifecycle for event hooks: install on first subscribe, remove on
    // last unsubscribe, type-erased callback registry, idempotent unsubscribe.
    class HookBase : public IHook {
    public:
        using Subscription = std::uint64_t;

        HookBase(HookManager& manager, HookSpec spec,
                 void (*trampoline)(SafetyHookContext&),
                 HookBase** active_slot)
            : manager_(manager), spec_(spec), trampoline_(trampoline), active_slot_(active_slot) {}

        ~HookBase() override {
            callbacks_.clear();
            *active_slot_ = nullptr;
            if (manager_.contains(spec_.name)) (void)manager_.remove_mid(spec_.name);
        }

        HookBase(const HookBase&) = delete;
        HookBase& operator=(const HookBase&) = delete;

        // Rolls back cleanly on install failure (slot cleared, no callback).
        template <typename Fn>
        std::expected<Subscription, HookError> subscribe(Fn&& callback) {
            using Value = std::remove_cv_t<
                std::remove_reference_t<detail::callable_argument_t<Fn>>>;

            if (callbacks_.empty()) {
                on_first_subscribe();

                *active_slot_ = this;

                auto result = manager_.install_mid(std::string{spec_.name}, spec_.pattern,
                                                   spec_.offset, trampoline_);
                if (!result) {
                    *active_slot_ = nullptr;
                    return std::unexpected(result.error());
                }
            }

            const Subscription id = next_subscription_++;
            callbacks_.emplace_back(id, erase<Value>(std::forward<Fn>(callback)));
            return id;
        }

        void unsubscribe(Subscription subscription) {
            std::erase_if(callbacks_, [subscription](const auto& entry) { return entry.first == subscription; });
            if (!callbacks_.empty()) return;

            *active_slot_ = nullptr;
            if (manager_.contains(spec_.name)) (void)manager_.remove_mid(spec_.name);
        }

    protected:
        // Install-time resolution (e.g. restart_game console range).
        virtual void on_first_subscribe() {}

        virtual void handle(SafetyHookContext& context) = 0;

        // Same-thread dispatch: subscribe/unsubscribe (game-thread tick) never
        // run concurrently with emit (game-thread mid-hook), so iteration is
        // safe and needs no copy.
        template <typename Ctx>
        void emit(const Ctx& value) {
            for (const auto& entry : callbacks_)
                if (entry.second) entry.second(&value);
        }

    private:
        using ErasedCallback = std::function<void(const void*)>;

        // Value is fixed by the caller's callback signature; the reverse cast
        // is safe since each hook emits only its own context type.
        template <typename Value>
        static ErasedCallback erase(std::function<void(Value)> callback) {
            if (!callback) return {};
            return [cb = std::move(callback)](const void* value) {
                cb(*static_cast<const Value*>(value));
            };
        }

        HookManager& manager_;
        HookSpec spec_;
        void (*trampoline_)(SafetyHookContext&) = nullptr;
        HookBase** active_slot_ = nullptr;

        std::uint64_t next_subscription_ = 1;
        std::vector<std::pair<Subscription, ErasedCallback>> callbacks_;
    };
}

#endif // ISAACCOYOTE_HOOK_BASE_H
