// Created by TsCat on 2026/7/23.

#include "app/rule/rule_utils.h"

#include <cmath>
#include <ranges>
#include <type_traits>

namespace app::rule
{
    ModifierOperation compile_operation(const config::StrengthModifier operation)
    {
        switch (operation) {
            case config::StrengthModifier::Override: return ModifierOperation::Override;
            case config::StrengthModifier::Increase: return ModifierOperation::Increase;
            case config::StrengthModifier::Decrease: return ModifierOperation::Decrease;
            case config::StrengthModifier::Multiply: return ModifierOperation::Multiply;
        }
        return ModifierOperation::Override;
    }

    std::optional<Modifier> compile_modifier(const std::optional<config::ModifierConfig>& modifier)
    {
        if (!modifier) return std::nullopt;
        return Modifier{compile_operation(modifier->modifier), modifier->value};
    }

    ChannelModifiers compile_modifiers(const config::ChannelModifiersConfig& modifiers)
    {
        return {compile_modifier(modifiers.channel_a), compile_modifier(modifiers.channel_b)};
    }

    std::optional<CompiledPlayerFilter> compile_player_filter(const config::PlayerFilterConfig& filter)
    {
        CompiledPlayerFilter result;
        switch (filter.scope) {
            case config::PlayerScope::Self: result.scope = PlayerScope::Self; break;
            case config::PlayerScope::Others: result.scope = PlayerScope::Others; break;
            case config::PlayerScope::Any: result.scope = PlayerScope::Any; break;
            case config::PlayerScope::Specific: result.scope = PlayerScope::Specific; break;
        }

        result.player_ids.insert(filter.player_ids.begin(), filter.player_ids.end());
        if (result.scope == PlayerScope::Specific && result.player_ids.empty())
            return std::nullopt;
        return result;
    }

    bool player_matches(const CompiledPlayerFilter& filter, const std::optional<EventContext>& context)
    {
        if (!context)
            return filter.scope == PlayerScope::Any;

        if (filter.scope == PlayerScope::Specific)
            return !context->player_name.empty() && filter.player_ids.contains(context->player_name);

        switch (filter.scope) {
            case PlayerScope::Self: return context->relation == PlayerRelation::Self;
            case PlayerScope::Others: return context->relation == PlayerRelation::Other;
            case PlayerScope::Any: return true;
            case PlayerScope::Specific: break;
        }
        return false;
    }

    namespace
    {
        bool entity_key_matches(const CompiledEntityKey& key, const int type, const int subtype, const int variant)
        {
            const auto matches_field = [](const int wanted, const int got) { return wanted == -1 || wanted == got; };
            return matches_field(key.type, type) &&
                   matches_field(key.subtype, subtype) &&
                   matches_field(key.variant, variant);
        }
    }

    bool entity_blacklisted(const CompiledEntityFilter& filter, const isaac_spy::isaac::Entity& entity)
    {
        return std::ranges::any_of(filter.blacklist_entities, [&](const CompiledEntityKey& key)
        {
            return entity_key_matches(key, entity.get_type(), entity.get_subtype(), entity.get_variant());
        });
    }

    bool entity_whitelisted(const CompiledEntityFilter& filter, const isaac_spy::isaac::Entity& entity)
    {
        return std::ranges::any_of(filter.whitelist_entities, [&](const CompiledEntityKey& key)
        {
            return entity_key_matches(key, entity.get_type(), entity.get_subtype(), entity.get_variant());
        });
    }

    bool item_blacklisted(const CompiledItemFilter& filter, const int item_id)
    {
        return filter.blacklist_items.contains(item_id);
    }

    bool item_whitelisted(const CompiledItemFilter& filter, const int item_id)
    {
        return filter.whitelist_items.contains(item_id);
    }

    namespace
    {
        std::optional<Modifier> repeated_modifier(const std::optional<Modifier>& modifier, const double count)
        {
            if (!modifier || count <= 0.0)
                return std::nullopt;

            auto result = *modifier;
            switch (result.operation) {
                case ModifierOperation::Increase:
                case ModifierOperation::Decrease:
                    result.value *= count;
                    break;
                case ModifierOperation::Multiply:
                    result.value = std::pow(result.value, count);
                    break;
                case ModifierOperation::Override:
                    break;
            }
            return result;
        }
    }

    ChannelModifiers repeated_modifiers(const ChannelModifiers& modifiers, const double count)
    {
        return {repeated_modifier(modifiers.a, count), repeated_modifier(modifiers.b, count)};
    }

    namespace
    {
        std::optional<Modifier> merge_channel(const std::vector<ChannelModifiers>& modifiers, const Channel channel)
        {
            std::optional<Modifier> last_override;
            double additive = 0.0;
            double multiplicative = 1.0;
            bool has_additive = false;
            bool has_multiplicative = false;

            for (const auto& cm : modifiers) {
                const auto& mod = channel == Channel::A ? cm.a : cm.b;
                if (!mod) continue;

                switch (mod->operation) {
                    case ModifierOperation::Override:
                        last_override = mod;
                        additive = 0.0;
                        multiplicative = 1.0;
                        has_additive = false;
                        has_multiplicative = false;
                        break;
                    case ModifierOperation::Increase:
                        additive += mod->value;
                        has_additive = true;
                        break;
                    case ModifierOperation::Decrease:
                        additive -= mod->value;
                        has_additive = true;
                        break;
                    case ModifierOperation::Multiply:
                        multiplicative *= mod->value;
                        has_multiplicative = true;
                        break;
                }
            }

            if (last_override) {
                if (!has_additive && !has_multiplicative)
                    return last_override;
                double base = last_override->value;
                if (has_additive) base += additive;
                if (has_multiplicative) base *= multiplicative;
                return Modifier{ModifierOperation::Override, base};
            }
            if (has_multiplicative && !has_additive)
                return Modifier{ModifierOperation::Multiply, multiplicative};
            if (has_additive && !has_multiplicative)
                return Modifier{additive >= 0.0 ? ModifierOperation::Increase : ModifierOperation::Decrease,
                                std::abs(additive)};
            if (has_additive && has_multiplicative)
                return Modifier{ModifierOperation::Override, additive * multiplicative};
            return std::nullopt;
        }
    }

    ChannelModifiers merge_modifiers(const std::vector<ChannelModifiers>& modifiers) {
        return {merge_channel(modifiers, Channel::A), merge_channel(modifiers, Channel::B)};
    }

    StaticSource source_kind_of(const CompiledStaticSource& source)
    {
        return std::holds_alternative<CompiledHealthSource>(source)
                   ? StaticSource::Health
                   : StaticSource::Collectible;
    }
}
