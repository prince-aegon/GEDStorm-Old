import pandas as pd
import json
import csv


def make_json(csvFilePath, jsonFilePath):

    # create a dictionary
    data = {}

    # Open a csv reader called DictReader
    with open(csvFilePath, encoding='utf-8') as csvf:
        csvReader = csv.DictReader(csvf)
        # Convert each row into a dictionary
        # and add it to data
        for rows in csvReader:

            # Assuming a column named 'No' to
            # be the primary key
            key = rows['id']
            [rows.pop(key) for key in ["id", ""]]
            data[key] = rows

    # Open a json writer, and use the json.dumps()
    # function to dump data
    with open(jsonFilePath, 'w', encoding='utf-8') as jsonf:
        jsonf.write(json.dumps(data, indent=4))


df = pd.read_csv("json_indi/indi_strg.csv", names=None)

first_row = df.columns.tolist()

df = df.drop(df.columns[[0, -1]], axis=1)

df.columns = ["id", "name", "year"]


df.loc[-1] = first_row[1:len(first_row)-1]  # adding a row
df.index = df.index + 1  # shifting index
df.sort_index(inplace=True)

df.to_csv('json_indi/indi_strg_out.csv', sep=',')

make_json("json_indi/indi_strg_out.csv", "json_indi/indi.json")
