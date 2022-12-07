#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <stack>
#include "address.h"
using namespace std;
#define FILE_NAME "/home/sarthak/projects/gedcom/GEDCOM-Files/submitter.ged"

typedef enum cSet
{
    ANSEL,
    UTF8,
    UNICODE,
    ASCII
} CharSet;

typedef enum evSet
{
    BIRT,
    MARR,
    DEAT
} EventSet;

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
    float gedcomVersion;
    CharSet encoding;
    SourceRecord *src;
    Submitter *submitter;
};

class Comment
{
public:
    string commentType;
    int commentLength;
};

/*
map : lines
    stores all lines for a particular tag
*/
map<string, vector<vector<string>>> lines;

// stores objects of submitter class with id as key
map<string, Submitter *> Submitters;

// 2d vector to help in lines
vector<vector<string>> submitterLines;

// stack having current parser tag and id of current tag
stack<pair<string, string>> curr;
vector<vector<vector<string>>> subsets;
vector<vector<string>> all_lines;

void commentCheck(string s, Comment comment)
{
    cout << "Type    : " << comment.commentType << endl;
    cout << "comment : " << s << endl;
    cout << "line no : " << comment.commentLength << endl
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
int main()
{
    char c, fn[10];
    string s;
    Comment cTYPE;

    if (FILE_NAME == "")
    {
        cout << "Enter the file name....:";
        cin >> fn;
    }
    ifstream in(FILE_NAME);
    if (!in)
    {
        cout << "Error! File Does not Exist";
        return 0;
    }
    int number_of_lines = 0;
    curr.push(make_pair("NULL", "@00@"));

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
    for (int i = 0; i < subsets.size(); i++)
    {
        for (int j = 0; j < subsets[i].size(); j++)
        {
            for (int k = 0; k < subsets[i][j].size(); k++)
            {
                cout << subsets[i][j][k] << " ";
            }
            cout << endl;
        }
        cout << endl;
        cout << "----------------------" << endl;
        cout << endl;
    }
    // print various functionalities
    // int i = 0;
    // for (auto const &x : Submitters)
    // {
    //     cout << x.first << " : " << x.second->submitterName << endl;
    // }
    // cout << endl;
    // for (auto const &x : lines)
    // {
    //     vector<vector<string>> slines = x.second;
    //     cout << x.first << endl;
    //     for (int i = 0; i < slines.size(); i++)
    //     {
    //         for (int j = 0; j < slines[i].size(); j++)
    //             cout << slines[i][j] << " ";
    //         cout << endl;
    //     }
    // }
    cout << "Number of lines...:" << number_of_lines << endl;

    in.close();

    return 0;
}