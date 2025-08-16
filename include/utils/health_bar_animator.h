#pragma once
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <functional>

/**
 * @brief Health Bar Animator for Pokemon Battle Simulator
 * 
 * Provides smooth animated health bar transitions with color coding
 * and cross-platform terminal support.
 */
class HealthBarAnimator {
public:
    /**
     * @brief Animation speed configuration
     */
    enum class AnimationSpeed {
        DISABLED,  // No animations, static display only
        FAST,      // Quick animations (25ms per frame)
        NORMAL,    // Standard animations (50ms per frame)
        SLOW       // Slower animations (100ms per frame)
    };

    /**
     * @brief Color theme options
     */
    enum class ColorTheme {
        NONE,      // No colors, monochrome display
        BASIC,     // Simple red/yellow/green
        ENHANCED   // Full color palette with effects
    };

    /**
     * @brief Configuration structure for health bar animations
     */
    struct Config {
        AnimationSpeed speed;
        ColorTheme colorTheme;
        int barLength;
        bool showPercentage;
        bool showStatusCondition;
        int stepDelayMs;
        
        Config() : speed(AnimationSpeed::NORMAL), colorTheme(ColorTheme::BASIC), 
                   barLength(20), showPercentage(true), showStatusCondition(true), stepDelayMs(50) {}
        Config(AnimationSpeed spd) : speed(spd), colorTheme(ColorTheme::BASIC), 
                                     barLength(20), showPercentage(true), showStatusCondition(true) {
            switch (speed) {
                case AnimationSpeed::FAST:
                    stepDelayMs = 25;
                    break;
                case AnimationSpeed::NORMAL:
                    stepDelayMs = 50;
                    break;
                case AnimationSpeed::SLOW:
                    stepDelayMs = 100;
                    break;
                case AnimationSpeed::DISABLED:
                    stepDelayMs = 0;
                    break;
            }
        }
    };

    /**
     * @brief Constructor with configuration
     */
    explicit HealthBarAnimator(const Config& config = Config()) : config_(config) {}

    /**
     * @brief Display animated health transition
     * @param pokemonName Name of the Pokemon
     * @param currentHP Current hit points
     * @param maxHP Maximum hit points
     * @param previousHP Previous hit points for animation (-1 for no animation)
     * @param statusCondition Status condition string (empty for none)
     */
    void displayAnimatedHealth(const std::string& pokemonName,
                             int currentHP, int maxHP,
                             int previousHP = -1,
                             const std::string& statusCondition = "") const;

    /**
     * @brief Display static health bar (no animation)
     */
    void displayStaticHealth(const std::string& pokemonName,
                           int currentHP, int maxHP,
                           const std::string& statusCondition = "") const;

    /**
     * @brief Check if terminal supports colors
     */
    static bool supportsColors();

    /**
     * @brief Detect optimal configuration for current terminal
     */
    static Config detectOptimalConfig();

private:
    Config config_;

    /**
     * @brief Internal health state tracking
     */
    struct HealthState {
        std::string pokemonName;
        int hp;
        int maxHP;
        std::chrono::steady_clock::time_point lastUpdate;
    };

    mutable HealthState lastPlayerState_;
    mutable HealthState lastOpponentState_;

    /**
     * @brief Calculate health percentage
     */
    double calculateHealthPercentage(int currentHP, int maxHP) const;

    /**
     * @brief Render health bar string
     */
    std::string renderHealthBar(double healthPercent, int barLength) const;

    /**
     * @brief Get appropriate bar characters for platform
     */
    std::pair<std::string, std::string> getBarCharacters() const;

    /**
     * @brief Get health color based on percentage
     */
    std::string getHealthColor(double healthPercent) const;

    /**
     * @brief Reset color codes
     */
    std::string getResetColor() const;

    /**
     * @brief Clear current line in terminal
     */
    void clearLine() const;

    /**
     * @brief Move cursor up specified lines
     */
    void moveCursorUp(int lines) const;

    /**
     * @brief Animate health transition
     */
    void animateHealthTransition(const std::string& pokemonName,
                               int fromHP, int toHP, int maxHP,
                               const std::string& statusCondition) const;

    /**
     * @brief Update internal health state tracking
     */
    void updateHealthState(const std::string& pokemonName, int hp, int maxHP) const;

    /**
     * @brief Check if Pokemon is player's (vs opponent's)
     */
    bool isPokemonPlayer(const std::string& pokemonName) const;
};