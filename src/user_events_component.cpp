#include <user_events_component.hpp>

#include <userver/storages/postgres/component.hpp>

#include <cstdint>

namespace messenger::chat {

UserEventsComponent::UserEventsComponent(const components::ComponentConfig& config,
                                         const components::ComponentContext& context)
    : components::RabbitMQ{config, context} {}

UserRemovedConsumer::UserRemovedConsumer(const components::ComponentConfig& config,
                                         const components::ComponentContext& context)
    : rabbitmq::ConsumerComponentBase(config, context) {}

void UserRemovedConsumer::Process(std::string message) {
  std::int32_t user_id = std::stol(message);
  pg_cluster_->Execute(userver::storages::postgres::ClusterHostType::kMaster,
                       "DELETE FROM messenger_schema.chat WHERE receiver_id = $1 OR sender_id = $2", user_id, user_id);
}

}  // namespace messenger::chat