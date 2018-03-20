// g++ -o json json.cpp -lboost_program_options
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <string>

using namespace boost::property_tree;
using namespace boost::program_options;
void parse_rec(ptree &pt, std::string parent);

int main(int argc, char *argv[]) {
    options_description opt("options");
    opt.add_options()
        ("help,h", "display help")
        ("key,k", value<std::string>(), "get value of key");

    variables_map argmap;
    auto const res = parse_command_line(argc, argv, opt);
    store(res, argmap);
    notify(argmap);

    auto const filenames = collect_unrecognized(res.options, include_positional);
    if (argmap.count("help") || (filenames.size() != 1)) {
        std::cerr << argmap.size() << opt << std::endl;
        return 0;
    }

    std::string filename = filenames.at(0);
    ptree pt;
    try {
        read_json(filename, pt);
    } catch (...) {
        return 0;
    }
    if (argmap.count("key")) {
        std::string key = argmap["key"].as<std::string>();
        boost::optional<std::string> str = pt.get_optional<std::string>(key);
        if (str) {
            std::cout << str.get() << std::endl;
            return 1;
        }
    } else {
        parse_rec(pt, "");
    }
    return 0;
}

void parse_rec(ptree &pt, std::string parent) {
    for (auto it = pt.begin(); it != pt.end(); ++it) {
        std::string key = it->first;
        std::string val = it->second.get_value<std::string>();
        std::string me = parent + "." + key;
        if (parent.empty()) me = key;
        if (key.empty()) me = parent + "[]";
        if (val.empty()) {
            parse_rec(it->second, me);
        } else {
            std::cout << me << " = " << val << std::endl;
        }
    }
}
