#include "arg_parser.hpp"
#include <iostream>
#include <stdexcept>

ArgParser::Option::Option(char s, const std::string &l, const std::string &d,
                          bool h, const std::string &def)
    : short_name(s), long_name(l), description(d), has_arg(h),
      default_value(def), is_set(false), value(def) {}

void ArgParser::Option::set_flag() { is_set = true; }

void ArgParser::Option::set_value(const std::string &val) {
  is_set = true;
  value = val;
}

void ArgParser::add_option(char short_name, const std::string &long_name,
                           const std::string &desc, bool has_arg,
                           const std::string &default_value) {
  std::string key = long_name;
  if (key.empty()) {
    if (short_name == 0)
      throw std::invalid_argument("Option must have at least one name");
    key = std::string(1, short_name);
  }

  if (long_options.find(key) != long_options.end())
    throw std::invalid_argument("Option with long name '" + key +
                                "' already exists");
  if (short_name != 0 && short_options.find(short_name) != short_options.end())
    throw std::invalid_argument("Option with short name '-" +
                                std::string(1, short_name) +
                                "' already exists");

  auto opt =
      std::make_unique<Option>(short_name, key, desc, has_arg, default_value);
  Option *ptr = opt.get();
  long_options.emplace(key, std::move(opt));
  if (short_name != 0)
    short_options.emplace(short_name, ptr);
  options_order.push_back(ptr);
}

void ArgParser::add_flag(char short_name, const std::string &long_name,
                         const std::string &desc) {
  add_option(short_name, long_name, desc, false);
}

void ArgParser::parse(int argc, char *argv[]) {
  positional_args.clear();

  for (auto &[key, opt] : long_options) {
    opt->is_set = false;
    opt->value = opt->default_value;
  }

  int i = 1;
  while (i < argc) {
    std::string token = argv[i];

    if (token == "--") {
      ++i;
      while (i < argc) {
        positional_args.push_back(argv[i]);
        ++i;
      }
      break;
    } else if (token.size() >= 2 && token[0] == '-' && token[1] == '-') {
      parse_long_option(token, i, argc, argv);
    } else if (token.size() >= 2 && token[0] == '-') {
      parse_short_option(token, i, argc, argv);
    } else {
      positional_args.push_back(token);
      ++i;
    }
  }
}

bool ArgParser::is_set(const std::string &name) const {
  auto it = long_options.find(name);
  if (it == long_options.end())
    throw std::runtime_error("Unknown option: " + name);
  return it->second->is_set;
}

std::string ArgParser::value_of(const std::string &name) const {
  auto it = long_options.find(name);
  if (it == long_options.end())
    throw std::runtime_error("Unknown option: " + name);
  const Option *opt = it->second.get();
  if (!opt->has_arg)
    throw std::runtime_error("Option '" + name + "' does not take an argument");
  return opt->value;
}

const std::vector<std::string> &ArgParser::positional() const {
  return positional_args;
}

void ArgParser::print_help(std::ostream &os) const {
  os << "Options:\n";
  for (const Option *opt : options_order) {
    os << "  ";
    if (opt->short_name != 0)
      os << "-" << opt->short_name;
    if (opt->short_name != 0 && !opt->long_name.empty())
      os << ", ";
    if (!opt->long_name.empty())
      os << "--" << opt->long_name;
    os << "\n\t" << opt->description;
    if (opt->has_arg)
      os << " (default: \"" << opt->default_value << "\")";
    os << "\n";
  }
}

void ArgParser::parse_long_option(const std::string &token, int &i, int argc,
                                  char *argv[]) {
  size_t eq_pos = token.find('=');
  std::string name, arg_value;
  bool has_eq = (eq_pos != std::string::npos);

  if (has_eq) {
    name = token.substr(2, eq_pos - 2);
    arg_value = token.substr(eq_pos + 1);
  } else {
    name = token.substr(2);
  }

  auto it = long_options.find(name);
  if (it == long_options.end())
    throw std::runtime_error("Unknown option: " + token);

  Option *opt = it->second.get();
  if (opt->has_arg) {
    if (has_eq) {
      opt->set_value(arg_value);
      ++i;
    } else {
      if (i + 1 < argc) {
        opt->set_value(argv[i + 1]);
        i += 2;
      } else {
        throw std::runtime_error("Option '" + token + "' requires an argument");
      }
    }
  } else {
    if (has_eq)
      throw std::runtime_error("Option '" + token +
                               "' does not take an argument");
    opt->set_flag();
    ++i;
  }
}

void ArgParser::parse_short_option(const std::string &token, int &i, int argc,
                                   char *argv[]) {
  std::string opt_str = token.substr(1);
  size_t eq_pos = opt_str.find('=');

  if (eq_pos != std::string::npos) {
    if (opt_str.empty() || eq_pos != 1)
      throw std::runtime_error("Invalid short option format: " + token);
    char c = opt_str[0];
    auto it = short_options.find(c);
    if (it == short_options.end())
      throw std::runtime_error("Unknown short option: -" + std::string(1, c));
    Option *opt = it->second;
    if (!opt->has_arg)
      throw std::runtime_error("Option '-" + std::string(1, c) +
                               "' does not take an argument");
    std::string arg = opt_str.substr(eq_pos + 1);
    opt->set_value(arg);
    ++i;
    return;
  }

  bool consumed_next = false;
  for (size_t j = 0; j < opt_str.size(); ++j) {
    char c = opt_str[j];
    auto it = short_options.find(c);
    if (it == short_options.end())
      throw std::runtime_error("Unknown short option: -" + std::string(1, c));
    Option *opt = it->second;

    if (opt->has_arg) {
      if (j + 1 < opt_str.size()) {
        std::string arg = opt_str.substr(j + 1);
        opt->set_value(arg);
        break;
      } else {
        if (i + 1 < argc) {
          opt->set_value(argv[i + 1]);
          i += 2;
          consumed_next = true;
          break;
        } else {
          throw std::runtime_error("Option '-" + std::string(1, c) +
                                   "' requires an argument");
        }
      }
    } else {
      opt->set_flag();
    }
  }

  if (consumed_next)
    return;
  ++i;
}

std::ostream &operator<<(std::ostream &os, const ArgParser &p) {
  for (const auto &[k, opt] : p.long_options) {
    os << opt->long_name << ' ' << opt->value << '\n';
  }
  for (const auto &[k, opt] : p.short_options) {
    os << opt->short_name << ' ' << opt->value << '\n';
  }
  return os;
}
