#pragma once

#include <moth/ecs/world.h>
#include <moth/ui/events/event_key.h>
#include <map>

struct ComponentInput {
  std::map<moth::ui::Key, bool> m_state;
  std::map<moth::ui::Key, bool> m_lastState;
};

class SystemInput {
public:
  static void OnKey(moth::ecs::World &registry, moth::ui::EventKey const &event);
};
