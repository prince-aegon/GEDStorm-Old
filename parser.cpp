#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stack>
#include <regex>
#include "address.h"
using namespace std;
// #define FILE_NAME "/home/sarthak/projects/gedcom/GEDCOM-Files/submitter.ged"
#define FILE_NAME "/home/sarthak/projects/gedcom/GEDCOM-Files/Shakespeare.ged"
// #define FILE_NAME "/home/sarthak/projects/gedcom/GEDCOM-Files/The English and British Kings and Queens.ged"

#define regex_cout "\w*(?<!:)std::cout"

typedef enum cSet
{
    ANSEL = 0,
    UTF8 = 1,
    UTF16 = 2,
    UNICODE = 3,
    ASCII = 4
} CharSet;

typedef enum evSet
{
    BIRT = 0,
    MARR = 1,
    DEAT = 2
} EventSet;

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
class Header
{
public:
    char source[256];
    string gedcomVersion;
    string formType;
    CharSet encoding;
    LangSet language;
    CorpRecord *corp;
};

class Comment
{
public:
    string commentType;
    int commentLength;
};
class Family;
class Individual
{
public:
    string id;
    string name;
    string srname;
    string givname;
    char sex;

    int birthYear;
    int deathYear;
    // stores all the families inividual is part of based on c or s
    map<char, vector<Family>> mpFamilies;
};

class Family
{
public:
    string id;
    Individual husband;
    Individual wife;
    vector<Individual> children;
};

class Date
{
public:
    int date;
    string month;
    int year;
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
vector<vector<vector<string>>> subsets;

// vector of all strings
vector<vector<string>> all_lines;

// data structure for individuals
map<string, Individual *> Individuals;

// data structure for families
map<string, Family *> Families;

// debugger details
vector<pair<int, string>> debugger;

void commentCheck(string s, Comment comment)
{
    std::cout << "Type    : " << comment.commentType << endl;
    std::cout << "comment : " << s << endl;
    std::cout << "line no : " << comment.commentLength << endl
              << endl;
}
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
    if (sanitised == "")
        return make_pair(0, sanitised);
    else
        return make_pair(1, sanitised);
}

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

Date parseDate(vector<string> date)
{
    Date nDate;
    nDate.date = 0;
    nDate.month = "None";
    nDate.year = 0;
    // regex to check if first is date
    std::regex reDate("^[0-9]{2}$");
    std::regex reYear("^[0-9]{4}$");
    // for (int i = 0; i < date.size(); i++)
    //     cout << date[i] << " ";
    // cout << endl;
    if (std::regex_match(date[0], reDate) && std::regex_match(date[date.size() - 1], reYear))
    {
        cout << "here" << endl;
        nDate.date = stoi(date[0]);
        nDate.year = stoi(date[date.size() - 1]);
    }
    return nDate;
}

int main(int argc, char *argv[])
{

    // implement custom flags
    // -d -> debugger mode
    // -f -> full mode
    // -s -> short mode

    char mode;
    // std::cout << argc << argv[0] << argv[1] << endl;
    if (argv[1] == "-d")
        std::cout << "debug mode" << endl;
    char c, fn[10];
    string s;
    Comment cTYPE;

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
    std::cout << "Parsing Start" << endl;
    std::cout << "Printing Comments..." << endl;
    std::cout << "-------------------------------" << endl;

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

    std::cout
        << "Number of lines...:" << number_of_lines << endl;
    std::cout << endl;
    for (int i = 0; i < all_lines.size();)
    {
        if (all_lines[i][0] == "0")
        {
            vector<vector<string>> temp;
            temp.push_back(all_lines[i]);
            i++;
            while ((i + 1) < all_lines.size() && all_lines[i][0] != "0")
            {
                temp.push_back(all_lines[i]);
                i++;
            }
            subsets.push_back(temp);
        }
    }

    // print all the blocks
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
                    throw invalid_argument("Invalid Encoding");
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
    std::cout << "-------------------------------" << endl;
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

    // parse all individuals
    std::cout << endl;
    std::cout << "----------------------------------------- " << endl;
    std::cout << endl;
    std::cout << "Individual details : " << endl;
    std::cout << endl;
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
            Individuals.insert(make_pair(subsets[i][0][1], new Individual()));
            Individuals[subsets[i][0][1]]->id = subsets[i][0][1];
            for (int j = 0; j < subsets[i].size(); j++)
            {
                if (subsets[i][j][1] == "NAME")
                {
                    string name = "";
                    for (int k = 2; k < int(subsets[i][j].size()); k++)
                    {
                        // sanitise name
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
                    event.push('B');
                }
                else if (subsets[i][j].size() >= 2 && subsets[i][j][1] == "DATE")
                {
                    if (event.size() >= 1 && event.top() == 'B')
                    {
                        Date BirthDate;
                        vector<string> date;
                        for (int m = 2; m < subsets[i][j].size(); m++)
                        {
                            date.push_back(subsets[i][j][m]);
                        }
                        BirthDate = parseDate(date);
                        event.pop();
                        // parse dates
                    }
                }
            }
        }
    }
    for (auto &x : Individuals)
    {
        std::cout << "Id of Individual        : " << x.second->id << endl;
        std::cout << "Name of Individual      : " << x.second->name << endl;
        std::cout << "Surname of Individual   : " << x.second->srname << endl;
        std::cout << "GivenName of Individual : " << x.second->givname << endl;
        std::cout << "Sex of Individual       : " << x.second->sex << endl;
        std::cout << endl;
    }
    std::cout << "Total number of individuals : " << Individuals.size() << endl;
    std::cout << endl;
    std::cout << "----------------------------------------- " << endl;
    std::cout << endl;
    std::cout << "Family details : " << endl;
    std::cout << endl;
    // parsing families
    for (int i = 0; i < subsets.size(); i++)
    {
        vector<Individual> children_in;
        if (subsets[i][0].size() > 2 && subsets[i][0][2] == "FAM")
        {
            Families.insert(make_pair(subsets[i][0][1], new Family()));
            Families[subsets[i][0][1]]->id = subsets[i][0][1];
            for (int j = 0; j < subsets[i].size(); j++)
            {
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
            }
        }
    }
    for (auto &x : Families)
    {
        std::cout << "Id of family              : " << x.second->id << endl;
        std::cout << "Husband in the family     : " << x.second->husband.name << endl;
        std::cout << "Wife in the family family : " << x.second->wife.name << endl;
        vector<Individual> ListChildren = x.second->children;
        std::cout << "Children in the family    : ";
        int NumberChildren = x.second->children.size();
        for (auto &c : ListChildren)
        {
            if (NumberChildren-- == 1)
                std::cout << c.name;
            else
                std::cout << c.name << " ,";
        }
        std::cout << endl;
        std::cout << endl;
    }
    std::cout << "Total number of Families  : " << Families.size() << endl;
    std::cout << "----------------------------------" << endl;
    std::cout << "Printing debugger info : " << endl;
    std::cout << endl;
    for (int i = 0; i < debugger.size(); i++)
    {
        std::cout << i + 1 << ". "
                  << "Line " << debugger[i].first << " : " << debugger[i].second << endl;
    }
    std::cout << endl;

    in.close();

    return 0;
}
