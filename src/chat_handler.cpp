#include "chat_handler.hpp"
#include "user_events_component.hpp"

#include <chrono>
#include <string>
#include <optional>

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
#include "userver/storages/postgres/io/row_types.hpp"

namespace messenger::chat {

namespace {

namespace server = userver::server;
namespace json = userver::formats::json;
namespace postgres = userver::storages::postgres;

std::optional<std::string> ValidatePostJson(const json::Value& body) {
  if (!body.HasMember("sender_id")) {
    return "Body must contain integer field 'sender_id'.";
  }

  if (!body.HasMember("receiver_id")) {
    return "Body must contain integer field 'receiver_id'.";
  }

  if (!body.HasMember("message")) {
    return "Body must contain string field 'message'.";
  }

  if (!body["sender_id"].IsUInt64()) {
    return "Field 'sender_id' must be unsigned integer.";
  }

  if (!body["receiver_id"].IsUInt64()) {
    return "Field 'sender_id' must be unsigned integer.";
  }

  if (!body["message"].IsString()) {
    return "Field 'message' must be string.";
  }

  return std::nullopt;
}

class AddMessageHandler final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-add-message";

  AddMessageHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& component_context)
      : HttpHandlerJsonBase(config, component_context),
        pg_cluster_(component_context.FindComponent<userver::components::Postgres>("messenger-chat").GetCluster()) {}

  userver::formats::json::Value HandleRequestJsonThrow(const server::http::HttpRequest& request,
                                                       const json::Value& json,
                                                       server::request::RequestContext&) const override {
    auto error_or_nullopt = ValidatePostJson(json);
    json::ValueBuilder response_body;

    if (error_or_nullopt.has_value()) {
      request.SetResponseStatus(server::http::HttpStatus::kBadRequest);
      response_body["error"] = error_or_nullopt.value();
      
      return response_body.ExtractValue();
    }

    auto message = json.As<Message>();

    if (message.receiver_id == message.sender_id) {
      request.SetResponseStatus(server::http::HttpStatus::kBadRequest);
      response_body["error"] = "sender identifier can not be equal to receiver identifier";

      return response_body.ExtractValue();
    }

    auto now_time_point = std::chrono::system_clock::now();
    auto insert_query = pg_cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                                             "INSERT INTO messenger_schema.chat(sender_id, receiver_id, timepoint, "
                                             "message) VALUES ($1, $2, TO_TIMESTAMP($3), $4) ON CONFLICT DO NOTHING",
                                             message.sender_id, message.receiver_id,
                                             std::chrono::system_clock::to_time_t(now_time_point), message.text);

    if (insert_query.RowsAffected() == 0) {
      throw server::handlers::InternalServerError{};
    }

    auto select_query = pg_cluster_->Execute(
        userver::storages::postgres::ClusterHostType::kMaster,
        "SELECT id, sender_id, receiver_id, message, EXTRACT(EPOCH FROM "
        "timepoint) FROM messenger_schema.chat WHERE sender_id = $1 AND receiver_id "
        "= $2 AND message = $3 AND timepoint = TO_TIMESTAMP($4)",
        message.sender_id, message.receiver_id, message.text, std::chrono::system_clock::to_time_t(now_time_point));
    auto result_set = select_query.AsSetOf<Message>(postgres::kRowTag);

    if (result_set.IsEmpty()) {
      throw server::handlers::InternalServerError{};
    }

    Message message_info = result_set[0];
    request.SetResponseStatus(server::http::HttpStatus::kCreated);
    response_body["message"] = message_info;

    return response_body.ExtractValue();
  }

  postgres::ClusterPtr pg_cluster_;
};

class GetMessagesHandler final : public userver::server::handlers::HttpHandlerJsonBase {
 public:
  static constexpr std::string_view kName = "handler-get-messages";

  GetMessagesHandler(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& component_context)
      : HttpHandlerJsonBase(config, component_context),
        pg_cluster_(component_context.FindComponent<userver::components::Postgres>("messenger-chat").GetCluster()) {}

  userver::formats::json::Value HandleRequestJsonThrow(const server::http::HttpRequest& request, const json::Value&,
                                                       server::request::RequestContext&) const override {
    std::int32_t id = std::stol(request.GetPathArg("id"));
    auto query_result = pg_cluster_->Execute(postgres::ClusterHostType::kMaster,
                                             "SELECT id, sender_id, receiver_id, message, EXTRACT(EPOCH FROM "
                                             "timepoint) FROM messenger_schema.chat WHERE id = $1)",
                                             id);

    if (query_result.RowsAffected() == 0) {
      throw server::handlers::ResourceNotFound();
    }

    auto result_set = query_result.AsSetOf<Message>(postgres::kRowTag);
    Message message_info = result_set[0];
    json::ValueBuilder response;
    response["message"] = message_info;

    return response.ExtractValue();
  }

  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace

void AppendChat(userver::components::ComponentList& component_list) {
  component_list.Append<AddMessageHandler>()
      .Append<UserEventsComponent>()
      .Append<GetMessagesHandler>()
      .Append<UserRemovedConsumer>()
      .Append<userver::components::Postgres>("messenger-chat")
      .Append<components::Secdist>()
      .Append<userver::clients::dns::Component>();
}

}  // namespace messenger::chat
