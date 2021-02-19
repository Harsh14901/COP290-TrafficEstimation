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
        bool is_standalone;

        Argument(string field, string short_field,string default_value,bool compulsory,bool is_standalone);
    };
    map<string, string> input_arguments;
    vector<Argument> arguments;

    void set_argument(string field, string short_field, string default_value, bool compulsory,bool is_standalone);
    void set_argument(string field);
    void set_argument(string field,string short_field,string default_value);
    void set_standalone_argument(string field,string short_field);
    bool get_bool_argument_value(string field);

    Argument* get_argument_field(string field);


    Argument* search_argument(string arg);
    string get_argument_value(string field);

    bool parse_arguments(int argc, char *argv[]);
};

#endif
