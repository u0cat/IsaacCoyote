// Created by TsCat on 2026/7/23.

#include "app/rule/handlers/static/health_source_handler.h"

#include <unordered_map>
#include <unordered_set>

#include "isaac_spy/isaac/game.h"
#include "isaac_spy/isaac/manager.h"
#include "isaac_spy/isaac/netplay_manager.h"

using namespace app::rule;

void HealthSourceHandler::evaluate(std::vector<StaticContribution>& out) {
    auto* player_manager = isaac_spy::isaac::Game::get_instance().get_player_manager();

    auto set_stale = [this, &out](const RuleId& rule_id, const std::string& reason)
    {
        if (const auto cached = cache_.find(rule_id); cached != cache_.end()) {
            auto contribution = cached->second;
            contribution.stale = true;
            contribution.stale_reason = reason;
            out.push_back(std::move(contribution));
        }
    };

    if (!player_manager) {
        for (const auto& rule : rules_) {
            set_stale(rule.rule_id, "player manager is unavailable");
        }
        return;
    }

    const auto players = player_manager->get_player_list();
    if (players.empty()) {
        for (const auto& rule : rules_) {
            set_stale(rule.rule_id, "no local player available");
        }
        return;
    }

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
        const auto* health = std::get_if<CompiledHealthSource>(&rule.source);
        if (!health) continue;

        std::unordered_set<isaac_spy::isaac::Player*> matched_players;
        for (auto* player : players) {
            if (!player) continue;
            const auto player_ptr = player->get_this_ptr();

            const EventContext context{
                ptr_player_id(player_ptr),
                player_manager->is_local_player(player_ptr) ? PlayerRelation::Self : PlayerRelation::Other,
                name_of(player_ptr),
            };

            if (!player_matches(rule.players, context)) continue;
            matched_players.insert(player);
        }

        if (matched_players.empty()) {
            set_stale(rule.rule_id, "no player matched the rule filter");
            continue;
        }

        double total = 0.0;
        for (auto* player : matched_players) {
            total += player->get_max_hearts() - player->get_red_hearts();
        }

        StaticContribution contribution{
            .rule_id = rule.rule_id,
            .name = rule.name,
            .rule_order = rule.order,
            .source = StaticSource::Health,
            .source_detail = "Red Heart",
            .modifiers = repeated_modifiers(health->per_red_heart, total),
        };

        cache_.insert_or_assign(rule.rule_id, contribution);
        out.push_back(std::move(contribution));
    }
}
