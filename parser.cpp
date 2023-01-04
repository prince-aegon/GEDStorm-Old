#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <vector>
#include <stack>
#include <regex>
#include <string>
#include "address.h"
using namespace std;

// set file to be read from to variable FILE_NAME

// #define FILE_NAME "/home/sarthak/projects/gedcom/GEDCOM-Files/submitter.ged"
#define FILE_NAME "/home/sarthak/projects/gedcom/GEDCOM-Files/Shakespeare.ged"
// #define FILE_NAME "/home/sarthak/projects/gedcom/GEDCOM-Files/The English and British Kings and Queens.ged"

#define regex_cout "\w*(?<!:)std::cout"

// enum for character set in header
typedef enum cSet
{
    ANSEL = 0,
    UTF8 = 1,
    UTF16 = 2,
    UNICODE = 3,
    ASCII = 4
} CharSet;

// enum for events
typedef enum evSet
{
    BIRT = 0,
    MARR = 1,
    DEAT = 2
} EventSet;

// enum for language in header
typedef enum lSet
{
    English = 0,
    German = 1,
    French = 2,
    Japanese = 3,
    Spanish = 4
} LangSet;

// for every id of an event, we might push it into map of id, object and use it to
// grab data when we encounter a need call.

/*
basic structure for individuals and families will be a map of id to objects with params
which can be recursive in nature, eg: an Individual can have fields father and mother which
are themselves individuals
*/

/*
Elements of header:
1. version, charser, form
2. source record

*/

/*
 Submitter Record:
    Id
    Name
    Address structure
*/

// in hindsight could have been an inherited class from individual
class Submitter
{
public:
    string id;
    string submitterName;
    Address *addr;
};

// Source Record
class SourceRecord
{
public:
    EventSet events[3];
    string date;
    string relInfo1[4];
};

// class for corp data
class CorpRecord
{
public:
    string id;
    string name;
    string website;
    string version;
    CorpRecord(string id)
    {
        this->id = id;
    }
    Address *addr;
};

// class to store information of header
class Header
{
public:
    // name of source
    char source[256];
    string gedcomVersion;
    string formType;
    CharSet encoding;
    LangSet language;

    // corp who built the file
    CorpRecord *corp;
};

// basic class for comment
class Comment
{
public:
    string commentType;
    int commentLength;
};

// an early declaration since it is invoked once in Individual
class Family;

// Data structure for dates
class Date
{
public:
    int date;

    // can change month to an enum
    string month;
    int year;
};

// Class for each individual
class Individual
{
public:
    // basic info
    string id;
    string name;
    string srname;
    string givname;
    char sex;

    // dates
    int birthYear;
    int deathYear;

    // parents
    Individual *father;
    Individual *mother;

    // stores all the families individual is part of based on c or s
    // not implemented, may use some other approach
    map<char, vector<Family>> mpFamilies;
};

/*
Class for Family:

Each family is identified by an id
Each family has a husband and a wife may not be unique to a family
Each family has a list of children
*/

class Family
{
public:
    string id;
    Individual husband;
    Individual wife;
    vector<Individual> children;
    Date MarrDate;
    string place;
};

/*
map : lines
    stores all lines for a particular tag
*/
map<string, vector<vector<string>>>
    lines;

// stores objects of submitter class with id as key
map<string, Submitter *> Submitters;

// 2d vector to help in lines
vector<vector<string>> submitterLines;

// stack having current parser tag and id of current tag
stack<pair<string, string>> curr;

// 3d vector of sets of lines of splitted strings
// THIS IS THE MAIN ACCESS POINT FOR ALL ALGOS
vector<vector<vector<string>>> subsets;

// vector of all strings
vector<vector<string>> all_lines;

// data structure for individuals
map<string, Individual *> Individuals;

// data structure for families
map<string, Family *> Families;

// debugger details
vector<pair<int, string>> debugger;

// storing birth dates
map<string, Date *> BirthDates;

// storing date of death
map<string, Date *> DeathDates;
vector<string> fatherList;

char mode;
// print comment data
void commentCheck(string s, Comment comment)
{
    if (mode == 'f')
    {
        std::cout << "Type    : " << comment.commentType << endl;
        std::cout << "comment : " << s << endl;
        std::cout << "line no : " << comment.commentLength << endl
                  << endl;
    }
}

