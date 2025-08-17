#include "health_bar_event_listener.h"
#include "pokemon.h"
#include <iostream>

HealthBarEventListener::HealthBarEventListener(std::shared_ptr<HealthBarAnimator> animator)
    : animator_(animator) {
    if (!animator_) {
        animator_ = std::make_shared<HealthBarAnimator>();
    }
}

void HealthBarEventListener::onHealthChanged(const BattleEvents::HealthChangeEvent& event) {
    if (!event.pokemon || !isPokemonRegistered(event.pokemon)) {
        return;
    }
    
    updateHealthBar(event.pokemon, event.newHealth, event.source);
    
    // Optionally log the health change for debugging
    std::string pokemonName = getPokemonDisplayName(event.pokemon);
    if (event.damage > 0) {
        std::cout << pokemonName << " took " << event.damage << " damage";
    } else if (event.damage < 0) {
        std::cout << pokemonName << " healed " << (-event.damage) << " HP";
    }
    
    if (!event.source.empty()) {
        std::cout << " from " << event.source;
    }
    std::cout << std::endl;
}

void HealthBarEventListener::onBattleStart(const BattleEvents::BattleStartEvent& event) {
    // Automatically register the starting Pokemon
    if (event.playerStartPokemon) {
        registerPokemon(event.playerStartPokemon, "Player");
    }
    if (event.aiStartPokemon) {
        registerPokemon(event.aiStartPokemon, "AI");
    }
}

void HealthBarEventListener::onPokemonSwitch(const BattleEvents::PokemonSwitchEvent& event) {
    if (event.newPokemon) {
        std::string prefix = event.isPlayerSwitch ? "Player" : "AI";
        registerPokemon(event.newPokemon, prefix);
        
        // Initialize health bar for the new Pokemon
        updateHealthBar(event.newPokemon, event.newPokemon->current_hp, "switch");
    }
}

void HealthBarEventListener::setAnimationSpeed(HealthBarAnimator::AnimationSpeed speed) {
    if (animator_) {
        // Create new config with updated speed
        HealthBarAnimator::Config newConfig;
        newConfig.speed = speed;
        animator_ = std::make_shared<HealthBarAnimator>(newConfig);
    }
}

void HealthBarEventListener::setColorTheme(HealthBarAnimator::ColorTheme theme) {
    if (animator_) {
        // Create new config with updated theme
        HealthBarAnimator::Config newConfig;
        newConfig.colorTheme = theme;
        animator_ = std::make_shared<HealthBarAnimator>(newConfig);
    }
}

void HealthBarEventListener::registerPokemon(Pokemon* pokemon, const std::string& displayName) {
    if (!pokemon) return;
    
    std::string name = displayName.empty() ? pokemon->name : displayName + " (" + pokemon->name + ")";
    pokemonDisplayNames_[pokemon] = name;
    
    // Display initial health bar
    if (animator_) {
        animator_->displayStaticHealth(name, pokemon->current_hp, pokemon->hp);
        std::cout << "Registered " << name << " - HP: " << pokemon->current_hp << "/" << pokemon->hp << std::endl;
    }
}

void HealthBarEventListener::unregisterPokemon(Pokemon* pokemon) {
    pokemonDisplayNames_.erase(pokemon);
}

bool HealthBarEventListener::isPokemonRegistered(Pokemon* pokemon) const {
    return pokemonDisplayNames_.find(pokemon) != pokemonDisplayNames_.end();
}

std::string HealthBarEventListener::getPokemonDisplayName(Pokemon* pokemon) const {
    auto it = pokemonDisplayNames_.find(pokemon);
    return (it != pokemonDisplayNames_.end()) ? it->second : "Unknown Pokemon";
}

void HealthBarEventListener::updateHealthBar(Pokemon* pokemon, int newHealth, const std::string& /*source*/) {
    if (!animator_ || !pokemon) return;
    
    std::string pokemonName = getPokemonDisplayName(pokemon);
    
    // Display animated health transition
    animator_->displayAnimatedHealth(pokemonName, newHealth, pokemon->hp, pokemon->current_hp);
}

// Factory function implementation
std::shared_ptr<HealthBarEventListener> createHealthBarListener(
    HealthBarAnimator::AnimationSpeed speed,
    HealthBarAnimator::ColorTheme theme) {
    
    HealthBarAnimator::Config config;
    config.speed = speed;
    config.colorTheme = theme;
    
    auto animator = std::make_shared<HealthBarAnimator>(config);
    return std::make_shared<HealthBarEventListener>(animator);
}