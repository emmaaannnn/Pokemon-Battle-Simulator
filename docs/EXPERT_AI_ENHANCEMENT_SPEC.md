# Pokemon Battle Simulator Expert AI Enhancement Specification

**Project:** Expert AI Phase 2 Implementation  
**Branch:** fix/expert-ai-stub-implementations  
**Status:** All Phase 1 tests passing (216/216)

## Implementation Summary

### 1. **Evaluation Methods Implementation** (`src/expert_ai.cpp`)

**Target Lines: 501-527** - Replace current stub implementations:

```cpp
// Current stubs to implement (all return 0.0):
double evaluateLongTermAdvantage(const BattleState& battleState) const;
double detectSetupAttempt(const BattleState& battleState) const;
double evaluateCounterPlay(const BattleState& battleState) const;
double assessPositionalAdvantage(const BattleState& battleState) const;
double evaluateResourceManagement(const BattleState& battleState) const;
```

**Implementation Approach:**

- **evaluateLongTermAdvantage** (Lines 501-505): Team health ratios, type coverage analysis, stat stage advantages
- **detectSetupAttempt** (Lines 506-509): Pattern matching on opponent stat-boost moves, turn patterns
- **evaluateCounterPlay** (Lines 510-513): Switch timing analysis, revenge kill opportunities  
- **assessPositionalAdvantage** (Lines 518-522): Speed control, type matchup scoring, field effects
- **evaluateResourceManagement** (Lines 523-527): PP conservation, HP preservation, team depth

**Scoring Model Integration:**
- Base scores: 20-50 points per evaluation
- Weight distribution: Long-term (0.3x), Setup detection (2.0x), Counter-play (1.5x), Positional (1.2x), Resource (0.8x)
- Tie-breaker: Highest single evaluation score wins

### 2. **Paralysis Determinism Fix** (`src/pokemon.cpp:218-225`)

**Current Problem:** Non-deterministic RNG using `std::random_device`
```cpp
// Lines 218-225: Problematic code
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_real_distribution<> dis(0.0, 1.0);
return dis(gen) >= 0.25;
```

**Solution:** Route through Battle class seeded RNG
1. Add `Battle* battle_context` parameter to `canAct()` method
2. Replace static RNG with `battle_context->rng` (already seeded in constructor)
3. Update all `canAct()` call sites in Expert AI and tests
4. **Alternative:** Add global seed function if Battle context unavailable

**Files to modify:**
- `/Users/benking/Desktop/Pokemon-Battle-Simulator/src/pokemon.h` (Line 67): Update signature
- `/Users/benking/Desktop/Pokemon-Battle-Simulator/src/pokemon.cpp` (Lines 218-225): Replace RNG
- `/Users/benking/Desktop/Pokemon-Battle-Simulator/src/expert_ai.cpp` (Lines 843-845): Update calls in generateLegalMoves

### 3. **Sleep/Freeze Move Legality** (`src/expert_ai.cpp:825-912`)

**Current generateLegalMoves:** Only checks `canAct()` at line 843
```cpp
if (!active_pokemon->canAct()) {
  continue;  // Skip all moves
}
```

**Enhancement:** Status-specific legality rules
- **Sleep:** Only allow "Sleep Talk" move (if known), otherwise skip turn but advance sleep counter
- **Freeze:** Allow Fire-type moves (thaw chance), otherwise skip turn with 20% thaw chance
- **Implementation:** Add status-specific checks after `canAct()` call

**Test scenarios to verify:**
1. Sleeping Pokemon with Sleep Talk should execute move
2. Frozen Pokemon using Fire move should have higher thaw probability  
3. Sleep/freeze counter progression works correctly

### 4. **Performance Optimization** (`src/expert_ai.cpp:848, 892`)

**Identified Quadratic Patterns:**
```cpp
// Line 848: Full BattleState copy per move option
BattleState new_state = current_state;  

// Line 892: Another full copy per switch option  
BattleState new_state = current_state;  
```

**Optimization Strategy:**
1. **Copy-on-write approach:** Create minimal diff structures instead of full copies
2. **Move pooling:** Pre-allocate BattleState vector with expected size
3. **Selective copying:** Only copy modified fields (active Pokemon, turn counter, specific stats)

**Performance Target:** Reduce minimax search time from O(n²) to O(n log n) for branching factor analysis

## File Touch List & Implementation Order

### Phase 1: Core Evaluations (cpp-engineer)
**Files:** `/Users/benking/Desktop/Pokemon-Battle-Simulator/src/expert_ai.cpp`
- Lines 501-505: `evaluateLongTermAdvantage` - Team health/advantage analysis  
- Lines 506-509: `detectSetupAttempt` - Opponent pattern recognition
- Lines 510-513: `evaluateCounterPlay` - Switch opportunity evaluation
- Lines 518-522: `assessPositionalAdvantage` - Field position scoring
- Lines 523-527: `evaluateResourceManagement` - Resource conservation analysis
- Lines 38, 58: Integration points in chooseBestMove scoring pipeline

### Phase 2: Determinism Fix (Bug-Hunter)  
**Files:** 
- `/Users/benking/Desktop/Pokemon-Battle-Simulator/src/pokemon.h` (Line 67)
- `/Users/benking/Desktop/Pokemon-Battle-Simulator/src/pokemon.cpp` (Lines 218-225)  
- `/Users/benking/Desktop/Pokemon-Battle-Simulator/src/expert_ai.cpp` (Line 843)

### Phase 3: Sleep/Freeze Legality (cpp-engineer)
**Files:** `/Users/benking/Desktop/Pokemon-Battle-Simulator/src/expert_ai.cpp`  
- Lines 838-845: Enhanced status condition checks
- Add sleep/freeze specific move filtering logic

### Phase 4: Performance (Code-Reviewer)
**Files:** `/Users/benking/Desktop/Pokemon-Battle-Simulator/src/expert_ai.cpp`
- Lines 848, 892: Replace full BattleState copies with selective updates
- Lines 825-912: generateLegalMoves optimization

## Acceptance Criteria & Testing

### Functional Requirements
1. **All 216 existing tests pass** after each phase
2. **Scoring measurability:** Each evaluation method produces detectable score differences (±15 points) in controlled scenarios
3. **Deterministic paralysis:** Same seed produces identical battle outcomes
4. **Sleep/freeze legality:** Status-specific move restrictions enforced correctly

### Performance Requirements  
1. **Search time:** <500ms for depth-4 minimax in typical positions
2. **Memory:** <50% increase in peak memory usage during search
3. **No test regressions:** All timing-sensitive tests continue passing

### Test Strategy
- **Unit tests:** Each evaluation method with constructed scenarios
- **Integration tests:** Full battles with seeded RNG for repeatability  
- **Performance tests:** Benchmark search times before/after optimization
- **Regression tests:** Existing 216 tests as safety net

## Implementation Handoff Order

1. **cpp-engineer:** Implement 5 evaluation methods + sleep/freeze legality
2. **Bug-Hunter:** Fix paralysis determinism + validation tests  
3. **Code-Reviewer:** Performance optimization + memory profiling
4. **technical-documenter:** Update evaluation method documentation

**Estimated Timeline:** 2-3 days for full implementation with testing
**Risk Level:** Low - surgical changes with existing test coverage

---
**Critical Success Factors:**
- Maintain existing API compatibility
- All changes must pass current test suite
- Performance improvements measurable via profiling
- Deterministic behavior enables reproducible testing