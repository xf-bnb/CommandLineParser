#include <iostream>

#include "../include/cmd_parser.h"


void test_regex(const std::vector<std::string>& strs, const std::string& reg_text)
{
    std::regex reg(reg_text);
    for (auto s : strs)
        std::cout << s << ": " << std::regex_match(s, reg) << std::endl;
}


int main()
{
    // test_regex({ "true", "True", "TRUE", "tRue", "true|false", "truef", "False", "false", "fAlse", "Frank" },
    //           "[Tt]rue|[Ff]alse|TRUE|FALSE");

    // test_regex({ "0", "01", "123", "-123", "+123", "123.45", "1", "a", "", "123a" },
    //            "0|[1-9][1-9]*");

    // test_regex({ "0", "01", "123", "-123", "+123", "123.45", "1", "-1", "-", "-0", "123a", "123+45", "+", "+0", "-01" },
    //            "[+-]?(0|[1-9][1-9]*)");


    test_regex({ "0", "01", "123", "-123", "+123", "123.45", "1", "-1", "-1.02", "-0", "123a", "123+45", "+2.003", "+0", "-01", "+1.2", "-2.7", "0.8.7", "1.2.3" },
               "[+-]?(0|[1-9][1-9]*)([.][0-9]+)?");

    const char* const argv[] = { "a", "1", "b", "7" };

    xf::cmd::Parser parser;
    
    xf::cmd::Parser::result_t r = parser.Parse(argv, 1, 3);

    const auto& g = r.get();

    auto msg = r.get_msg();

    return 0;
}

