import requests
from pprint import pprint
import json
import os

folder_path = "D:\\PersonalProjects\\Pokemon\\Pokemon-Battle-Simulator\\Moves"

for filename in os.listdir(folder_path):
    pprint("-----------------------------------------------------------------------------------------")
    file_path = os.path.join(folder_path, filename)
    if os.path.isfile(file_path) and filename.endswith(".json"):
        with open(file_path, 'r') as file:
            data = json.load(file)
            for item in data:
                url = f"https://pokeapi.co/api/v2/move/" + item["move"]["name"]
                response = requests.get(url)
                response = response.json()
                simplified_data = {
                        "name": response["name"],
                        "accuracy": response["accuracy"],
                        "effect_chance": response["effect_chance"],
                        "pp": response["pp"],
                        "priority": response["priority"],
                        "power": response["power"],
                        "damage_class": response["damage_class"],
                        "effect_entries": response["effect_entries"],
                        "Info": response["meta"]
                    }
                pprint(simplified_data)
                with open("D:\PersonalProjects\Pokemon\Pokemon-Battle-Simulator\Moves_Data\\" + str(item["move"]["name"]) +".json", 'w') as f:
                    json.dump(simplified_data, f)