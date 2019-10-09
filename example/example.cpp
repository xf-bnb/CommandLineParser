#include <iostream>

#include "../include/cmd_parser.h"

using v_t = xf::cmd::value_t;
using opt_t = xf::cmd::option_t;

void Show(const xf::cmd::Parser::result_t& result)
{
    std::cout << "parse result: code: " << result.code() << ", info: " << result.info() << std::endl;
    for (auto item : result.args())
        std::cout << item.first << ": " << item.second << std::endl;

    std::cout << std::endl;
}

bool test_0()
{
    xf::cmd::Parser parser(
        { {{"--a",      "-a"}, {v_t::vt_boolean,  false, true, true}},
          {{"--ab",     "-b"}, {v_t::vt_integer,  false, true, true}},
          {{"--abc",    "-c"}, {v_t::vt_unsigned, false, true, true}},
          {{"--abcd",   "-d"}, {v_t::vt_float,    false, true, false}},
          {{"--abcde",  "-e"}, {v_t::vt_string,   false, true, true}},
          {{"--abcdef", "-f"}, {v_t::vt_nothing,  false, true, false}} });

    const char* args[] = { "--abcd=1.2", "--abc", "7", "--abcdef", "-b", "-2", "-a", "false", "-e=Frank" };

    auto result = parser.Parse(args);

    Show(result);

    return (result
            && (result.get<bool>       ("-a") == false)
            && (result.get<int>        ("-b") == -2)
            && (result.get<unsigned>   ("-c") == 7)
            && (result.get<double>     ("-d") == 1.2)
            && (result.get<std::string>("-e") == "Frank")
            && (result.is_existing("-f")));
}

bool test_1()
{
    xf::cmd::Parser parser(
        { {{"-i", "--input"},    opt_t::make<xf::cmd::string_t>(false, true, true)},
          {{"-o", "--output"},   {v_t::vt_string, false, false, false}},
          {{"-m", "--mode"},     {v_t::vt_unsigned, false, false, true, [](const std::string& v) { return (opt_t::is_unsigned(v) && 0 == std::stoi(v) % 2); }}},
          {{"-l", "--level"},    {v_t::vt_string, false, true, true, "L[1234]"}},
          {{"-x", "--external"}, opt_t::make<bool>(false, false, true)},
          {{"-f", "--format"},   opt_t::make<std::string>(false, false, true, "xml|json|edn")},
          {{"-d", "--detail"},   opt_t::make<nullptr_t>(false, false)} });

    auto result = parser.Parse({ });
    Show(result);
    bool p1 = (!result.is_valid() && xf::cmd::Parser::state_t::s_nothing == result.code());

    result = parser.Parse({ "--format=json", "--setting=true" });
    Show(result);
    bool p2 = (!result && xf::cmd::Parser::state_t::s_k_unrecognized == result.code());

    result = parser.Parse({ "--format=json", "-f=xml" });
    Show(result);
    bool p3 = (!result && xf::cmd::Parser::state_t::s_k_duplicated == result.code());

    result = parser.Parse({ "-o", "/home/user/workspace/", "--format=json", "--level=L4", "--detail" });
    Show(result);
    bool p4 = (!result && xf::cmd::Parser::state_t::s_k_missing == result.code());

    result = parser.Parse({ "-i", "/home/file", "-o", "/home/user/workspace/", "--format=json", "--level=L4", "-x" });
    Show(result);
    bool p5 = (!result && xf::cmd::Parser::state_t::s_v_missing == result.code());

    result = parser.Parse({ "-i", "/home/file", "-o", "/home/user/workspace/", "--format=json", "--level=L4", "-d=1" });
    Show(result);
    bool p6 = (!result && xf::cmd::Parser::state_t::s_v_redundant == result.code());

    result = parser.Parse({ "-i", "/home/file", "-o", "/home/user/workspace/", "--format=html", "-x=true", "--level=L4" });
    Show(result);
    bool p7 = (!result && xf::cmd::Parser::state_t::s_v_error == result.code());

    result = parser.Parse({"-i", "/home/file", "-o=.", "-m=4", "--level", "L2", "-x=true", "-d", "-f", "xml" });
    Show(result);
    bool p8 = (result && (result.get<std::string>("--input") == "/home/file")
                  && (result.get<std::string>("--output") == ".")
                  && (result.get<std::string>("--level") == "L2")
                  && (result.get<std::string>("--format") == "xml")
                  && (result.get<unsigned int>("--mode") == 4)
                  && result.get<bool>("--external")
                  && result.is_existing("--detail"));

    return (p1 && p2 && p3 && p4 && p5 && p6 && p7 && p8);
}

bool test_2()
{
    xf::cmd::Parser parser(
        { {{"-x", "--xx"}, {v_t::vt_boolean, true, true, true}},
          {{"-y", "--yy"}, {v_t::vt_integer, false, true, true}} });

    auto result = parser.Parse({ "-x=false", "-y=7" });

    Show(result);

    return (xf::cmd::Parser::state_t::s_k_conflict == result.code());
}

bool test_3()
{
    xf::cmd::Parser parser;
    parser.AddOption({ {"-x", "--xx"}, opt_t::make<bool>(true, true, true) })
          .AddOption({ {"-y", "--yy"}, {v_t::vt_string, false, true, true, std::bind(&xf::cmd::Parser::IsValid, &parser, std::placeholders::_1)} })
          .AddOption({ {"-z", "--zz"}, {v_t::vt_integer, false, true, true, "[12][0-9]"} });

    auto result = parser.Parse({ "-x=false", "-y=--xx" });

    Show(result);

    return (xf::cmd::Parser::state_t::s_k_conflict == result.code());

    return false;
}

int main()
{
    bool (*test_func_list[])() = { test_0, test_1, test_2, test_3 };

    const unsigned int total = std::extent<decltype(test_func_list)>::value;
    unsigned int ok = 0;

    for (auto i = 0; i < total; ++i)
    {
        bool result = (*test_func_list[i])();
        std::cout << "--> testing: " << i << ", result: " << (result ? "true" : "false") << std::endl << std::endl;

        if (result) ++ok;
    }

    std::cout << "total: " << total << " testing, " << ok << " successes, " << (total - ok) << " failures." << std::endl;

    return 0;
}
