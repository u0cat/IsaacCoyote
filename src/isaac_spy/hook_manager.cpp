//
// Created by TsCat on 2026/7/10.
//

#include "isaac_spy/hook_manager.h"

#include <utility>

#include "isaac_spy/scanner.h"

using namespace isaac_spy::hook;

std::expected<safetyhook::MidHook*, HookError> HookManager::install_mid(
    std::string name,
    std::string_view pattern,
    std::ptrdiff_t offset,
    safetyhook::MidHookFn callback) {
    if (mid_hooks_.contains(name)) {
        return std::unexpected(HookError{HookErrorCode::DuplicateName, "hook name already exists"});
    }

    mem::Scanner scanner(pattern);
    if (!scanner.valid()) {
        return std::unexpected(HookError{HookErrorCode::InvalidPattern, "invalid AOB pattern"});
    }

    auto scan = scanner.scan(true);
    if (!scan.found) {
        return std::unexpected(HookError{HookErrorCode::PatternNotFound, "AOB pattern was not found"});
    }
    if (scan.addresses.size() != 1) {
        return std::unexpected(HookError{HookErrorCode::PatternNotUnique, "AOB pattern matched more than once"});
    }

    auto* target = scan.addresses.front() + offset;
    auto result = SafetyHookMid::create(target, callback);
    if (result) {
        auto [it, _] = mid_hooks_.emplace(std::move(name), std::move(*result));
        return &it->second;
    }

    return std::unexpected(HookError{HookErrorCode::CreateFailed, "SafetyHook failed to create the mid hook"});
}

std::expected<safetyhook::InlineHook*, HookError> HookManager::install_inline(
    std::string name,
    std::string_view pattern,
    std::ptrdiff_t offset,
    void* callback) {
    if (inline_hooks_.contains(name)) {
        return std::unexpected(HookError{HookErrorCode::DuplicateName, "hook name already exists"});
    }

    mem::Scanner scanner(pattern);
    if (!scanner.valid()) {
        return std::unexpected(HookError{HookErrorCode::InvalidPattern, "invalid AOB pattern"});
    }

    auto scan = scanner.scan(true);
    if (!scan.found) {
        return std::unexpected(HookError{HookErrorCode::PatternNotFound, "AOB pattern was not found"});
    }
    if (scan.addresses.size() != 1) {
        return std::unexpected(HookError{HookErrorCode::PatternNotUnique, "AOB pattern matched more than once"});
    }

    auto* target = scan.addresses.front() + offset;
    auto result = SafetyHookInline::create(target, callback);
    if (result) {
        auto [it, _] = inline_hooks_.emplace(std::move(name), std::move(*result));
        return &it->second;
    }

    return std::unexpected(HookError{HookErrorCode::CreateFailed, "SafetyHook failed to create the inline hook"});
}

std::expected<void, HookError> HookManager::remove_inline(std::string_view name) {
    auto it = inline_hooks_.find(std::string{name});
    if (it == inline_hooks_.end()) {
        return std::unexpected(HookError{HookErrorCode::NotFound, "hook name was not found"});
    }
    inline_hooks_.erase(it);
    return {};
}

std::expected<void, HookError> HookManager::remove_mid(std::string_view name) {
    auto it = mid_hooks_.find(std::string{name});
    if (it == mid_hooks_.end()) {
        return std::unexpected(HookError{HookErrorCode::NotFound, "hook name was not found"});
    }
    mid_hooks_.erase(it);
    return {};
}

bool HookManager::contains(std::string_view name) {
    const std::string key{name};
    return mid_hooks_.contains(key) || inline_hooks_.contains(key);
}

void HookManager::reset() {
    mid_hooks_.clear();
    inline_hooks_.clear();
    hook_instances_.clear();
}
