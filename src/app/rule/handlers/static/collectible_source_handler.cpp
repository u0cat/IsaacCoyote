// Created by TsCat on 2026/7/23.

#include "app/rule/handlers/static/collectible_source_handler.h"

#include <algorithm>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "isaac_spy/isaac/collectible.h"
#include "isaac_spy/isaac/game.h"
#include "isaac_spy/isaac/item_config_manager.h"
#include "isaac_spy/isaac/netplay_manager.h"

namespace app::rule
{
    namespace
    {
        void merge_collectibles(
            const std::unordered_map<int, int>& source,
            std::unordered_map<int, int>& destination) {
            for (const auto& [collectible_id, count] : source) {
                auto [it, inserted] = destination.try_emplace(collectible_id, count);
                if (!inserted) it->second += count;
            }
        }

        void resolve_contribution(
            const std::unordered_map<int, int>& collectibles,
            const CompiledCollectibleSource& rule,
            StaticContribution& contribution
        ) {
            auto& details = contribution.collectible_details;
            details.clear();
            details.reserve(collectibles.size());

            std::vector<ChannelModifiers> matched_modifiers;
            matched_modifiers.reserve(collectibles.size());

            const auto& overrides = rule.override_rule;
            const auto& quality_modifiers = rule.modifiers_by_quality;

            for (const auto& [collectible_id, count] : collectibles) {
                isaac_spy::isaac::CollectibleDesc description{collectible_id};
                const auto* item_config = description.get_config();
                if (!item_config) {
                    details.push_back({
                        .id = collectible_id,
                        .name = "Unknown",
                        .count = count,
                        .matched = false,
                        .rule_source = "无法读取配置",
                    });
                    continue;
                }

                const int item_id = item_config->id;
                const int quality = item_config->quality;
                const ChannelModifiers* base_modifiers = nullptr;
                std::string rule_source = "无匹配规则";

                if (auto it = overrides.find(item_id); it != overrides.end()) {
                    base_modifiers = &it->second;
                    rule_source = "By Override";
                }
                else if (it = quality_modifiers.find(quality); it != quality_modifiers.end()) {
                    base_modifiers = &it->second;
                    rule_source = "By Quality";
                }

                ChannelModifiers applied_modifiers;
                if (base_modifiers) {
                    applied_modifiers = repeated_modifiers(*base_modifiers, count);
                }

                bool ok = false;
                auto name = description.get_localized_name(
                    isaac_spy::isaac::LANGUAGE_CHINESE, ok);
                if (!ok || name.empty()) {
                    name = description.get_localized_name(isaac_spy::isaac::LANGUAGE_ENGLISH, ok);
                    if (!ok || name.empty()) name = "Unknown";
                }

                details.push_back({
                    .id = item_id,
                    .name = std::move(name),
                    .quality = quality,
                    .count = count,
                    .matched = base_modifiers != nullptr,
                    .rule_source = rule_source,
                    .modifiers = applied_modifiers,
                });

                if (base_modifiers) {
                    matched_modifiers.push_back(std::move(applied_modifiers));
                }
            }

            std::ranges::sort(details, {}, &CollectibleContributionDetail::id);
            contribution.modifiers = merge_modifiers(std::move(matched_modifiers));
        }
    } // namespace

    void CollectibleSourceHandler::evaluate(std::vector<StaticContribution>& out) {

        const auto set_stale = [this, &out](const RuleId& rule_id, const std::string_view reason)
        {
            if (const auto cached = cache_.find(rule_id); cached != cache_.end()) {
                out.push_back(cached->second);
                auto& contribution = out.back();
                contribution.stale = true;
                contribution.stale_reason = std::string(reason);
            }
        };

        const auto set_all_stale = [&set_stale, this](const std::string_view reason)
        {
            for (const auto& rule : rules_) set_stale(rule.rule_id, reason);
        };

        auto* player_manager = isaac_spy::isaac::Game::get_instance().get_player_manager();
        if (!player_manager) {
            set_all_stale("player manager is unavailable");
            return;
        }

        const auto players = player_manager->get_player_list();
        if (players.empty()) {
            set_all_stale("no local player available");
            return;
        }

        // Player collectible maps are expensive to rebuild from game memory;
        // read each player once per tick and share across rules.
        std::unordered_map<isaac_spy::isaac::Player*, std::unordered_map<int, int>> collectible_cache;
        const auto collectibles_of = [&collectible_cache](isaac_spy::isaac::Player* player)
            -> const std::unordered_map<int, int>& {
            auto [it, inserted] = collectible_cache.try_emplace(player);
            if (inserted) it->second = player->get_collectibles();
            return it->second;
        };

        const bool needs_names = std::ranges::any_of(rules_, [](const auto& rule)
        {
            return rule.players.scope == PlayerScope::Specific;
        });
        std::unordered_map<std::uintptr_t, std::string> name_cache;
        const auto name_of = [&name_cache, needs_names](const std::uintptr_t player_ptr) -> std::string
        {
            if (!needs_names) return {};
            auto [it, inserted] = name_cache.try_emplace(player_ptr);
            if (inserted) it->second = isaac_spy::isaac::player_name_of(player_ptr);
            return it->second;
        };

        for (const auto& rule : rules_) {
            const auto* collectible_rule = std::get_if<CompiledCollectibleSource>(&rule.source);
            if (!collectible_rule) continue;

            std::unordered_set<isaac_spy::isaac::Player*> matched_players;
            matched_players.reserve(players.size());
            std::unordered_map<int, int> combined_collectibles;

            for (auto* player : players) {
                if (!player) continue;
                const auto player_ptr = player->get_this_ptr();
                const EventContext context{
                    ptr_player_id(player_ptr),
                    player_manager->is_local_player(player_ptr)
                        ? PlayerRelation::Self
                        : PlayerRelation::Other,
                    name_of(player_ptr),
                };
                if (!player_matches(rule.players, context)) continue;

                if (!matched_players.insert(player).second)
                    continue;
                merge_collectibles(collectibles_of(player), combined_collectibles);
            }

            if (matched_players.empty()) {
                set_stale(rule.rule_id, "no player matched the rule filter");
                continue;
            }

            StaticContribution contribution{
                .rule_id = rule.rule_id,
                .name = rule.name,
                .rule_order = rule.order,
                .source = StaticSource::Collectible,
                .source_detail = "藏品",
                .modifiers = {},
                .collectible_details = {},
            };

            resolve_contribution(combined_collectibles, *collectible_rule, contribution);
            cache_.insert_or_assign(rule.rule_id, contribution);
            out.push_back(std::move(contribution));
        }
    }
} // namespace app::rule
