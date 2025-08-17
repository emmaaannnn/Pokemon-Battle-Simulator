#include "battle_events.h"
#include "pokemon.h"
#include "move.h"
#include <algorithm>

namespace BattleEvents {

// BattleEventManager implementation

void BattleEventManager::subscribe(ListenerPtr listener) {
    if (listener && std::find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
        listeners_.push_back(listener);
    }
}

void BattleEventManager::unsubscribe(ListenerPtr listener) {
    listeners_.erase(
        std::remove(listeners_.begin(), listeners_.end(), listener),
        listeners_.end()
    );
}

void BattleEventManager::clear() {
    listeners_.clear();
}

void BattleEventManager::notifyHealthChanged(const HealthChangeEvent& event) {
    notifyAll(event, [](ListenerPtr listener, const HealthChangeEvent& e) {
        listener->onHealthChanged(e);
    });
}

void BattleEventManager::notifyStatusChanged(const StatusChangeEvent& event) {
    notifyAll(event, [](ListenerPtr listener, const StatusChangeEvent& e) {
        listener->onStatusChanged(e);
    });
}

void BattleEventManager::notifyMoveUsed(const MoveUsedEvent& event) {
    notifyAll(event, [](ListenerPtr listener, const MoveUsedEvent& e) {
        listener->onMoveUsed(e);
    });
}

void BattleEventManager::notifyWeatherChanged(const WeatherChangeEvent& event) {
    notifyAll(event, [](ListenerPtr listener, const WeatherChangeEvent& e) {
        listener->onWeatherChanged(e);
    });
}

void BattleEventManager::notifyPokemonSwitch(const PokemonSwitchEvent& event) {
    notifyAll(event, [](ListenerPtr listener, const PokemonSwitchEvent& e) {
        listener->onPokemonSwitch(e);
    });
}

void BattleEventManager::notifyBattleStart(const BattleStartEvent& event) {
    notifyAll(event, [](ListenerPtr listener, const BattleStartEvent& e) {
        listener->onBattleStart(e);
    });
}

void BattleEventManager::notifyBattleEnd(const BattleEndEvent& event) {
    notifyAll(event, [](ListenerPtr listener, const BattleEndEvent& e) {
        listener->onBattleEnd(e);
    });
}

void BattleEventManager::notifyTurnStart(int turnNumber) {
    for (auto& listener : listeners_) {
        if (listener) {
            listener->onTurnStart(turnNumber);
        }
    }
}

void BattleEventManager::notifyTurnEnd(int turnNumber) {
    for (auto& listener : listeners_) {
        if (listener) {
            listener->onTurnEnd(turnNumber);
        }
    }
}

// Convenience event creation methods

HealthChangeEvent BattleEventManager::createHealthChangeEvent(Pokemon* pokemon, int oldHp, int newHp, 
                                                            const std::string& source) {
    return HealthChangeEvent{
        pokemon,
        oldHp,
        newHp,
        oldHp - newHp,  // Positive for damage, negative for healing
        source
    };
}

StatusChangeEvent BattleEventManager::createStatusChangeEvent(Pokemon* pokemon, StatusCondition oldStatus, 
                                                            StatusCondition newStatus, int turns, 
                                                            const std::string& source) {
    return StatusChangeEvent{
        pokemon,
        oldStatus,
        newStatus,
        turns,
        source
    };
}

MoveUsedEvent BattleEventManager::createMoveUsedEvent(Pokemon* user, const Move* move, Pokemon* target, 
                                                    bool successful, bool critical, double effectiveness) {
    return MoveUsedEvent{
        user,
        move,
        target,
        successful,
        critical,
        effectiveness
    };
}

// Template implementation for safe notification
template<typename EventType, typename NotifyFunc>
void BattleEventManager::notifyAll(const EventType& event, NotifyFunc func) {
    // Create a copy of listeners to avoid issues if listeners are modified during notification
    auto listenersCopy = listeners_;
    
    for (auto& listener : listenersCopy) {
        if (listener) {
            try {
                func(listener, event);
            } catch (...) {
                // Swallow exceptions from listeners to prevent one bad listener 
                // from breaking the notification chain
                // In a production system, you might want to log this
            }
        }
    }
}

} // namespace BattleEvents