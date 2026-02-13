#ifndef _ARG_PARSER_HPP_
#define _ARG_PARSER_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class ArgParser {
private:
  struct Option {
    char short_name;
    std::string long_name;
    std::string description;
    bool has_arg;
    std::string default_value;
    bool is_set;
    std::string value;

    Option(char s, const std::string &l, const std::string &d, bool h,
           const std::string &def);
    void set_flag();
    void set_value(const std::string &val);
  };

  std::unordered_map<std::string, std::unique_ptr<Option>> long_options;
  std::unordered_map<char, Option *> short_options;
  std::vector<Option *> options_order;
  std::vector<std::string> positional_args;

  void parse_long_option(const std::string &token, int &i, int argc,
                         char *argv[]);
  void parse_short_option(const std::string &token, int &i, int argc,
                          char *argv[]);

public:
  ArgParser() = default;

  void add_option(char short_name, const std::string &long_name,
                  const std::string &desc, bool has_arg = false,
                  const std::string &default_value = "");
  void add_flag(char short_name, const std::string &long_name,
                const std::string &desc);

  void parse(int argc, char *argv[]);

  bool is_set(const std::string &name) const;
  std::string value_of(const std::string &name) const;
  const std::vector<std::string> &positional() const;

  void print_help(std::ostream &) const;
  friend std::ostream &operator<<(std::ostream &, const ArgParser &);
};

#endif