#ifndef ISAACSPY_CONFIG_TAB_H
#define ISAACSPY_CONFIG_TAB_H

#include <functional>
#include <string>
#include <vector>

#include "app/service/config/config_service.h"
#include "app/service/overlay/core/input_state.h"
#include "app/service/overlay/types.h"
#include "app/service/overlay/ui/components.h"

namespace app::overlay::tabs
{
    struct CollectibleOption {
        int id = 0;
        int quality = 0;
        std::string name;
        std::string internal_name;
    };

    enum class DeleteTarget {
        None,
        Pulse,
        StaticRule,
        EventRule,
        EventAction,
        CollectibleOverride
    };

    class ConfigTabState {
    public:
        config::AppConfig draft;
        int page = 0;
        bool initialized = false;
        bool dirty = false;
        std::string status;
        ui::Toast toast;

        int selected_static = 0;
        int selected_event = 0;

        std::string selected_pulse;
        std::string pulse_editor_key;
        std::string pulse_name_draft;
        std::string pulse_frames_draft;
        std::string pulse_name_error;

        DeleteTarget pending_delete = DeleteTarget::None;
        int pending_delete_index = -1;
        int pending_collectible_id = 0;
        std::string pending_delete_name;

        bool collectible_picker_open = false;
        bool collectible_picker_multi = false;
        std::string collectible_query;
        int collectible_quality = -1;
        std::optional<int> selected_collectible_id = -1;
        int manual_collectible_id = 1;
        std::vector<CollectibleOption> collectibles;

        std::vector<int> picker_selected_ids;
        bool picker_confirmed = false;
        std::vector<int> picker_result_ids;

        std::vector<CollectibleOption> pills;
        std::vector<CollectibleOption> cards;

        std::vector<std::string> whitelist_entity_drafts;
        std::vector<std::string> blacklist_entity_drafts;
    };

    class ConfigTab : public ITab
    {
    public:
        ConfigTab(std::string id, std::string display_name, InputState& input_state,
                  config::ConfigService& config);

        void render() override;

        std::string get_id() override { return id_; };
        std::string get_display_name() override { return display_name_; };
    private:
        void draw_overview();
        void draw_connection();
        void draw_game();
        void draw_static_rules();
        void draw_events();
        void draw_logging();
        bool draw_modifiers(config::ChannelModifiersConfig& modifiers, const char* id);
        bool draw_player_filter(config::PlayerFilterConfig& players, const char* id);
        bool draw_entity_filter(config::OnHurtConfig& config);
        bool draw_entity_key_list(const char* title, std::vector<config::OnHurtConfig::EntityKey>& keys,
                                  std::vector<std::string>& drafts);
        bool draw_item_filter(config::OnUseActiveItemConfig& config);
        bool draw_item_key_list(const char* title, std::vector<int>& items, bool whitelist);
        bool draw_pill_filter(config::OnUsePillConfig& config);
        bool draw_pill_key_list(const char* title, std::vector<int>& pills, bool whitelist);
        bool draw_card_filter(config::OnUseCardConfig& config);
        bool draw_card_key_list(const char* title, std::vector<int>& cards, bool whitelist);
        bool draw_action(config::EventAction& action, int index);
        void draw_collectible_picker(config::CollectibleSourceConfig& source);
        void draw_collectible_picker_modal(const char* popup_id, const char* title,
                                           const std::function<bool(int)>& is_taken,
                                           bool only_active = false);
        void draw_pill_picker_modal(const char* popup_id, const char* title,
                                    const std::function<bool(int)>& is_taken);
        void draw_card_picker_modal(const char* popup_id, const char* title,
                                    const std::function<bool(int)>& is_taken);
        void draw_delete_modal();
        void load_collectibles();
        void load_pills();
        void load_cards();
        void save_draft();

        std::string id_;
        std::string display_name_;
        ConfigTabState state_;
        InputState& input_state_;
        config::ConfigService& config_;
    };
}

#endif
