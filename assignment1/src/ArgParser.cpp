#include <ArgParser.hpp>

// using namespace ArgParser;

ArgParser::Argument::Argument(string field, bool compulsory)
{
    this->field = field;
    this->compulsory = compulsory;
}

void ArgParser::set_argument(string field, bool compulsory)
{
    arguments.push_back(Argument(field, compulsory));
}

void ArgParser::set_argument(string field)
{
    set_argument(field, false);
}

bool ArgParser::search_argument(string arg)
{
    if (arg.substr(0, 2) != "--")
    {
        return false;
    }
    for (auto i = 0u; i < arguments.size(); i++)
    {
        if (("--" + arguments[i].field) == arg)
        {
            return true;
        }
    }
    return false;
}

string ArgParser::get_argument_value(string field)
{
    if (input_arguments.find(field) == input_arguments.end())
    {
        return "";
    }

    return input_arguments.find(field)->second;
}

bool ArgParser::parse_arguments(int argc, char *argv[])
{
    try
    {
        if (argc % 2 == 0)
        {
            return false;
        }
        for (int i = 1; i < argc; i++)
        {
            string temp = argv[i];
            if (search_argument(temp))
            {
                input_arguments.insert(pair<string, string>(temp.substr(2), argv[++i]));
            }
            else
            { // Update this
                return false;
            }
        }
        return argc==1?false:true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

