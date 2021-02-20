#include <arg_parser.hpp>

ArgParser arg_parser = ArgParser();

ArgParser::Argument::Argument(string field, string short_field,
                              string default_value, bool compulsory,
                              bool is_standalone) {
  this->field = field;
  this->compulsory = compulsory;
  this->short_field = short_field;
  this->default_value = default_value;
  this->is_standalone = is_standalone;
}

void ArgParser::set_argument(string field, string short_field,
                             string default_value, bool compulsory,
                             bool is_standalone) {
  arguments.push_back(
      Argument(field, short_field, default_value, compulsory, is_standalone));
}

void ArgParser::set_argument(string field) {
  set_argument(field, "", "", false, false);
}

void ArgParser::set_standalone_argument(string field, string short_field) {
  set_argument(field, short_field, "", false, true);
}

void ArgParser::set_argument(string field, string short_field,
                             string default_value) {
  set_argument(field, short_field, default_value, false, false);
}

ArgParser::Argument *ArgParser::search_argument(string arg) {
  bool is_short = false;
  if (arg.substr(0, 2) != "--") {
    if (arg.substr(0, 1) == "-") {
      is_short = true;
    } else {
      return NULL;
    }
  }
  for (auto i = 0u; i < arguments.size(); i++) {
    if (!is_short && ("--" + arguments[i].field) == arg) {
      return &arguments[i];
    } else if (is_short && ("-" + arguments[i].short_field) == arg &&
               arguments[i].short_field != "") {
      return &arguments[i];
    }
  }
  return NULL;
}

ArgParser::Argument *ArgParser::get_argument_field(string field) {
  for (auto i = 0u; i < arguments.size(); i++) {
    if (arguments[i].field == field) {
      return &arguments[i];
    }
  }
  return NULL;
}

string ArgParser::get_argument_value(string field) {
  if (input_arguments.find(field) == input_arguments.end()) {
    Argument *arg = get_argument_field(field);
    if (arg == nullptr) {
      return "";
    }
    return arg->default_value;
  }

  return input_arguments.find(field)->second;
}

bool ArgParser::get_bool_argument_value(string field) {
  Argument *arg = get_argument_field(field);
  if (!arg->is_standalone) {
    return false;
  }
  if (input_arguments.find(field) == input_arguments.end()) {
    return false;
  }
  return true;
}

bool ArgParser::parse_arguments(int argc, char *argv[]) {
  if (argc == 2 &&
      (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
    return false;
  }
  try {
    for (int i = 1; i < argc; i++) {
      string temp = argv[i];
      Argument *arg = search_argument(temp);
      if (arg == nullptr) {
        return false;
      }
      if (arg->is_standalone) {
        input_arguments.insert(pair<string, string>(arg->field, "true"));
      } else {
        input_arguments.insert(pair<string, string>(arg->field, argv[++i]));
      }
    }

    // Check for compulsory arguments
    for (auto i = 0u; i < arguments.size(); i++) {
      if (arguments[i].compulsory) {
        cout << arguments[i].field << endl;
        if (input_arguments.find(arguments[i].field) == input_arguments.end()) {
          return false;
        }
      }
    }
    cout << "OK" << endl;
    return true;
  } catch (const std::exception &e) {
    return false;
  }
}
