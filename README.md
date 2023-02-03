# GEDStorm

A parser written in C++ for GEDCOM 5.5.5 Files.

## 1. Getting Started

#### Method 1:

1. Compile and Run the C++ code:
   `g++ -std=c++11 parser.cpp -o parser; ./parser -f > output-files/Shakespeare.txt`

   a. The -f flag indicates pushing the entire output to the file.
   b. The -g flag indicated pushing only the generational output to file.

2. Run the Python3 script:
   `python3 indi_parse.py`
3. cd to the Angular project folder:
   `cd gedcom-parser`

4. Development Server:
   Run `ng serve` for a dev server. Navigate to `http://localhost:4200/`. The app will automatically reload if you change any of the source files.

#### Method 2:

All the commands are entered in shell file at root. Run the file using:
`./shell.sh`

## 2. Status

### Current Status

- Import a GEDCOM file into the program - Currently done manually in the codebase. Idea is to import a file in the Angular frontend.

- THe header information will be parsed and displayed - Might give error in some specific format files

- Added functionality of comments which can be parsed and stored based on req., and ignored from file

- Based on the file format, most relevant and consistent info of individual and family are displayed

- Individuals and their families are linked together to get Relationships

- Four specific modes to be designed for optimal usage

- Process errors in file and display them appropriately

- Currently pushing the objects onto a csv file and using a Python script to convert into JSON file.

- Using Angular to create frontend from the JSON file data

### TODO

- [Not considered] Use [JSON Spirit](https://github.com/cierelabs/json_spirit) or [ThorsSerializer](https://github.com/Loki-Astari/ThorsSerializer) to convert our objects into JSON Strings

- Instead of storing the JSON file locally, use a database to store the same! (redis)

- Improve the frontend to allow for filtering, sorting and searching options

- File input from frontend (import the file from user's disk)

- Fix bugs for specific files that do not parse correctly

### Future ideas

- Display graph/tree structure for family generations

- Compress a file removing irrelevant info, but that can still be parsed by the original parser
