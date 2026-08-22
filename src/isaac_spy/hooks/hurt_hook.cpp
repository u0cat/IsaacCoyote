// Created by TsCat on 2026/7/9.

#include "isaac_spy/hooks/hurt_hook.h"

#include "app/service/log/log_service.h"
#include "isaac_spy/game_constants.h"
#include "isaac_spy/memory.h"

namespace isaac_spy::hook
{
    namespace
    {
        using namespace isaac_spy::constants;

        constexpr std::string_view kHookName = "post_EntityPlayer::TakeDamage";

        spdlog::logger& log_()
        {
            static auto logger = app::log::get("isaac_spy");
            return *logger;
        }
    }

    HurtHook::HurtHook(HookManager& manager)
        : HookBase(manager, HookSpec{kHookName, kPatternEntityPlayerTakeDamage, kOffsetHurtPostCall},
                   &trampoline, &s_active) {}

    void HurtHook::trampoline(SafetyHookContext& context) {
        if (auto* hook = s_active)
            static_cast<HurtHook*>(hook)->handle(context);
    }

    void HurtHook::handle(SafetyHookContext& context) {
        HurtContext hurt{};
        hurt.player = context.edi;
        hurt.cancelled = (context.eax & 0xFF) == 0;

        const auto frame = context.ebp;
        std::uint32_t flags = 0;

        bool read_ok =
                mem::safe_read_raw(&hurt.raw_damage, frame + kOffsetHurtRawDamage, sizeof(hurt.raw_damage)) &&
                mem::safe_read_raw(&hurt.final_damage, frame + kOffsetHurtFinalDamage, sizeof(hurt.final_damage)) &&
                mem::safe_read_raw(&flags, frame + kOffsetHurtDamageFlags, sizeof(flags));
        auto ref_ptr = mem::read_ptr(frame + kOffsetHurtEntityRef);
        if (!read_ok || ref_ptr == 0) return;

        hurt.entity_ref = isaac::EntityRef(ref_ptr);
        hurt.flags = static_cast<isaac::enums::DamageFlag>(flags);

        if (hurt.entity_ref) {
            const auto entity = hurt.entity_ref->get_entity();
            const auto spawner = entity.get_spawner();
            log_().debug("[Hurt] player={:#x} cancelled={} raw={:.3f} final={} flags={:#x} "
                         "entity={:#x} type={} variant={} subtype={} "
                         "spawner={:#x} stype={} svariant={} ssubtype={}",
                         hurt.player,
                         hurt.cancelled ? 1 : 0,
                         hurt.raw_damage,
                         hurt.final_damage,
                         static_cast<unsigned long long>(hurt.flags),
                         entity.get_this_ptr(),
                         entity.get_type(),
                         entity.get_variant(),
                         entity.get_subtype(),
                         spawner.get_this_ptr(),
                         spawner.get_type(),
                         spawner.get_variant(),
                         spawner.get_subtype());
        }
        else {
            log_().debug("[Hurt] player={:#x} cancelled={} raw={:.3f} final={} flags={:#x} (no entity ref)",
                         hurt.player,
                         hurt.cancelled ? 1 : 0,
                         hurt.raw_damage,
                         hurt.final_damage,
                         static_cast<unsigned long long>(hurt.flags));
        }

        emit(hurt);
    }
}
