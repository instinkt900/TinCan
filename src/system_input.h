#pragma once

#include <entt/entity/fwd.hpp>
#include <moth_ui/events/event_key.h>
#include <map>

struct ComponentInput {
  std::map<moth_ui::Key, bool> m_state;
  std::map<moth_ui::Key, bool> m_lastState;
};

class SystemInput {
public:
  static void OnKey(entt::registry &registry, moth_ui::EventKey const &event);
};
