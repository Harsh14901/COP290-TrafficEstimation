#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <map>
#include <bits/stdc++.h>

using namespace std;

class ArgParser
{
public:
    class Argument
    {
    public:
        bool compulsory;
        string field;
        string short_field;
        string default_value;
        string value;
        

        Argument(string field, bool compulsory);
    };
    map<string, string> input_arguments;
    vector<Argument> arguments;

    void set_argument(string field);
    void set_argument(string field, bool compulsory);

    bool search_argument(string arg);
    string get_argument_value(string field);

    bool parse_arguments(int argc, char *argv[]);
};

#endif
