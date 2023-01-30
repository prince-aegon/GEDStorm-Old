# GEDCOM-Parser

A parser in C++ for GEDCOM 5.5.5 Files.

## 1. Running the parser

#### Method 1:

1. Compile and Run the C++ code:
   `g++ -std=c++11 parser.cpp -o parser; ./parser -f > output-files/Shakespeare.txt`

   a. The -f flag indicates pushing the entire output to the file.
   b. The -g flag indicated pushing only the generational output to file.

2. Run the Python3 script:
   `python3 indi_parse.py`
3. cd to the Angular project folder:
   `cd gedcom-parser`

4. Development Server
   Run `ng serve` for a dev server. Navigate to `http://localhost:4200/`. The app will automatically reload if you change any of the source files.

#### Method 2:

All the commands are entered in shell file at root. Run the file using:
`bash shell`

## 2. Status

### Current Status

- Import a GEDCOM file into the program

- THe header information will be parsed and displayed

- Added functionality of comments which can be parsed and stored based on req., and ignored from file

- All individuals are parsed and their name, sex and id are displayed.

- Number of individuals is calculated

- Calculate and link individuals to families

- Create modes to run based on user

- Process errors in file and display them appropriately

### TODO

- Calculate ages of individuals during events

- Use [JSON Spirit](https://github.com/cierelabs/json_spirit) or [ThorsSerializer](https://github.com/Loki-Astari/ThorsSerializer) to convert our objects into JSON Strings

- Use the json strings to store data on a web server

- On multiple parses of a single file, store the json object in a database like Postgres or MySql

### Future ideas

- Process and correct error on the fly

- Remove irrelevant and redundant data from the GEDCOM file before parsing
