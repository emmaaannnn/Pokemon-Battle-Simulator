import requests
from pprint import pprint
import json


def main():

    print("starting fetch")
    for i in range(1, 152):
        url = f"https://pokeapi.co/api/v2/pokemon/{i}/"
        response = requests.get(url)
        data = response.json()
        moves = data["moves"]
        filteredMoves = []
        for item in moves:
            for i in range(0, len(item["version_group_details"])):
                if item["version_group_details"][i]["version_group"]["url"] == "https://pokeapi.co/api/v2/version-group/9/" and item["version_group_details"][i]["move_learn_method"]["name"] == "level-up":
                    filteredMoves.append({"move":{
                        "name": item["move"]["name"],
                        "url": item["move"]["url"],
                        "Generation-level-up-data": item["version_group_details"][i]
                            }
                        })
                    pprint(filteredMoves)
        with open("Moves/" + data["forms"][0]["name"] +".json", 'w') as f:
            json.dump(filteredMoves,f)



    #used this to fetch the data for each pokemon's list of move We are using Generation 1 moves
    # print("starting fetch")
    # for i in range(1, 152):
    #     url = f"https://pokeapi.co/api/v2/pokemon/{i}/"
    #     response = requests.get(url)
    #     data = response.json()
    #     moves = data["moves"]
    #     filteredMoves = []
    #     for item in moves:
    #         for i in range(0, len(item["version_group_details"])):
    #             if item["version_group_details"][i]["version_group"]["url"] == "https://pokeapi.co/api/v2/version-group/9/" and item["version_group_details"][i]["move_learn_method"]["name"] == "level-up":
    #                 filteredMoves.append({"move":{
    #                     "name": item["move"]["name"],
    #                     "url": item["move"]["url"],
    #                     "Generation-level-up-data": item["version_group_details"][i]
    #                         }
    #                     })
    #                 pprint(filteredMoves)
    #     with open("Moves/" + data["forms"][0]["name"] +".json", 'w') as f:
    #         json.dump(filteredMoves,f)

        # simplified_data = {
        #     "name": data["name"],
        #     "id": data["id"],
        #     "types": [type_info["type"]["name"] for type_info in data["types"]],
        #     "base_stats": {stat["stat"]["name"]: stat["base_stat"] for stat in data["stats"]}
        # }
        # pprint(simplified_data)

        # with open("Pokemon/" + data["forms"][0]["name"] +".json", 'w') as f:
        #     json.dump(data,f)


# Use this for the more simplified json file, but use the whole file during development (use this when when done)
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
