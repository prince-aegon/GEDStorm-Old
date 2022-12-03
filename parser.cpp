#include <iostream>
#include <fstream>
#include <map>
#include "address.h"
using namespace std;
#define FILE_NAME "Basic.ged"

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

// Submitter Record
class Submitter
{
public:
    char submitterName[64];
    Address *addr;
    map<string, string> details;
};

// Source Record
class SourceRecord
{
public:
    EventSet events[3];
    string date;
    string relInfo1[4];
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
    while (in.eof() == 0)
    {
        number_of_lines++;
        getline(in, s);

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

    cout << "Number of lines...:" << number_of_lines << endl;

    in.close();

    return 0;
}