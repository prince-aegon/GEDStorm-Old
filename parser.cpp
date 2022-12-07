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
    Name
    Address structure
*/
class Submitter
{
public:
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
map<string, Submitter *> Submitters;

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
    vector<string> submitterLines;
    stack<string> curr;
    curr.push("NULL");
    while (in.eof() == 0)
    {

        number_of_lines++;
        getline(in, s);
        string submitterId;
        vector<string> submSplit = splitString(s, " ");
        if (submSplit.size() >= 3 && submSplit[0] == "0" && submSplit[2] == "SUBM")
        {
            submitterLines.push_back(submSplit[1]);
            submitterId = submSplit[1];
            Submitters.insert(make_pair(submSplit[1], new Submitter()));
            curr.push("SUBM");
        }
        if (curr.top() == "SUBM")
        {
            getline(in, s);
            vector<string> nameSplit = splitString(s, " ");
            if (nameSplit[0] == "1" && nameSplit[1] == "NAME")
            {
                Submitters[submitterId]->submitterName = nameSplit[2];
            }
            curr.pop();
        }
        // working on comments
        if (s[0] == '/' && s[1] == '/')
        {
            string commentString = s.substr(3, s.length() - 2);
            cTYPE.commentType = "new-line";
            cTYPE.commentLength = number_of_lines;
            commentCheck(commentString, cTYPE);
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
                        commentCheck(commentString, cTYPE);
                    }
                }
            }
        }
    }
    // for (int i = 0; i < submitterLines.size(); i++)
    // {
    //     Submitters.insert(make_pair(submitterLines[i], new Submitter()));
    // }
    int i = 0;
    for (auto const &x : Submitters)
    {
        cout << x.first << " : " << x.second->submitterName << endl;
    }

    cout << "Number of lines...:" << number_of_lines << endl;

    in.close();

    return 0;
}