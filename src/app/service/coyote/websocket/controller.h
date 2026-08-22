//
// Created by TsCat on 2026/7/20.
//

#ifndef ISAACCOYOTE_WS_CONTROLLER_H
#define ISAACCOYOTE_WS_CONTROLLER_H
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include <ixwebsocket/IXWebSocket.h>

#include "app/service/coyote/pulse_helper/types.h"
#include "models/frame.h"

namespace app::coyote::websocket
{
    class WsRemoteClient;

    class WsController {
    public:
        WsController(std::string_view endpoint);
        ~WsController();

        void start();
        void stop();
        bool is_ready();

        std::vector<model::ClientId> client_ids() const;
        std::shared_ptr<WsRemoteClient> find_client(const model::ClientId& client_id) const;

        void broadcast_intensity(model::Channel channel, int intensity);
        void broadcast_pulse(model::Channel channel, model::PulseTrack track);
        void clean();

        std::string get_address();

    private:
        bool send_to(const model::ClientId& client_id, const model::AppMessage& msg);
        void remove_all_clients();

        void handle_frame(const model::ServerFrame& frame);

        std::atomic<bool> stopping_ = false;
        std::atomic<bool> initialized_ = false;

        mutable std::mutex state_mutex_;
        mutable std::mutex clients_mutex_;
        std::unordered_map<model::ClientId, std::shared_ptr<WsRemoteClient>> clients_;

        model::ClientId client_id_;
        ix::WebSocket websocket_;

        // allow client to use send_to()
        friend class WsRemoteClient;
    };
}
#endif //ISAACCOYOTE_WS_CONTROLLER_H
