#include <iostream>
#include <fstream>

using namespace std;
#define FILE_NAME "Basic1.ged"

class Comment
{
public:
    string commentType;
    int commentLength;
};
class Header
{
public:
    char source[256];
    float gedcomVersion;
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