// basic algo to split string based on a delimiter
vector<string> splitString(string s, string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

void submTag(vector<string> submSplit)
{

    // push current subm line onto 2d vector to add to map
    submitterLines.push_back(submSplit);

    // check if subm tag already in lines map
    if (lines.find("SUBM") == lines.end())
        lines.insert(make_pair("SUBM", submitterLines));
    else
        lines["SUBM"] = submitterLines;

    // store the id
    string submitterId = submSplit[1];

    // create map for submitter object, to create multiple objects of submitter
    // class
    Submitters.insert(make_pair(submSplit[1], new Submitter()));

    // add id to object of current processing id
    Submitters[submitterId]->id = submitterId;

    // push current state onto stack
    curr.push(make_pair("SUBM", submitterId));
}

// some processing related to submitter info
void getName(vector<string> nameSplit, string submitterId)
{

    // process names
    if (nameSplit[0] == "1" && nameSplit[1] == "NAME")
    {
        string name;
        for (int i = 2; i < nameSplit.size(); i++)
        {
            name = name + nameSplit[i] + " ";
        }

        // index id in submitter map and add name in the class object
        Submitters[submitterId]->submitterName = name;
    }

    // pop the current state from stack
    curr.pop();
}

// sanitize lines with comments
pair<int, string> parseComments(string s, int number_of_lines, Comment cTYPE)
{
    string sanitised = "";
    if (s[0] == '/' && s[1] == '/')
    {
        string commentString = s.substr(3, s.length() - 2);
        cTYPE.commentType = "new-line";
        cTYPE.commentLength = number_of_lines;
        commentCheck(commentString, cTYPE);
        sanitised = "null null null";
    }
    else
    {
        for (int i = 0; i < s.length(); i++)
        {
            if (s[i] == ' ' || s[i] == '\t')
            {
                if (s[i + 1] == '/' && s[i + 2] == '/')
                {
                    if (s[i] == ' ')
                        cTYPE.commentType = "space-inline";
                    else if (s[i] == '\t')
                        cTYPE.commentType = "tab-inline";
                    cTYPE.commentLength = number_of_lines;
                    string commentString = s.substr(i + 4, s.length() - (i + 3));
                    sanitised = s.substr(0, i);
                    commentCheck(commentString, cTYPE);
                }
            }
        }
    }

    // check if sanitised string is not null
    if (sanitised == "")
        return make_pair(0, sanitised);
    else
        return make_pair(1, sanitised);
}

// given a block number and a line number give the line number from the
// entire file
int getLine(int m, int n)
{
    int ans = 0;
    for (int i = 0; i < m; i++)
    {
        ans += subsets[i].size();
    }
    ans += n;
    return ans + 1;
}

// given an id, returns individual object for the id
Individual *parseId(string id)
{
    Individual *gtempin;
    for (auto const &x : Individuals)
    {
        if (x.second->id == id)
        {
            gtempin = x.second;
        }
    }
    return gtempin;
}

// given a name, returns individual object for the id
Individual *parseName(string name)
{
    Individual *gtempin;
    for (auto const &x : Individuals)
    {
        if (x.second->name == name)
        {
            gtempin = x.second;
        }
    }
    return gtempin;
}

// given an id, recursively fills a list with names of
// successors
void fetchParent(string id)
{
    fatherList.push_back(parseId(id)->name);
    if (parseId(id)->father != nullptr)
    {

        fetchParent(parseId(id)->father->id);
    }
}

/*
parse dates based on various formats

only a few very specific formats have been implemented
works on shakespeare, might not on others

in various cases hard-coding had to be done
a more cleaner and better approach might be needed
there exists redundancy and issues with the function which
may cause difficulty in adding future formats

regex was used wherever possible to improve the standard of code
*/
Date parseDate(vector<string> date)
{

    // initialize data structure
    Date nDate;

    // set default to avoid extremum in printing
    nDate.date = 0;
    nDate.month = "None";
    nDate.year = 0;

    // regex for date
    std::regex reDate("^[0-9]{2}$");
    // regex for month
    std::regex reYear("^[0-9]{4}$");

    // for (int i = 0; i < date.size(); i++)
    //     cout << date[i] << " ";
    // cout << endl;

    // format - DD MMM YYYY
    if (std::regex_match(date[0], reDate) && std::regex_match(date[date.size() - 1], reYear))
    {
        nDate.date = stoi(date[0]);
        nDate.month = date[1];
        nDate.year = stoi(date[date.size() - 1]);
    }

    // a redundant case based on below case
    if (date.size() == 4)
    {
        nDate.date = stoi(date[1]);
        nDate.month = date[2];
        nDate.year = stoi(date[3]);
    }

    // format - MMM YYYY
    vector<string> months = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

    if (std::find(months.begin(), months.end(), date[0]) != months.end())
    {
        nDate.date = 0;
        nDate.month = date[0];
        nDate.year = stoi(date[1]);
    }

    // special case
    if (date[0] == "ABT" || (date[0] == "BEF") || (date[0] == "AFT"))
    {
        // three cases form - qqq yyyy, qqq mmm yyyy, qqq dd mmm yyyy
        if (date.size() == 2)
        {
            nDate.date = 0;
            nDate.month = "NULL";
            nDate.year = stoi(date[1]);
        }
        else if (date.size() == 3)
        {
            if (std::find(months.begin(), months.end(), date[1]) != months.end())
            {
                nDate.date = 0;
                nDate.month = date[1];
                nDate.year = stoi(date[2]);
            }
        }
        else if (date.size() == 4)
        {
            nDate.date = stoi(date[1]);
            nDate.month = date[2];
            nDate.year = stoi(date[3]);
        }
    }

    // format - yyyy
    if (date.size() == 1)
    {
        if (std::regex_match(date[0], reYear))
        {
            nDate.date = 0;
            nDate.month = "NULL";
            nDate.year = stoi(date[0]);
        }
    }
    return nDate;
}

int main(int argc, char *argv[])
{

    // implement custom flags - TODO
    // -d -> debugger mode
    // -f -> full mode
    // -s -> short mode
    // -g -> generational mode
    // some ways include - if some way of implementing such flags exist
    // use command-line args to read flags
    // then based on flags create cases for code & print statements

    // an example using command-line arguments
    // default base mode
    mode = 'g';
    // // std::cout << argc << argv[0] << argv[1] << endl;
    // if (argv[1] == "-d")
    //     std::cout << "debug mode" << endl;

    // // first flag is for printing generational parents of an individual
    if (string(argv[1]) == "-g")
    {
        std::cout << "Entering Full mode....:" << endl;
        mode = 'g';
    }
    if (string(argv[1]) == "-f")
    {
        std::cout << "Entering generational mode....:" << endl;
        mode = 'f';
    }

    char c, fn[10];
    string s;
    Comment cTYPE;

    // read file name
    if (FILE_NAME == "")
    {
        std::cout << "Enter the file name....:";
        cin >> fn;
    }
    ifstream in(FILE_NAME);
    if (!in)
    {
        std::cout << "Error! File Does not Exist";
        return 0;
    }
    else
    {
        std::cout << "Loading File...:" << endl;
    }
    int number_of_lines = 0;
    curr.push(make_pair("NULL", "@00@"));

    std::cout << endl;
    if (mode == 'f')
    {
        std::cout << "Parsing Start" << endl;

        std::cout << "Printing Comments..." << endl;
        std::cout << "-------------------------------" << endl;
    }
    while (in.eof() == 0)
    {

        number_of_lines++;
        getline(in, s);

        // working on comments

        pair<int, string> s1 = parseComments(s, number_of_lines, cTYPE);
        // sanitise all comments
        if (s1.first == 1)
            s = s1.second;

        // split current line
        vector<string> split = splitString(s, " ");

        // store lines in an list
        all_lines.push_back(split);

        // process subm tag
        // if (split.size() >= 3 && split[0] == "0" && split[2] == "SUBM")
        // {
        //     submTag(split);
        //     continue;
        // }

        // // get name for subm tag
        // if (curr.top().first == "SUBM")
        // {
        //     getName(split, curr.top().second);
        // }
    }
    std::cout << "-------------------------------" << endl;

    // print number of lines
    std::cout
        << "Number of lines...:" << number_of_lines << endl;
    std::cout << endl;

    /*
    Access to ged code is done in a 3D Matrix
    We have divided the ged file into blocks of atomic relevance
    Each block is either a family, an individual, a header, etc.
    Blocks are defined using first char of first line being a 0.

    Each block has lines
    Each line has words

    subsets[i][j][k] -> block i, line j, word k
    */

    // split all_lines into blocks of text
    for (int i = 0; i < all_lines.size();)
    {
        // if 0 in ged line then we split from that line onwards
        if (all_lines[i][0] == "0")
        {
            vector<vector<string>> temp;
            temp.push_back(all_lines[i]);
            i++;

            // insert into temp till we encounter next 0
            while ((i + 1) < all_lines.size() && all_lines[i][0] != "0")
            {
                temp.push_back(all_lines[i]);
                i++;
            }

            // push temp to subsets
            subsets.push_back(temp);
        }
    }

    // print all the blocks
    //
    //  for (int i = 0; i < subsets.size(); i++)
    //  {
    //      for (int j = 0; j < subsets[i].size(); j++)
    //      {
    //          for (int k = 0; k < subsets[i][j].size(); k++)
    //          {
    //              std::cout << subsets[i][j][k] << " ";
    //          }
    //          std::cout << endl;
    //      }
    //      std::cout << endl;
    //      std::cout << "----------------------" << endl;
    //      std::cout << endl;
    //  }

    // parse all submitters
    // start processing header
    int header_main = 1;
    Submitter header_submitter;
    Header header;
    CorpRecord corp("0");
    Address addrCorp;
    header.corp = &corp;
    int i = 0; // indexing the first set that is header

    // start from first line
    if (subsets[i][0].size() == 2 && subsets[i][0][1] == "HEAD")
    {
        // init header object
        int j = 0, k = 0, submp = 0;

        // iterate through lines of header
        for (j = 0; j < subsets[i].size(); j++)
        {

            // extract version info
            // header main to keep diff bw gedcom version and software version
            if (subsets[i][j][1] == "VERS" && header_main == 1)
            {
                header.gedcomVersion = subsets[i][j][2];
                header_main = 0;
            }

            // charset info
            else if (subsets[i][j][1] == "CHAR")
            {
                if (subsets[i][j][2] == "UTF-8")
                    header.encoding = UTF8;
                else if (subsets[i][j][2] == "UNICODE")
                    header.encoding = UNICODE;
                else if (subsets[i][j][2] == "ANSEL")
                    header.encoding = ANSEL;
                else if (subsets[i][j][2] == "ASCII")
                    header.encoding = ASCII;
                else if (subsets[i][j][2] == "UTF-16")
                    header.encoding = UTF16;
                else
                {
                    string debug = "Invalid encoding encountered";
                    debugger.push_back(make_pair(getLine(i, j), debug));
                    // throw invalid_argument("Invalid Encoding");
                }
            }

            // form info
            else if (subsets[i][j][1] == "FORM")
            {
                header.formType = subsets[i][j][2];
            }

            // source software info
            else if (subsets[i][j][1] == "SOUR")
            {
                curr.push(make_pair("SOUR", subsets[i][j][2]));
                k = j;
            }

            // submitter id object made
            else if (subsets[i][j].size() > 2 && subsets[i][j][1] == "SUBM")
            {
                header_submitter.id = subsets[i][j][1];
            }

            // lang info
            else if (subsets[i][j][1] == "LANG")
            {
                if (subsets[i][j][2] == "English")
                    header.language = English;
                else if (subsets[i][j][2] == "German")
                    header.language = German;
                else if (subsets[i][j][2] == "French")
                    header.language = French;
                else if (subsets[i][j][2] == "Spanish")
                    header.language = Spanish;
                else if (subsets[i][j][2] == "Japanese")
                    header.language = Japanese;
                else
                {
                    string debug = "Invalid language encountered";
                    debugger.push_back(make_pair(getLine(i, j), debug));
                    // throw invalid_argument("Invalid Language");
                }
            }
        }

        // start parsing source software
        if (curr.top().first == "SOUR")
        {
            // std::cout << "in sour" << endl;
            // parse corp record in object
            corp.id = (curr.top().second);

            for (k = k; k < subsets[i].size(); k++, j++)
            {
                if (subsets[i][k][1] == "NAME")
                {
                    corp.name = subsets[i][k][2];
                }
                else if (subsets[i][k][1] == "VERS")
                {
                    corp.version = subsets[i][k][2];
                }
                else if (subsets[i][k][1] == "CITY")
                {
                    addrCorp.city = subsets[i][k][2];
                }
                else if (subsets[i][k][1] == "CTRY")
                {
                    addrCorp.country = subsets[i][k][2];
                }
                else if (subsets[i][k][1] == "POST")
                {
                    addrCorp.post = subsets[i][k][2];
                }
                else if (subsets[i][k][1] == "WWW")
                {
                    corp.website = subsets[i][k][2];
                }
                corp.addr = &addrCorp;
            }
            curr.pop();
        }

        // if (split.size() >= 3 && split[0] == "0" && split[2] == "SUBM")
        // {
        //     submTag(split);
        //     continue;
        // }

        // // get name for subm tag
        // if (curr.top().first == "SUBM")
        // {
        //     getName(split, curr.top().second);
        // }
    }

    // printing all header info
    if (mode == 'f')
    {
        for (int i = 0; i < 2; i++)
            std::cout << endl;
        std::cout << "Parsing Header info...:" << endl;
        std::cout << "-------------------------------" << endl;
        std::cout << endl;
        std::cout << "Header version   : " << header.gedcomVersion << endl;
        std::cout << "Header encoding  : " << header.encoding << endl;
        std::cout << "Header form type : " << header.formType << endl;
        std::cout << "Header  Language : " << header.language << endl;
        std::cout << "Header corp      : " << header.corp->id << endl;
        std::cout << "Header corp name : " << header.corp->name << endl;
        std::cout << "Header corp city : " << header.corp->addr->city << endl;
        std::cout << "Header corp web  : " << header.corp->website << endl;
        std::cout << "Header corp vers : " << header.corp->version << endl;
        std::cout << endl;
        // std::cout << "-------------------------------" << endl;
        std::cout << endl;
        // print various functionalities
        // int i = 0;
        // for (auto const &x : Submitters)
        // {
        //     std::std::cout << x.first << " : " << x.second->submitterName << endl;
        // }
        // std::std::cout << endl;
        // for (auto const &x : lines)
        // {
        //     vector<vector<string>> slines = x.second;
        //     std::std::cout << x.first << endl;
        //     for (int i = 0; i < slines.size(); i++)
        //     {
        //         for (int j = 0; j < slines[i].size(); j++)
        //             std::std::cout << slines[i][j] << " ";
        //         std::std::cout << endl;
        //     }
        // }

        // Parse all Individuals
        std::cout << endl;
        std::cout << "----------------------------------------- " << endl;
        std::cout << endl;
        std::cout << "Individual details : " << endl;
        std::cout << endl;
    }

    // iterate through all blocks
    stack<char> event;
    for (int i = 0; i < subsets.size(); i++)
    {
        // check if the block is an individual
        vector<Family> fm;
        if (subsets[i][0].size() > 2 && subsets[i][0][2] == "INDI")
        {
            // if block is individual then insert object into ds
            // std::cout << "Working on ...:" << subsets[i][0][1] << endl;

            // insert into map an object linked to its individuals id
            Individuals.insert(make_pair(subsets[i][0][1], new Individual()));

            // redundantly put the same id into a field in Individual class
            // thus id can be accessed from a field in Individual class and
            // as key in Individuals map

            Individuals[subsets[i][0][1]]->id = subsets[i][0][1];

            for (int j = 0; j < subsets[i].size(); j++)
            {
                if (subsets[i][j][1] == "NAME")
                {
                    string name = "";
                    for (int k = 2; k < int(subsets[i][j].size()); k++)
                    {
                        // sanitize name - contains / to separate words in name
                        if (subsets[i][j][k][0] == '/')
                            name += subsets[i][j][k].substr(1, (subsets[i][j][k]).size() - 2);
                        else
                            name += subsets[i][j][k] + " ";
                    }
                    Individuals[subsets[i][0][1]]->name = name;
                }
                else if (subsets[i][j][1] == "SURN")
                {
                    Individuals[subsets[i][0][1]]->srname = subsets[i][j][2];
                }
                else if (subsets[i][j][1] == "GIVN")
                {
                    Individuals[subsets[i][0][1]]->givname = subsets[i][j][2];
                }
                else if (subsets[i][j][1] == "SEX")
                {
                    if (subsets[i][j][2] == "M")
                        Individuals[subsets[i][0][1]]->sex = 'M';
                    else if (subsets[i][j][2] == "F")
                        Individuals[subsets[i][0][1]]->sex = 'F';
                    else
                    {
                        string debug = Individuals[subsets[i][0][1]]->name + " has invalid sex value";
                        debugger.push_back(make_pair(getLine(i, j), debug));
                        // std::cout << Individuals[subsets[i][0][1]]->name << " has invalid sex value" << endl;
                        // throw invalid_argument("Invalid Sex Value for individual with id : " + subsets[i][0][1]);
                    }
                }

                // not done, turned out to be possibly redundant data
                else if (subsets[i][j][1] == "FAMS")
                {
                    // fm.push_back(nullptr);
                    // Individuals[subsets[i][0][1]]->mpFamilies.insert(make_pair('C', ))
                }

                /*
                Age - check for birth and death events and give range based on it
                Parents - check for famc tag and get husb and wife tags in the id of famc indicated
                */
                else if (subsets[i][j].size() == 2 && subsets[i][j][1] == "BIRT")
                {

                    /*
                    if we have a birth event we know it will have a date next,
                    since we don't follow a tree structure, where birth will have a child date
                    instead we push a tag onto a stack and when a date is encountered in next
                    iterations we check top of stack to know what event that date is linked to
                    and further pop it

                    an assumption made is that a date must have a related preceding event,
                    i.e., two dates can't be simultaneously present and must have an event
                    between them
                    */
                    event.push('B');
                }
                else if (subsets[i][j].size() == 2 && subsets[i][j][1] == "DEAT")
                {
                    // check comments for birth event
                    event.push('D');
                }

                // parse dates
                else if (subsets[i][j].size() >= 2 && subsets[i][j][1] == "DATE")
                {

                    // parse birth event
                    if (event.size() >= 1 && event.top() == 'B')
                    {

                        // in birthdates data structure insert date object linked to its individuals id
                        BirthDates.insert(make_pair(Individuals[subsets[i][0][1]]->id, new Date()));
                        Date BirthDate;
                        vector<string> date;
                        for (int m = 2; m < subsets[i][j].size(); m++)
                        {
                            date.push_back(subsets[i][j][m]);
                        }
                        BirthDate = parseDate(date);

                        // currently our print requirements require only year to be stored
                        BirthDates[Individuals[subsets[i][0][1]]->id]->year = BirthDate.year;

                        // BirthDates[Individuals[subsets[i][0][1]]->id]->month = BirthDate.month;
                        // BirthDates[Individuals[subsets[i][0][1]]->id]->date = BirthDate.date;
                        // cout << Individuals[subsets[i][0][1]]->name << endl;
                        // cout << "Birth date " << BirthDate.date << endl;
                        // cout << "Birth month " << BirthDate.month << endl;
                        // cout << "Birth year " << BirthDate.year << endl;
                        // cout << endl;

                        // at end of parsing date, pop the event from the stack
                        event.pop();
                    }

                    // parse death event
                    else if (event.size() >= 1 && event.top() == 'D')
                    {
                        // check comments in birth event
                        DeathDates.insert(make_pair(Individuals[subsets[i][0][1]]->id, new Date()));
                        Date DeathDate;
                        vector<string> date;
                        for (int m = 2; m < subsets[i][j].size(); m++)
                        {
                            date.push_back(subsets[i][j][m]);
                        }
                        DeathDate = parseDate(date);
                        DeathDates[Individuals[subsets[i][0][1]]->id]->year = DeathDate.year;

                        // DeathDate[Individuals[subsets[i][0][1]]->id]->month = DeathDate.month;
                        // DeathDate[Individuals[subsets[i][0][1]]->id]->date = DeathDate.date;
                        // cout << Individuals[subsets[i][0][1]]->name << endl;
                        // cout << "Death date " << DeathDate.date << endl;
                        // cout << "Death month " << DeathDate.month << endl;
                        // cout << "Death year " << DeathDate.year << endl;
                        // cout << endl;
                        event.pop();
                        // parse dates
                    }
                }
            }
        }
    }

    // parsing families
    for (int i = 0; i < subsets.size(); i++)
    {
        vector<Individual> children_in;
        stack<char> event;
        // check all blocks which are for families
        if (subsets[i][0].size() > 2 && subsets[i][0][2] == "FAM")
        {

            // in families data structure insert new object for family linked to family's id
            Families.insert(make_pair(subsets[i][0][1], new Family()));
            Families[subsets[i][0][1]]->id = subsets[i][0][1];
            for (int j = 0; j < subsets[i].size(); j++)
            {

                /*
                the husband field in the family's class is linked to the individual with the husband id
                similarly wife field is linked to individual with the id
                children follow the same only that they are stored in a vector and that vector is assigned to children field
                */
                if (subsets[i][j][1] == "HUSB")
                {
                    Families[subsets[i][0][1]]->husband = *Individuals[subsets[i][j][2]];
                }
                else if (subsets[i][j][1] == "WIFE")
                {
                    Families[subsets[i][0][1]]->wife = *Individuals[subsets[i][j][2]];
                }
                else if (subsets[i][j][1] == "CHIL")
                {
                    children_in.push_back(*Individuals[subsets[i][j][2]]);
                    Families[subsets[i][0][1]]->children = children_in;
                }
                else if (subsets[i][j][1] == "MARR")
                {
                    event.push('M');
                    event.push('M');
                }
                else if (subsets[i][j][1] == "DATE")
                {
                    if (event.size() >= 1 && event.top() == 'M')
                    {
                        Date marr;
                        vector<string> date;
                        for (int m = 2; m < subsets[i][j].size(); m++)
                        {
                            date.push_back(subsets[i][j][m]);
                        }
                        marr = parseDate(date);
                        Families[subsets[i][0][1]]->MarrDate = marr;
                        event.pop();
                    }
                }
                else if (subsets[i][j][1] == "PLAC")
                {
                    if (event.size() >= 1 && event.top() == 'M')
                    {
                        string plac;
                        for (int m = 2; m < subsets[i][j].size(); m++)
                        {
                            plac += (subsets[i][j][m]) + " ";
                        }
                        Families[subsets[i][0][1]]->place = plac;
                    }
                }
            }
        }
    }

    /*
    To get parents of an individual:
        1. All individuals must be created
        2. All families must be created
        3. Fields in family must be linked to the respective individuals
        4. Only then we can link husband of a family to father of an individual and wife of a family to mother of an individual
    To do the above we must first parse all individuals and all families and then reparse individuals to get parents from families class
    */

    // reparse individuals after family details have been parsed
    for (int i = 0; i < subsets.size(); i++)
    {
        if (subsets[i][0].size() > 2 && subsets[i][0][2] == "INDI")
        {
            for (int j = 0; j < subsets[i].size(); j++)
            {
                // we check for FAMC tag which indicates that the individual was a child in the family with tag indicated with FAMC
                if (subsets[i][j].size() == 3 && subsets[i][j][1] == "FAMC")
                {
                    // Family family = *Families[subsets[i][j][2]];
                    // cout << &family.husband << endl;
                    if (&Families[subsets[i][j][2]]->husband)
                        Individuals[subsets[i][0][1]]->father = &Families[subsets[i][j][2]]->husband;
                    if (&Families[subsets[i][j][2]]->wife)
                        Individuals[subsets[i][0][1]]->mother = &Families[subsets[i][j][2]]->wife;
                }
            }
        }
    }
    if (mode == 'f')
    {
        std::cout << "----------------------------------------- " << endl;
        std::cout << endl;

        // print all individual details
        for (auto &x : Individuals)
        {
            std::cout << "Id of Individual        : " << x.second->id << endl;
            std::cout << "Name of Individual      : " << x.second->name << endl;
            // std::cout << "Surname of Individual   : " << x.second->srname << endl;
            // std::cout << "GivenName of Individual : " << x.second->givname << endl;
            if (x.second->sex == 'M')
                std::cout << "Sex of Individual       : Male " << endl;
            else if (x.second->sex == 'F')
                std::cout << "Sex of Individual       : Female " << endl;
            else
                std::cout << "Sex of Individual       : Other " << endl;

            // if (BirthDates.find(x.second->id) != BirthDates.end())
            //     std::cout << "DOB of Individual       : " << BirthDates[x.second->id]->year << endl;
            // else
            //     std::cout << "DOB of Individual       : No record" << endl;

            // if (DeathDates.find(x.second->id) != DeathDates.end())
            //     std::cout << "DOD of Individual       : " << DeathDates[x.second->id]->year << endl;
            // else
            //     std::cout << "DOD of Individual       : No record" << endl;

            // we display age in form of [YYYY - YYYY] (BirthYear - DeathYear) followed by age of individual at time of death
            // months are not accounted for and a crude subtraction is followed
            if (BirthDates.find(x.second->id) != BirthDates.end() && DeathDates.find(x.second->id) != DeathDates.end())
            {
                std::cout << "Lived during            : [" << BirthDates[x.second->id]->year << " - " << DeathDates[x.second->id]->year << "]"
                          << " and died at age of " << (DeathDates[x.second->id]->year - BirthDates[x.second->id]->year) << endl;
            }
            else if (BirthDates.find(x.second->id) == BirthDates.end() && DeathDates.find(x.second->id) == DeathDates.end())
            {
                std::cout << "Lived during            : No record " << endl;
            }
            else
            {
                if (BirthDates.find(x.second->id) != BirthDates.end())
                {
                    std::cout << "Lived during            : [" << BirthDates[x.second->id]->year << " - "
                              << "]" << endl;
                }
                else
                {
                    std::cout << "Lived during            : ["
                              << " - " << DeathDates[x.second->id]->year << "]" << endl;
                }
            }

            // parents of the individual are printed with son, daughter, child also accounted for
            if (x.second->father && x.second->mother)
            {
                if (x.second->sex == 'M')
                    std::cout << "Son of                  : " << x.second->father->name << " and " << x.second->mother->name << endl;
                else if (x.second->sex == 'F')
                    std::cout << "Daughter of             : " << x.second->father->name << " and " << x.second->mother->name << endl;
                else
                    std::cout << "Child of                : " << x.second->father->name << " and " << x.second->mother->name << endl;
            }
            else if (!x.second->father && !x.second->mother)
            {
                if (x.second->sex == 'M')
                    std::cout << "Son of                  : No record" << endl;

                else if (x.second->sex == 'F')
                    std::cout << "Daughter of             : No record" << endl;

                else
                    std::cout << "Child of                : No record" << endl;
            }
            else
            {
                if (x.second->father)
                {
                    if (x.second->sex == 'M')
                        std::cout << "Son of                  : " << x.second->father->name << endl;
                    else if (x.second->sex == 'F')
                        std::cout << "Daughter of                  : " << x.second->father->name << endl;
                    else
                        std::cout << "Child of                  : " << x.second->father->name << endl;
                }
                else if (x.second->mother)
                {
                    if (x.second->sex == 'M')
                        std::cout << "Son of                  : " << x.second->mother->name << endl;
                    else if (x.second->sex == 'F')
                        std::cout << "Daughter of                  : " << x.second->mother->name << endl;
                    else
                        std::cout << "Child of                  : " << x.second->mother->name << endl;
                }
            }

            std::cout << endl;
            std::cout << " *** " << endl;
            std::cout << endl;
        }
        std::cout << "Total number of individuals : " << Individuals.size() << endl;
        std::cout << endl
                  << endl;
        std::cout << "----------------------------------" << endl;
        std::cout << endl;
        std::cout << "Family details : " << endl;
        std::cout << endl;

        // print all families's details
        for (auto &x : Families)
        {
            std::cout << "Id of family              : " << x.second->id << endl;
            std::cout << "Husband in the family     : " << x.second->husband.name << endl;
            std::cout << "Wife in the family        : " << x.second->wife.name << endl;
            vector<Individual> ListChildren = x.second->children;
            std::cout << "Children in family (";
            std::cout << std::setw(2) << std::setfill('0') << x.second->children.size();
            std::cout << ")   : ";

            int NumberChildren = x.second->children.size();

            for (auto &c : ListChildren)
            {
                if (NumberChildren-- == 1)
                    std::cout << c.name;
                else
                    std::cout << c.name << ", ";
            }

            if (x.second->children.size() == 0)
            {
                cout << "None" << endl;
            }
            else
            {
                cout << endl;
            }
            if (x.second->MarrDate.year == 0)
            {
                std::cout << "Date of marriage          : "
                          << "No record" << endl;
            }
            else
            {
                std::cout << "Date of marriage          : " << x.second->MarrDate.year << endl;
            }
            std::cout << "Place of marriage         : " << x.second->place << endl;

            std::cout << endl;
            std::cout << " *** " << endl;
            std::cout << endl;
        }
        std::cout << "Total number of Families  : " << Families.size() << endl;
        std::cout << endl;
        std::cout << "----------------------------------" << endl;

        // at end of parsing we also display various deviations in the ged file format
        std::cout << "Printing deviations info : " << endl;
        std::cout << endl;
        for (int i = 0; i < debugger.size(); i++)
        {
            std::cout << i + 1 << ". "
                      << "Line " << debugger[i].first << " : " << debugger[i].second << endl;
        }
        std::cout << endl;
    }

    if (mode == 'g')
    {
        string gname, choice;
        std::cout << "Print for all or one individual?" << endl;
        cin >> choice;
        if (choice == "one")
        {
            std::cout << "Enter name of individual : " << endl;

            std::getline(std::cin >> std::ws, gname);
            fetchParent(parseName(gname)->id);
            for (int i = fatherList.size() - 1; i > -1; i--)
            {
                cout << i << ". " << fatherList[i] << " ";
            }
            cout << endl;
        }
        else if (choice == "all")
        {
            int j = 1;
            for (auto const &x : Individuals)
            {
                gname = x.second->id;
                fatherList.clear();
                fetchParent(gname);
                for (int i = fatherList.size() - 1; i > -1; i--)
                {
                    if (fatherList.size() == 1)
                        cout << j++ << ". " << fatherList[i] << " $";
                    else if (i == fatherList.size() - 1 && (fatherList.size()) == 2)
                        cout << j++ << ". " << fatherList[i] << " <- ";
                    else if (i == fatherList.size() - 1)
                        cout << j++ << ". " << fatherList[i];

                    else if (i == 0)
                        cout << fatherList[i] << " $";
                    else
                        cout << " <- " << fatherList[i];
                }
                cout << endl;
            }
        }
    }
    cout << endl;

    // close the gedcom file
    in.close();

    return 0;
}
