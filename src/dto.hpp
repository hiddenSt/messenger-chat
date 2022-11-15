#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>

#include <userver/formats/json.hpp>

namespace messenger::chat {

struct Message {
  std::int32_t sender_id = 0;
  std::int32_t receiver_id = 0;
  std::string message;
};

struct MessageInfo {
  std::int32_t id;
  std::int32_t sender_id;
  std::int32_t receiver_id;
  std::string message;
  std::time_t timepoint;
};

Message Parse(const userver::formats::json::Value& json, userver::formats::parse::To<Message>);

userver::formats::json::Value Serialize(const MessageInfo& data,
                                        userver::formats::serialize::To<userver::formats::json::Value>);

}  // namespace messenger::chat