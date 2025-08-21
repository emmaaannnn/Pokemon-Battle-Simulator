#pragma once

#include "battle_events.h"
#include "health_bar_animator.h"
#include <memory>
#include <unordered_map>

class Pokemon;

// Event listener that connects battle events to health bar animations
class HealthBarEventListener : public BattleEvents::BattleEventListener {
public:
    explicit HealthBarEventListener(std::shared_ptr<HealthBarAnimator> animator);
    ~HealthBarEventListener() override = default;

    // BattleEventListener interface
    void onHealthChanged(const BattleEvents::HealthChangeEvent& event) override;
    void onBattleStart(const BattleEvents::BattleStartEvent& event) override;
    void onPokemonSwitch(const BattleEvents::PokemonSwitchEvent& event) override;

    // Configuration methods
    void setAnimationSpeed(HealthBarAnimator::AnimationSpeed speed);
    void setColorTheme(HealthBarAnimator::ColorTheme theme);
    
    // Health bar management
    void registerPokemon(Pokemon* pokemon, const std::string& displayName = "");
    void unregisterPokemon(Pokemon* pokemon);
    
    // Query methods
    bool isPokemonRegistered(Pokemon* pokemon) const;
    std::shared_ptr<HealthBarAnimator> getAnimator() const { return animator_; }

private:
    std::shared_ptr<HealthBarAnimator> animator_;
    std::unordered_map<Pokemon*, std::string> pokemonDisplayNames_;
    
    // Helper methods
    std::string getPokemonDisplayName(Pokemon* pokemon) const;
    void updateHealthBar(Pokemon* pokemon, int newHealth, int previousHealth, const std::string& source);
};

// Factory function for easy creation
std::shared_ptr<HealthBarEventListener> createHealthBarListener(
    HealthBarAnimator::AnimationSpeed speed = HealthBarAnimator::AnimationSpeed::NORMAL,
    HealthBarAnimator::ColorTheme theme = HealthBarAnimator::ColorTheme::BASIC
);