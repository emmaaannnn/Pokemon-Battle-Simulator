#include "move_type_mapping.h"

// Static member initialisation
std::map<std::string, std::string> MoveTypeMapping::moveTypeMap;
bool MoveTypeMapping::initialised = false;

void MoveTypeMapping::ensureInitialised() {
  if (!initialised) {
    initialiseMoveTypes();
    initialised = true;
  }
}

std::string MoveTypeMapping::getMoveType(const std::string &moveName) {
  ensureInitialised();

  auto it = moveTypeMap.find(moveName);
  if (it != moveTypeMap.end()) {
    return it->second;
  }

  // Default to normal type if move not found
  return "normal";
}

void MoveTypeMapping::initialiseMoveTypes() {
  // Normal type moves
  moveTypeMap["tackle"] = "normal";
  moveTypeMap["scratch"] = "normal";
  moveTypeMap["pound"] = "normal";
  moveTypeMap["pay-day"] = "normal";
  moveTypeMap["mega-punch"] = "normal";
  moveTypeMap["mega-kick"] = "normal";
  moveTypeMap["headbutt"] = "normal";
  moveTypeMap["body-slam"] = "normal";
  moveTypeMap["wrap"] = "normal";
  moveTypeMap["take-down"] = "normal";
  moveTypeMap["double-edge"] = "normal";
  moveTypeMap["tail-whip"] = "normal";
  moveTypeMap["leer"] = "normal";
  moveTypeMap["growl"] = "normal";
  moveTypeMap["roar"] = "normal";
  moveTypeMap["sing"] = "normal";
  moveTypeMap["supersonic"] = "normal";
  moveTypeMap["sonic-boom"] = "normal";
  moveTypeMap["disable"] = "normal";
  moveTypeMap["hyper-beam"] = "normal";
  moveTypeMap["strength"] = "normal";
  moveTypeMap["growth"] = "normal";
  moveTypeMap["quick-attack"] = "normal";
  moveTypeMap["rage"] = "normal";
  moveTypeMap["mimic"] = "normal";
  moveTypeMap["screech"] = "normal";
  moveTypeMap["double-team"] = "normal";
  moveTypeMap["recover"] = "normal";
  moveTypeMap["harden"] = "normal";
  moveTypeMap["minimize"] = "normal";
  moveTypeMap["smokescreen"] = "normal";
  moveTypeMap["confuse-ray"] = "normal";
  moveTypeMap["withdraw"] = "normal";
  moveTypeMap["defense-curl"] = "normal";
  moveTypeMap["barrier"] = "normal";
  moveTypeMap["light-screen"] = "normal";
  moveTypeMap["haze"] = "normal";
  moveTypeMap["reflect"] = "normal";
  moveTypeMap["focus-energy"] = "normal";
  moveTypeMap["bide"] = "normal";
  moveTypeMap["metronome"] = "normal";
  moveTypeMap["mirror-move"] = "normal";
  moveTypeMap["self-destruct"] = "normal";
  moveTypeMap["egg-bomb"] = "normal";
  moveTypeMap["swift"] = "normal";
  moveTypeMap["skull-bash"] = "normal";
  moveTypeMap["spike-cannon"] = "normal";
  moveTypeMap["constrict"] = "normal";
  moveTypeMap["amnesia"] = "normal";
  moveTypeMap["soft-boiled"] = "normal";
  moveTypeMap["dizzy-punch"] = "normal";
  moveTypeMap["flash"] = "normal";
  moveTypeMap["splash"] = "normal";
  moveTypeMap["explosion"] = "normal";
  moveTypeMap["fury-swipes"] = "normal";
  moveTypeMap["bonemerang"] = "normal";
  moveTypeMap["rest"] = "normal";
  moveTypeMap["rock-slide"] = "normal";
  moveTypeMap["hyper-fang"] = "normal";
  moveTypeMap["sharpen"] = "normal";
  moveTypeMap["conversion"] = "normal";
  moveTypeMap["tri-attack"] = "normal";
  moveTypeMap["super-fang"] = "normal";
  moveTypeMap["slash"] = "normal";
  moveTypeMap["substitute"] = "normal";
  moveTypeMap["struggle"] = "normal";

  // Fire type moves
  moveTypeMap["ember"] = "fire";
  moveTypeMap["flamethrower"] = "fire";
  moveTypeMap["fire-punch"] = "fire";
  moveTypeMap["fire-blast"] = "fire";
  moveTypeMap["fire-spin"] = "fire";
  moveTypeMap["flame-wheel"] = "fire";
  moveTypeMap["fire-fang"] = "fire";
  moveTypeMap["blaze-kick"] = "fire";
  moveTypeMap["heat-wave"] = "fire";
  moveTypeMap["flare-blitz"] = "fire";
  moveTypeMap["lava-plume"] = "fire";

  // Water type moves
  moveTypeMap["water-gun"] = "water";
  moveTypeMap["hydro-pump"] = "water";
  moveTypeMap["surf"] = "water";
  moveTypeMap["ice-beam"] = "water";
  moveTypeMap["blizzard"] = "water";
  moveTypeMap["bubble-beam"] = "water";
  moveTypeMap["water-pulse"] = "water";
  moveTypeMap["aqua-tail"] = "water";
  moveTypeMap["scald"] = "water";
  moveTypeMap["bubble"] = "water";
  moveTypeMap["crabhammer"] = "water";
  moveTypeMap["clamp"] = "water";
  moveTypeMap["withdraw"] = "water";
  moveTypeMap["aqua-jet"] = "water";
  moveTypeMap["brine"] = "water";
  moveTypeMap["aqua-ring"] = "water";

  // Electric type moves
  moveTypeMap["thunder-shock"] = "electric";
  moveTypeMap["thunderbolt"] = "electric";
  moveTypeMap["thunder-wave"] = "electric";
  moveTypeMap["thunder"] = "electric";
  moveTypeMap["thunder-punch"] = "electric";
  moveTypeMap["spark"] = "electric";
  moveTypeMap["shock-wave"] = "electric";
  moveTypeMap["discharge"] = "electric";
  moveTypeMap["charge-beam"] = "electric";
  moveTypeMap["thunder-fang"] = "electric";

  // Grass type moves
  moveTypeMap["vine-whip"] = "grass";
  moveTypeMap["razor-leaf"] = "grass";
  moveTypeMap["solar-beam"] = "grass";
  moveTypeMap["petal-dance"] = "grass";
  moveTypeMap["absorb"] = "grass";
  moveTypeMap["mega-drain"] = "grass";
  moveTypeMap["leech-seed"] = "grass";
  moveTypeMap["growth"] = "grass";
  moveTypeMap["sleep-powder"] = "grass";
  moveTypeMap["poison-powder"] = "grass";
  moveTypeMap["stun-spore"] = "grass";
  moveTypeMap["string-shot"] = "grass";
  moveTypeMap["spore"] = "grass";
  moveTypeMap["cotton-spore"] = "grass";
  moveTypeMap["giga-drain"] = "grass";
  moveTypeMap["synthesis"] = "grass";
  moveTypeMap["magical-leaf"] = "grass";
  moveTypeMap["leaf-blade"] = "grass";
  moveTypeMap["energy-ball"] = "grass";
  moveTypeMap["leaf-storm"] = "grass";
  moveTypeMap["power-whip"] = "grass";
  moveTypeMap["seed-bomb"] = "grass";

  // Ice type moves
  moveTypeMap["ice-beam"] = "ice";
  moveTypeMap["blizzard"] = "ice";
  moveTypeMap["aurora-beam"] = "ice";
  moveTypeMap["ice-punch"] = "ice";
  moveTypeMap["powder-snow"] = "ice";
  moveTypeMap["icy-wind"] = "ice";
  moveTypeMap["mist"] = "ice";
  moveTypeMap["haze"] = "ice";
  moveTypeMap["ice-shard"] = "ice";
  moveTypeMap["avalanche"] = "ice";
  moveTypeMap["ice-fang"] = "ice";
  moveTypeMap["sheer-cold"] = "ice";
  moveTypeMap["icicle-spear"] = "ice";

  // Fighting type moves
  moveTypeMap["karate-chop"] = "fighting";
  moveTypeMap["double-kick"] = "fighting";
  moveTypeMap["jump-kick"] = "fighting";
  moveTypeMap["rolling-kick"] = "fighting";
  moveTypeMap["sand-attack"] = "fighting";
  moveTypeMap["headbutt"] = "fighting";
  moveTypeMap["horn-attack"] = "fighting";
  moveTypeMap["fury-attack"] = "fighting";
  moveTypeMap["horn-drill"] = "fighting";
  moveTypeMap["tackle"] = "fighting";
  moveTypeMap["body-slam"] = "fighting";
  moveTypeMap["wrap"] = "fighting";
  moveTypeMap["take-down"] = "fighting";
  moveTypeMap["thrash"] = "fighting";
  moveTypeMap["double-edge"] = "fighting";
  moveTypeMap["low-kick"] = "fighting";
  moveTypeMap["counter"] = "fighting";
  moveTypeMap["seismic-toss"] = "fighting";
  moveTypeMap["submission"] = "fighting";
  moveTypeMap["high-jump-kick"] = "fighting";
  moveTypeMap["mach-punch"] = "fighting";
  moveTypeMap["dynamic-punch"] = "fighting";
  moveTypeMap["focus-punch"] = "fighting";
  moveTypeMap["superpower"] = "fighting";
  moveTypeMap["revenge"] = "fighting";
  moveTypeMap["brick-break"] = "fighting";
  moveTypeMap["arm-thrust"] = "fighting";
  moveTypeMap["sky-uppercut"] = "fighting";
  moveTypeMap["bulk-up"] = "fighting";
  moveTypeMap["cross-chop"] = "fighting";
  moveTypeMap["reversal"] = "fighting";
  moveTypeMap["vital-throw"] = "fighting";
  moveTypeMap["close-combat"] = "fighting";
  moveTypeMap["hammer-arm"] = "fighting";

  // Poison type moves
  moveTypeMap["poison-sting"] = "poison";
  moveTypeMap["smog"] = "poison";
  moveTypeMap["sludge"] = "poison";
  moveTypeMap["poison-powder"] = "poison";
  moveTypeMap["poison-gas"] = "poison";
  moveTypeMap["acid"] = "poison";
  moveTypeMap["sludge-bomb"] = "poison";
  moveTypeMap["toxic"] = "poison";
  moveTypeMap["poison-fang"] = "poison";
  moveTypeMap["poison-jab"] = "poison";
  moveTypeMap["toxic-spikes"] = "poison";
  moveTypeMap["gunk-shot"] = "poison";
  moveTypeMap["cross-poison"] = "poison";

  // Ground type moves
  moveTypeMap["sand-attack"] = "ground";
  moveTypeMap["dig"] = "ground";
  moveTypeMap["bone-club"] = "ground";
  moveTypeMap["bonemerang"] = "ground";
  moveTypeMap["earthquake"] = "ground";
  moveTypeMap["fissure"] = "ground";
  moveTypeMap["sand-tomb"] = "ground";
  moveTypeMap["mud-shot"] = "ground";
  moveTypeMap["mud-bomb"] = "ground";
  moveTypeMap["earth-power"] = "ground";
  moveTypeMap["mud-slap"] = "ground";
  moveTypeMap["spikes"] = "ground";
  moveTypeMap["magnitude"] = "ground";
  moveTypeMap["bone-rush"] = "ground";
  moveTypeMap["mud-sport"] = "ground";
  moveTypeMap["muddy-water"] = "ground";

  // Flying type moves
  moveTypeMap["gust"] = "flying";
  moveTypeMap["wing-attack"] = "flying";
  moveTypeMap["whirlwind"] = "flying";
  moveTypeMap["fly"] = "flying";
  moveTypeMap["peck"] = "flying";
  moveTypeMap["drill-peck"] = "flying";
  moveTypeMap["sky-attack"] = "flying";
  moveTypeMap["aeroblast"] = "flying";
  moveTypeMap["air-slash"] = "flying";
  moveTypeMap["hurricane"] = "flying";
  moveTypeMap["brave-bird"] = "flying";
  moveTypeMap["roost"] = "flying";
  moveTypeMap["tailwind"] = "flying";
  moveTypeMap["air-cutter"] = "flying";
  moveTypeMap["defog"] = "flying";
  moveTypeMap["bounce"] = "flying";
  moveTypeMap["aerial-ace"] = "flying";
  moveTypeMap["feather-dance"] = "flying";
  moveTypeMap["sky-uppercut"] = "flying";

  // Psychic type moves
  moveTypeMap["confusion"] = "psychic";
  moveTypeMap["psychic"] = "psychic";
  moveTypeMap["hypnosis"] = "psychic";
  moveTypeMap["meditate"] = "psychic";
  moveTypeMap["agility"] = "psychic";
  moveTypeMap["quick-attack"] = "psychic";
  moveTypeMap["rage"] = "psychic";
  moveTypeMap["teleport"] = "psychic";
  moveTypeMap["night-shade"] = "psychic";
  moveTypeMap["mimic"] = "psychic";
  moveTypeMap["screech"] = "psychic";
  moveTypeMap["double-team"] = "psychic";
  moveTypeMap["recover"] = "psychic";
  moveTypeMap["light-screen"] = "psychic";
  moveTypeMap["reflect"] = "psychic";
  moveTypeMap["focus-energy"] = "psychic";
  moveTypeMap["bide"] = "psychic";
  moveTypeMap["metronome"] = "psychic";
  moveTypeMap["mirror-move"] = "psychic";
  moveTypeMap["amnesia"] = "psychic";
  moveTypeMap["barrier"] = "psychic";
  moveTypeMap["kinesis"] = "psychic";
  moveTypeMap["soft-boiled"] = "psychic";
  moveTypeMap["dream-eater"] = "psychic";
  moveTypeMap["psybeam"] = "psychic";
  moveTypeMap["psycho-cut"] = "psychic";
  moveTypeMap["zen-headbutt"] = "psychic";
  moveTypeMap["psycho-boost"] = "psychic";
  moveTypeMap["future-sight"] = "psychic";
  moveTypeMap["calm-mind"] = "psychic";
  moveTypeMap["cosmic-power"] = "psychic";
  moveTypeMap["stored-power"] = "psychic";
  moveTypeMap["psyshock"] = "psychic";
  moveTypeMap["miracle-eye"] = "psychic";
  moveTypeMap["extrasensory"] = "psychic";
  moveTypeMap["synchronoise"] = "psychic";

  // Bug type moves
  moveTypeMap["string-shot"] = "bug";
  moveTypeMap["leech-life"] = "bug";
  moveTypeMap["twin-needle"] = "bug";
  moveTypeMap["pin-missile"] = "bug";
  moveTypeMap["spider-web"] = "bug";
  moveTypeMap["fury-cutter"] = "bug";
  moveTypeMap["megahorn"] = "bug";
  moveTypeMap["bug-bite"] = "bug";
  moveTypeMap["x-scissor"] = "bug";
  moveTypeMap["bug-buzz"] = "bug";
  moveTypeMap["signal-beam"] = "bug";
  moveTypeMap["silver-wind"] = "bug";
  moveTypeMap["u-turn"] = "bug";
  moveTypeMap["attack-order"] = "bug";
  moveTypeMap["defend-order"] = "bug";
  moveTypeMap["heal-order"] = "bug";

  // Rock type moves
  moveTypeMap["rock-throw"] = "rock";
  moveTypeMap["rock-slide"] = "rock";
  moveTypeMap["rock-blast"] = "rock";
  moveTypeMap["rollout"] = "rock";
  moveTypeMap["sandstorm"] = "rock";
  moveTypeMap["ancient-power"] = "rock";
  moveTypeMap["rock-tomb"] = "rock";
  moveTypeMap["rock-polish"] = "rock";
  moveTypeMap["power-gem"] = "rock";
  moveTypeMap["stone-edge"] = "rock";
  moveTypeMap["stealth-rock"] = "rock";
  moveTypeMap["rock-wrecker"] = "rock";
  moveTypeMap["head-smash"] = "rock";

  // Ghost type moves
  moveTypeMap["lick"] = "ghost";
  moveTypeMap["night-shade"] = "ghost";
  moveTypeMap["confuse-ray"] = "ghost";
  moveTypeMap["shadow-punch"] = "ghost";
  moveTypeMap["shadow-ball"] = "ghost";
  moveTypeMap["destiny-bond"] = "ghost";
  moveTypeMap["spite"] = "ghost";
  moveTypeMap["curse"] = "ghost";
  moveTypeMap["nightmare"] = "ghost";
  moveTypeMap["grudge"] = "ghost";
  moveTypeMap["astonish"] = "ghost";
  moveTypeMap["shadow-claw"] = "ghost";
  moveTypeMap["ominous-wind"] = "ghost";
  moveTypeMap["shadow-sneak"] = "ghost";
  moveTypeMap["shadow-force"] = "ghost";

  // Dragon type moves
  moveTypeMap["dragon-rage"] = "dragon";
  moveTypeMap["twister"] = "dragon";
  moveTypeMap["dragon-breath"] = "dragon";
  moveTypeMap["dragon-claw"] = "dragon";
  moveTypeMap["dragon-dance"] = "dragon";
  moveTypeMap["dragon-pulse"] = "dragon";
  moveTypeMap["dragon-rush"] = "dragon";
  moveTypeMap["draco-meteor"] = "dragon";
  moveTypeMap["outrage"] = "dragon";
  moveTypeMap["spacial-rend"] = "dragon";
  moveTypeMap["roar-of-time"] = "dragon";

  // Dark type moves
  moveTypeMap["bite"] = "dark";
  moveTypeMap["crunch"] = "dark";
  moveTypeMap["thief"] = "dark";
  moveTypeMap["feint-attack"] = "dark";
  moveTypeMap["pursuit"] = "dark";
  moveTypeMap["torment"] = "dark";
  moveTypeMap["flatter"] = "dark";
  moveTypeMap["memento"] = "dark";
  moveTypeMap["facade"] = "dark";
  moveTypeMap["taunt"] = "dark";
  moveTypeMap["knock-off"] = "dark";
  moveTypeMap["snatch"] = "dark";
  moveTypeMap["fake-tears"] = "dark";
  moveTypeMap["payback"] = "dark";
  moveTypeMap["assurance"] = "dark";
  moveTypeMap["embargo"] = "dark";
  moveTypeMap["fling"] = "dark";
  moveTypeMap["punishment"] = "dark";
  moveTypeMap["sucker-punch"] = "dark";
  moveTypeMap["dark-void"] = "dark";
  moveTypeMap["night-daze"] = "dark";
  moveTypeMap["dark-pulse"] = "dark";
  moveTypeMap["nasty-plot"] = "dark";
  moveTypeMap["night-slash"] = "dark";
  moveTypeMap["switcheroo"] = "dark";

  // Steel type moves
  moveTypeMap["metal-claw"] = "steel";
  moveTypeMap["steel-wing"] = "steel";
  moveTypeMap["meteor-mash"] = "steel";
  moveTypeMap["iron-tail"] = "steel";
  moveTypeMap["metal-sound"] = "steel";
  moveTypeMap["iron-defense"] = "steel";
  moveTypeMap["doom-desire"] = "steel";
  moveTypeMap["gyro-ball"] = "steel";
  moveTypeMap["magnet-rise"] = "steel";
  moveTypeMap["mirror-shot"] = "steel";
  moveTypeMap["flash-cannon"] = "steel";
  moveTypeMap["bullet-punch"] = "steel";
  moveTypeMap["iron-head"] = "steel";
  moveTypeMap["magnet-bomb"] = "steel";
  moveTypeMap["autotomize"] = "steel";
  moveTypeMap["heavy-slam"] = "steel";
  moveTypeMap["gear-grind"] = "steel";

  // Fairy type moves
  moveTypeMap["sweet-kiss"] = "fairy";
  moveTypeMap["charm"] = "fairy";
  moveTypeMap["moonlight"] = "fairy";
  moveTypeMap["sweet-scent"] = "fairy";
  moveTypeMap["fairy-wind"] = "fairy";
  moveTypeMap["draining-kiss"] = "fairy";
  moveTypeMap["crafty-shield"] = "fairy";
  moveTypeMap["flower-shield"] = "fairy";
  moveTypeMap["misty-terrain"] = "fairy";
  moveTypeMap["play-rough"] = "fairy";
  moveTypeMap["fairy-lock"] = "fairy";
  moveTypeMap["aromatic-mist"] = "fairy";
  moveTypeMap["eerie-impulse"] = "fairy";
  moveTypeMap["dazzling-gleam"] = "fairy";
  moveTypeMap["baby-doll-eyes"] = "fairy";
  moveTypeMap["disarming-voice"] = "fairy";
  moveTypeMap["moonblast"] = "fairy";
  moveTypeMap["geomancy"] = "fairy";
  moveTypeMap["light-of-ruin"] = "fairy";
}