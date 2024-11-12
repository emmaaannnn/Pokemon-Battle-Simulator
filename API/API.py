import requests
from pprint import pprint
import json


def main():
    print("starting fetch")
    for i in range(1, 152):
        url = f"https://pokeapi.co/api/v2/pokemon/{i}/"
        response = requests.get(url)
        data = response.json()

        # simplified_data = {
        #     "name": data["name"],
        #     "id": data["id"],
        #     "types": [type_info["type"]["name"] for type_info in data["types"]],
        #     "base_stats": {stat["stat"]["name"]: stat["base_stat"] for stat in data["stats"]}
        # }
        # pprint(simplified_data)

        with open("Pokemon/" + data["forms"][0]["name"] +".json", 'w') as f:
            json.dump(data,f)

# def main():
#     for i in range(1, 152):
#         moves = []
#         with open("Pokemon/" + str(i) +".json", 'r') as file:
#             data = json.load(file)
#             for item in data["moves"]:
#                 name = item["move"]["name"]
#                 url = item["move"]["url"]   
#                 response = requests.get(url)
#                 data = response.json()
#                 simplified_data = {
#                     "id": data["id"],
#                     "name": data["name"],
#                     "accuracy": data["accuracy"],
#                     "pp": data["pp"],
#                     "priority": data["priority"],
#                     "power": data["power"],
#                     "damage_class": data["damage_class"],
#                     "effect": data["effect_entries"],
#                     "info":{
#                         "ailment": data["meta"]
#                     }
#                 }
#                 pprint(simplified_data)
#                 moves.append(simplified_data)
  
#         with open("moves/" + str(i) +".json", 'w') as file:
#             data = json.dump(moves, file)

main()
