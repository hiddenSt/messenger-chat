#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/concurrent/variable.hpp>
#include <userver/rabbitmq.hpp>
#include <userver/storages/postgres/cluster.hpp>
#include <userver/urabbitmq/consumer_base.hpp>

namespace messenger::chat {

namespace rabbitmq = userver::urabbitmq;
namespace components = userver::components;

class UserEventsComponent final : public components::RabbitMQ {
 public:
  static constexpr std::string_view kName = "user-events";

  UserEventsComponent(const components::ComponentConfig& config, const components::ComponentContext& context);
};

class UserCreatedConsumer final : public rabbitmq::ConsumerComponentBase {
 public:
  static constexpr std::string_view kName = "user-created-consumer";

  UserCreatedConsumer(const components::ComponentConfig& config, const components::ComponentContext& context);

  std::vector<std::int32_t> GetConsumedMessages();

 protected:
  void Process(std::string message) override;

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

class UserRemovedConsumer final : public rabbitmq::ConsumerComponentBase {
 public:
  static constexpr std::string_view kName = "user-removed-consumer";

  UserRemovedConsumer(const components::ComponentConfig& config, const components::ComponentContext& context);

  std::vector<std::int32_t> GetConsumedMessages();

 protected:
  void Process(std::string message) override;

 private:
  userver::storages::postgres::ClusterPtr pg_cluster_;
};

}  // namespace messenger::chat