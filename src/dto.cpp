#include "dto.hpp"

namespace messenger::chat {

Message Parse(const userver::formats::json::Value& json,
              userver::formats::parse::To<Message>) {
  return Message{json["sender_id"].As<std::int32_t>(),
                 json["receiver_id"].As<std::int32_t>(),
                 json["message"].As<std::string>()};
}

userver::formats::json::Value Serialize(
    const MessageInfo& data,
    userver::formats::serialize::To<userver::formats::json::Value>) {}

}  // namespace messenger::chat