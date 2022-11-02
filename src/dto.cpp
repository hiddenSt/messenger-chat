#include <dto.hpp>

#include <userver/formats/json/value_builder.hpp>

namespace messenger::chat {

Message Parse(const userver::formats::json::Value& json, userver::formats::parse::To<Message>) {
  return Message{json["sender_id"].As<std::int32_t>(), json["receiver_id"].As<std::int32_t>(),
                 json["message"].As<std::string>()};
}

userver::formats::json::Value Serialize(const MessageInfo& data,
                                        userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["sender_id"] = data.sender_id;
  return builder.ExtractValue();
}

}  // namespace messenger::chat