count = 0
arr = []
with open("Basic1.ged", "r") as file:
    for line in file:
        count += 1
        arr.append(line)
# print(count)
print(arr[0:10])
