#include "health_bar_animator.h"
#include <algorithm>
#include <cmath>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#include <cstdlib>
#endif

void HealthBarAnimator::displayAnimatedHealth(const std::string& pokemonName,
                                            int currentHP, int maxHP,
                                            int previousHP,
                                            const std::string& statusCondition) const {
    if (config_.speed == AnimationSpeed::DISABLED || previousHP == -1) {
        displayStaticHealth(pokemonName, currentHP, maxHP, statusCondition);
        return;
    }

    animateHealthTransition(pokemonName, previousHP, currentHP, maxHP, statusCondition);
    updateHealthState(pokemonName, currentHP, maxHP);
}

void HealthBarAnimator::displayStaticHealth(const std::string& pokemonName,
                                          int currentHP, int maxHP,
                                          const std::string& statusCondition) const {
    double healthPercent = calculateHealthPercentage(currentHP, maxHP);
    
    std::cout << pokemonName << " HP: ";
    
    if (config_.colorTheme != ColorTheme::NONE) {
        std::cout << getHealthColor(healthPercent);
    }
    
    std::cout << renderHealthBar(healthPercent, config_.barLength);
    
    if (config_.colorTheme != ColorTheme::NONE) {
        std::cout << getResetColor();
    }
    
    if (config_.showPercentage) {
        std::cout << " " << currentHP << "/" << maxHP;
        std::cout << " (" << static_cast<int>(healthPercent) << "%)";
    }
    
    if (config_.showStatusCondition && !statusCondition.empty()) {
        std::cout << " (" << statusCondition << ")";
    }
    
    std::cout << std::endl;
}

bool HealthBarAnimator::supportsColors() {
#ifdef _WIN32
    // Try to enable VT processing on Windows
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return false;
    
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return false;
    
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        return false;
    }
    return true;
#else
    // Check for color terminal support on Unix/Linux/macOS
    const char* term = getenv("TERM");
    if (!term) return false;
    
    std::string termStr(term);
    return termStr.find("color") != std::string::npos ||
           termStr.find("xterm") != std::string::npos ||
           termStr.find("screen") != std::string::npos ||
           isatty(STDOUT_FILENO);
#endif
}

HealthBarAnimator::Config HealthBarAnimator::detectOptimalConfig() {
    Config config;
    
    if (supportsColors()) {
        config.colorTheme = ColorTheme::BASIC;
    } else {
        config.colorTheme = ColorTheme::NONE;
    }
    
    // Default to normal speed
    config.speed = AnimationSpeed::NORMAL;
    
    return config;
}

double HealthBarAnimator::calculateHealthPercentage(int currentHP, int maxHP) const {
    if (maxHP <= 0) return 0.0;
    return std::max(0.0, std::min(100.0, (static_cast<double>(currentHP) / maxHP) * 100.0));
}

std::string HealthBarAnimator::renderHealthBar(double healthPercent, int barLength) const {
    auto [filledChar, emptyChar] = getBarCharacters();
    int filledBars = static_cast<int>((healthPercent / 100.0) * barLength);
    
    std::string bar = "[";
    for (int i = 0; i < barLength; ++i) {
        if (i < filledBars) {
            bar += filledChar;
        } else {
            bar += emptyChar;
        }
    }
    bar += "]";
    
    return bar;
}

std::pair<std::string, std::string> HealthBarAnimator::getBarCharacters() const {
#ifdef _WIN32
    // Use ASCII characters on Windows for better compatibility
    return {"=", "-"};
#else
    // Use Unicode blocks on Unix/macOS
    return {"█", "░"};
#endif
}

std::string HealthBarAnimator::getHealthColor(double healthPercent) const {
    if (config_.colorTheme == ColorTheme::NONE) {
        return "";
    }
    
    if (healthPercent > 50.0) {
        return "\033[32m";  // Green
    } else if (healthPercent > 20.0) {
        return "\033[33m";  // Yellow
    } else {
        return "\033[31m";  // Red
    }
}

std::string HealthBarAnimator::getResetColor() const {
    if (config_.colorTheme == ColorTheme::NONE) {
        return "";
    }
    return "\033[0m";
}

void HealthBarAnimator::clearLine() const {
    std::cout << "\r\033[K";  // Clear line and return to beginning
    std::cout.flush();
}

void HealthBarAnimator::moveCursorUp(int lines) const {
    if (lines > 0) {
        std::cout << "\033[" << lines << "A";
        std::cout.flush();
    }
}

void HealthBarAnimator::animateHealthTransition(const std::string& pokemonName,
                                              int fromHP, int toHP, int maxHP,
                                              const std::string& statusCondition) const {
    if (fromHP == toHP) {
        displayStaticHealth(pokemonName, toHP, maxHP, statusCondition);
        return;
    }
    
    int hpDifference = std::abs(toHP - fromHP);
    int steps = std::min(std::max(5, hpDifference / 5), 20);  // 5-20 steps
    
    for (int step = 0; step <= steps; ++step) {
        double progress = static_cast<double>(step) / steps;
        int currentAnimHP = fromHP + static_cast<int>((toHP - fromHP) * progress);
        
        // Clear previous line and display current state
        if (step > 0) {
            clearLine();
        }
        
        double healthPercent = calculateHealthPercentage(currentAnimHP, maxHP);
        
        std::cout << pokemonName << " HP: ";
        
        if (config_.colorTheme != ColorTheme::NONE) {
            std::cout << getHealthColor(healthPercent);
        }
        
        std::cout << renderHealthBar(healthPercent, config_.barLength);
        
        if (config_.colorTheme != ColorTheme::NONE) {
            std::cout << getResetColor();
        }
        
        if (config_.showPercentage) {
            std::cout << " " << currentAnimHP << "/" << maxHP;
            std::cout << " (" << static_cast<int>(healthPercent) << "%)";
        }
        
        if (config_.showStatusCondition && !statusCondition.empty()) {
            std::cout << " (" << statusCondition << ")";
        }
        
        std::cout.flush();
        
        if (step < steps) {
            std::this_thread::sleep_for(std::chrono::milliseconds(config_.stepDelayMs));
        }
    }
    
    std::cout << std::endl;
}

void HealthBarAnimator::updateHealthState(const std::string& pokemonName, int hp, int maxHP) const {
    auto now = std::chrono::steady_clock::now();
    
    if (isPokemonPlayer(pokemonName)) {
        lastPlayerState_ = {pokemonName, hp, maxHP, now};
    } else {
        lastOpponentState_ = {pokemonName, hp, maxHP, now};
    }
}

bool HealthBarAnimator::isPokemonPlayer(const std::string& pokemonName) const {
    // Simple heuristic: if the last player state matches this name, it's likely the player
    if (lastPlayerState_.pokemonName == pokemonName) {
        return true;
    }
    
    // Default assumption: first Pokemon encountered is player
    return lastPlayerState_.pokemonName.empty();
}