import requests
from pprint import pprint
import json

url = f"https://pokeapi.co/api/v2/move/fake-out"
response = requests.get(url)
data = response.json()
simplified_data = {
    "name": data["name"],
    "accuracy": data["accuracy"],
    "effect_chance": data["effect_chance"],
    "pp": data["pp"],
    "priority": data["priority"],
    "power": data["power"],
    "damage_class": data["damage_class"],
    "effect_entries": data["effect_entries"],
    "Info": data["meta"]
}
pprint(simplified_data)

with open(str(data["name"]) +".json", 'w') as f:
    json.dump(simplified_data,f)