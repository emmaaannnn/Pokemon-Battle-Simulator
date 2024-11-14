import requests
from pprint import pprint
import json

for i in range(2, 152):
    url = f"https://pokeapi.co/api/v2/pokemon/{i}/"
    response = requests.get(url)
    data = response.json()
    simplified_data = {
        "name": data["name"],
        "id": data["id"],
        "types": [type_info["type"]["name"] for type_info in data["types"]],
        "base_stats": {stat["stat"]["name"]: stat["base_stat"] for stat in data["stats"]}
    }
    pprint(simplified_data)

    with open("Pokemon2/" + f"{data['name']}.json", 'w') as f:
        json.dump(simplified_data,f)