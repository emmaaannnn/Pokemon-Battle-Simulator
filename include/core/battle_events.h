#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <string>

// Forward declarations
class Pokemon;
class Move;
enum class StatusCondition;
enum class WeatherCondition;

namespace BattleEvents {

// Event data structures
struct HealthChangeEvent {
    Pokemon* pokemon;
    int oldHealth;
    int newHealth;
    int damage;  // Positive for damage taken, negative for healing
    std::string source;  // "move", "weather", "status", etc.
};

struct StatusChangeEvent {
    Pokemon* pokemon;
    StatusCondition oldStatus;
    StatusCondition newStatus;
    int turnsRemaining;
    std::string source;
};

struct MoveUsedEvent {
    Pokemon* user;
    const Move* move;
    Pokemon* target;
    bool wasSuccessful;
    bool wasCritical;
    double effectiveness;  // Type effectiveness multiplier
};

struct WeatherChangeEvent {
    WeatherCondition oldWeather;
    WeatherCondition newWeather;
    int turnsRemaining;
};

struct PokemonSwitchEvent {
    Pokemon* oldPokemon;
    Pokemon* newPokemon;
    bool isPlayerSwitch;  // true if player, false if AI
};

struct BattleStartEvent {
    Pokemon* playerStartPokemon;
    Pokemon* aiStartPokemon;
};

struct BattleEndEvent {
    enum class Winner { PLAYER, AI, DRAW } winner;
    int totalTurns;
};

// Abstract observer interface
class BattleEventListener {
public:
    virtual ~BattleEventListener() = default;
    
    // Event handlers - override only the events you care about
    virtual void onHealthChanged(const HealthChangeEvent& /*event*/) {}
    virtual void onStatusChanged(const StatusChangeEvent& /*event*/) {}
    virtual void onMoveUsed(const MoveUsedEvent& /*event*/) {}
    virtual void onWeatherChanged(const WeatherChangeEvent& /*event*/) {}
    virtual void onPokemonSwitch(const PokemonSwitchEvent& /*event*/) {}
    virtual void onBattleStart(const BattleStartEvent& /*event*/) {}
    virtual void onBattleEnd(const BattleEndEvent& /*event*/) {}
    virtual void onTurnStart(int /*turnNumber*/) {}
    virtual void onTurnEnd(int /*turnNumber*/) {}
};

// Event manager - handles subscription and notification
class BattleEventManager {
public:
    using ListenerPtr = std::shared_ptr<BattleEventListener>;
    
    // Subscription management
    void subscribe(ListenerPtr listener);
    void unsubscribe(ListenerPtr listener);
    void clear();
    
    // Event notification methods
    void notifyHealthChanged(const HealthChangeEvent& event);
    void notifyStatusChanged(const StatusChangeEvent& event);
    void notifyMoveUsed(const MoveUsedEvent& event);
    void notifyWeatherChanged(const WeatherChangeEvent& event);
    void notifyPokemonSwitch(const PokemonSwitchEvent& event);
    void notifyBattleStart(const BattleStartEvent& event);
    void notifyBattleEnd(const BattleEndEvent& event);
    void notifyTurnStart(int turnNumber);
    void notifyTurnEnd(int turnNumber);
    
    // Utility methods
    size_t getListenerCount() const { return listeners_.size(); }
    bool hasListeners() const { return !listeners_.empty(); }
    
    // Convenience methods for common event creation
    HealthChangeEvent createHealthChangeEvent(Pokemon* pokemon, int oldHp, int newHp, 
                                            const std::string& source);
    StatusChangeEvent createStatusChangeEvent(Pokemon* pokemon, StatusCondition oldStatus, 
                                            StatusCondition newStatus, int turns, 
                                            const std::string& source);
    MoveUsedEvent createMoveUsedEvent(Pokemon* user, const Move* move, Pokemon* target, 
                                    bool successful, bool critical, double effectiveness);

private:
    std::vector<ListenerPtr> listeners_;
    
    // Helper to safely notify all listeners
    template<typename EventType, typename NotifyFunc>
    void notifyAll(const EventType& event, NotifyFunc func);
};

// Convenience type aliases
using EventManager = BattleEventManager;
using EventListener = BattleEventListener;

} // namespace BattleEvents