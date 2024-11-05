import requests
from pprint import pprint
import json

for i in range(1, 151):
    url = f"https://pokeapi.co/api/v2/pokemon/{i}/"
    response = requests.get(url)
    data = response.json()
    pprint(data["name"])
    pprint(data["stats"])

    with open(str(i) +".json", 'w') as f:
        json.dump(data,f)

