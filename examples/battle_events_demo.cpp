/**
 * @file battle_events_demo.cpp
 * @brief Demonstration of the Observer Pattern implementation for battle events
 * 
 * This example shows how to use the battle event system to create responsive
 * UI components that react to battle events in real-time.
 */

#include "battle_events.h"
#include "health_bar_event_listener.h"
#include "pokemon.h"
#include <iostream>
#include <memory>

// Example custom event listener for logging battle events
class BattleLogger : public BattleEvents::BattleEventListener {
public:
    void onHealthChanged(const BattleEvents::HealthChangeEvent& event) override {
        std::cout << "[LOG] " << event.pokemon->name << " health changed from " 
                  << event.oldHealth << " to " << event.newHealth;
        if (!event.source.empty()) {
            std::cout << " (source: " << event.source << ")";
        }
        std::cout << std::endl;
    }

    void onMoveUsed(const BattleEvents::MoveUsedEvent& event) override {
        std::cout << "[LOG] " << event.user->name << " used " << event.move->name;
        if (event.target) {
            std::cout << " on " << event.target->name;
        }
        std::cout << " (success: " << (event.wasSuccessful ? "yes" : "no")
                  << ", critical: " << (event.wasCritical ? "yes" : "no") << ")" << std::endl;
    }

    void onBattleStart(const BattleEvents::BattleStartEvent& event) override {
        std::cout << "[LOG] Battle started! " << event.playerStartPokemon->name 
                  << " vs " << event.aiStartPokemon->name << std::endl;
    }

    void onBattleEnd(const BattleEvents::BattleEndEvent& event) override {
        std::cout << "[LOG] Battle ended after " << event.totalTurns << " turns. ";
        switch (event.winner) {
            case BattleEvents::BattleEndEvent::Winner::PLAYER:
                std::cout << "Player wins!";
                break;
            case BattleEvents::BattleEndEvent::Winner::AI:
                std::cout << "AI wins!";
                break;
            case BattleEvents::BattleEndEvent::Winner::DRAW:
                std::cout << "It's a draw!";
                break;
        }
        std::cout << std::endl;
    }
};

// Example usage of the battle event system
void demonstrateBattleEvents() {
    std::cout << "=== Battle Events System Demo ===" << std::endl;
    
    // Create event manager
    auto eventManager = std::make_shared<BattleEvents::BattleEventManager>();
    
    // Create listeners
    auto logger = std::make_shared<BattleLogger>();
    auto healthBarListener = createHealthBarListener(
        HealthBarAnimator::AnimationSpeed::NORMAL,
        HealthBarAnimator::ColorTheme::ENHANCED
    );
    
    // Subscribe listeners to events
    eventManager->subscribe(logger);
    eventManager->subscribe(healthBarListener);
    
    std::cout << "Created event manager with " << eventManager->getListenerCount() 
              << " listeners" << std::endl;
    
    // Simulate battle events (normally these would come from the Battle class)
    
    // Create test Pokemon
    Pokemon pikachu("pikachu");
    Pokemon charizard("charizard");
    
    // Simulate battle start
    BattleEvents::BattleStartEvent battleStart{&pikachu, &charizard};
    eventManager->notifyBattleStart(battleStart);
    
    // Simulate some health changes
    auto healthEvent1 = eventManager->createHealthChangeEvent(&pikachu, 100, 85, "move");
    eventManager->notifyHealthChanged(healthEvent1);
    
    auto healthEvent2 = eventManager->createHealthChangeEvent(&charizard, 120, 95, "move");
    eventManager->notifyHealthChanged(healthEvent2);
    
    // Simulate move usage
    Move thunderbolt("thunderbolt");
    auto moveEvent = eventManager->createMoveUsedEvent(&pikachu, &thunderbolt, &charizard, true, false, 2.0);
    eventManager->notifyMoveUsed(moveEvent);
    
    // Simulate battle end
    BattleEvents::BattleEndEvent battleEnd{BattleEvents::BattleEndEvent::Winner::PLAYER, 15};
    eventManager->notifyBattleEnd(battleEnd);
    
    std::cout << "=== Demo Complete ===" << std::endl;
}

// Integration example showing how to add event support to existing Battle class
void showBattleIntegration() {
    std::cout << "\n=== Battle Integration Example ===" << std::endl;
    std::cout << "To integrate events into the Battle class, add:" << std::endl;
    std::cout << "\n1. Add event manager member:" << std::endl;
    std::cout << "   std::shared_ptr<BattleEvents::BattleEventManager> eventManager_;" << std::endl;
    
    std::cout << "\n2. Add event notifications in battle methods:" << std::endl;
    std::cout << "   // In damage calculation method:" << std::endl;
    std::cout << "   auto event = eventManager_->createHealthChangeEvent(pokemon, oldHp, newHp, \"move\");" << std::endl;
    std::cout << "   eventManager_->notifyHealthChanged(event);" << std::endl;
    
    std::cout << "\n3. Allow external listeners:" << std::endl;
    std::cout << "   void Battle::addEventListener(std::shared_ptr<BattleEventListener> listener) {" << std::endl;
    std::cout << "       eventManager_->subscribe(listener);" << std::endl;
    std::cout << "   }" << std::endl;
    
    std::cout << "\n4. Usage in main:" << std::endl;
    std::cout << "   Battle battle(playerTeam, aiTeam);" << std::endl;
    std::cout << "   battle.addEventListener(createHealthBarListener());" << std::endl;
    std::cout << "   battle.addEventListener(std::make_shared<BattleLogger>());" << std::endl;
    std::cout << "   battle.startBattle(); // Now events will be fired!" << std::endl;
}

int main() {
    demonstrateBattleEvents();
    showBattleIntegration();
    return 0;
}