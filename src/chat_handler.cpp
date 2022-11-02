#include "chat_handler.hpp"
#include "user_events_component.hpp"

#include <chrono>

#include <fmt/format.h>

#include <userver/clients/dns/component.hpp>
#include <userver/server/handlers/exceptions.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/storages/secdist/component.hpp>
#include <userver/urabbitmq/component.hpp>
#include <userver/utils/assert.hpp>

#include "dto.hpp"
#include "userver/formats/json/value_builder.hpp"
#include "userver/server/http/http_status.hpp"

namespace messenger::chat {

namespace {

class AddMessageHandler final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-add-message";

  AddMessageHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& component_context)
      : HttpHandlerJsonBase(config, component_context),
        pg_cluster_(component_context.FindComponent<userver::components::Postgres>("messenger-chat").GetCluster()) {}

  userver::formats::json::Value HandleRequestJsonThrow(const userver::server::http::HttpRequest& request,
                                                       const userver::formats::json::Value& json,
                                                       userver::server::request::RequestContext&) const override {
    auto message = json.As<Message>();
    auto now_time_point = std::chrono::system_clock::now();
    auto query_result = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "INSERT INTO messenger_schema.chat(sender_id, receiver_id, timepoint, message) VALUES ($1, $2, $3, $4)");

    if (query_result.RowsAffected() == 0) {
      throw userver::server::handlers::ClientError();
    }

    request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
    userver::formats::json::ValueBuilder builder;
    builder["time_point"] = now_time_point;
    builder["sender_id"] = message.sender_id;
    builder["receiver_id"] = message.receiver_id;

    return builder.ExtractValue();
  }

  userver::storages::postgres::ClusterPtr pg_cluster_;
};

class RemoveMessageHandler final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-delete-message";

  RemoveMessageHandler(const userver::components::ComponentConfig& config,
                       const userver::components::ComponentContext& component_context)
      : HttpHandlerJsonBase(config, component_context),
        pg_cluster_(component_context.FindComponent<userver::components::Postgres>("messenger-chat").GetCluster()) {}

  userver::formats::json::Value HandleRequestJsonThrow(const userver::server::http::HttpRequest& request,
                                                       const userver::formats::json::Value& json,
                                                       userver::server::request::RequestContext&) const override {}

  userver::storages::postgres::ClusterPtr pg_cluster_;
};

class GetMessagesHandler final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-get-messages";

  GetMessagesHandler(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& component_context)
      : HttpHandlerJsonBase(config, component_context),
        pg_cluster_(component_context.FindComponent<userver::components::Postgres>("messenger-chat").GetCluster()) {}

  userver::formats::json::Value HandleRequestJsonThrow(const userver::server::http::HttpRequest& request,
                                                       const userver::formats::json::Value& request_json,
                                                       userver::server::request::RequestContext&) const override {}

  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendChat(userver::components::ComponentList& component_list) {
  component_list.Append<AddMessageHandler>()
      .Append<UserEventsComponent>()
      .Append<GetMessagesHandler>()
      .Append<RemoveMessageHandler>()
      .Append<UserRemovedConsumer>()
      .Append<userver::components::Postgres>("messenger-chat")
      .Append<components::Secdist>()
      .Append<userver::clients::dns::Component>();
}

}  // namespace messenger::chat
