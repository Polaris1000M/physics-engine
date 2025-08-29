import json

def create_sphere(pos, data):
    data["objects"].append({
      "type": "tetrahedron",
      "size": 0.5,
      "mass": 0.5,
      "position": [pos, 0.5, 0],
      "color": [0.592156862745098, 0.7019607843137254, 0.6823529411764706]
    });

data = {
  "gravity": 9.8,
  "lightDir": [0, -1, 0],
  "cameraDir": [0, -0.25, -1],
  "cameraPos": [2, 5, 8],
  "objects":
  [
  ]
}

half = 10000
ct = 0
for i in range(-half, half):
    create_sphere(i, data)
    ct += 1
    print(ct)

with open("ten.json", "w") as json_file:
    json.dump(data, json_file, indent = 2)
