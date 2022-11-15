#include <dto.hpp>

#include <ctime>
#include <stdexcept>

#include <userver/formats/json/value_builder.hpp>

namespace messenger::chat {

Message Parse(const userver::formats::json::Value& json, userver::formats::parse::To<Message>) {
  return Message{0, json["sender_id"].As<std::int32_t>(), json["receiver_id"].As<std::int32_t>(),
                 json["message"].As<std::string>(), 0};
}

userver::formats::json::Value Serialize(const Message& data,
                                        userver::formats::serialize::To<userver::formats::json::Value>) {
  userver::formats::json::ValueBuilder builder;
  builder["id"] = data.id;
  builder["sender_id"] = data.sender_id;
  builder["receiver_id"] = data.sender_id;
  builder["message"] = data.text;

  // std::tm* calendar_time = std::localtime(&data.timepoint);

  // if (calendar_time == nullptr) {
  //  throw std::runtime_error{"AAAA"};
  // }

  // char buffer[100];
  // std::strftime(buffer, 100, "%Y-%m-%dT%H:%M:%S", calendar_time);
  // std::string time_str{buffer, 100};

  // builder["timepoint"] = time_str;
  return builder.ExtractValue();
}

}  // namespace messenger::chat