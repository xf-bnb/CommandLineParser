#include <iostream>

#include "../include/cmd_parser.h"

void Show(const xf::cmd::Parser::result_t& result)
{
    std::cout << "code: " << result.code() << ", msg: " << result.msg() << std::endl;
    for (auto item : result.args())
        std::cout << item.first << ": " << item.second << std::endl;
}


int main()
{
    using v_t = xf::cmd::value_t;
    using opt_t = xf::cmd::option_t;

    auto opt1 = xf::cmd::option_t::make<bool>(true, true, true, "");
    auto opt2 = xf::cmd::option_t::make<bool>(true, true, true, [](const std::string&) { return false; });
    auto opt3 = xf::cmd::option_t::make<bool>(true, true, true);
    auto opt4 = xf::cmd::option_t::make<int>(true, true, true);
    auto opt5 = xf::cmd::option_t::make<std::string>(true, true, true);
    /*
    xf::cmd::Parser parser(
        { {{"-a", "--aa"}, {v_t::vt_boolean, m_t::k_required | m_t::v_required}},
          {{"-ab", "--bb"}, {v_t::vt_integer, m_t::k_required}},
          {{"-abc", "--cc"}, {v_t::vt_nothing, m_t::k_required | m_t::v_required}},
          {{"-abcd", "--dd"}, {v_t::vt_unsigned, m_t::v_required}}, 
          {{"-x", "--xx"}, {v_t::vt_string, m_t::k_required | m_t::v_required}},
          {{"-y", "--yy"}, {v_t::vt_unsigned, m_t::k_required}},
          {{"-z", "--zz"}, {v_t::vt_string, m_t::mt_none | m_t::k_unique}} });
    */
    xf::cmd::Parser parser(
        { {{"-a", "--aa"}, opt_t::make<bool>(false, true, true)},
          {{"-ab", "--bb"}, opt_t::make<int>(false, true, false)},
          {{"-abc", "--cc"}, opt_t::make<nullptr_t>(false, true)},
          {{"-abcd", "--dd"}, opt_t::make<unsigned int>(false, false, true)},
          {{"-x", "--xx"}, opt_t::make<std::string>(false, false, true)},
          {{"-y", "--yy"}, opt_t::make<unsigned int>(false, true, false)},
          {{"-z", "--zz"}, opt_t::make<std::string>(false, false, false)} });

    // const char* args[] = { "-a", "true", "--bb", "123", "-c", "-d=8", "--xx", "Frank", "--yy", "Xiong", "--zz" };
    const char* args[] = { "-abcd=10", "-abc", "-ab", "-a", "true", "--yy", "100", "--zz", "--xx=Frank" };

    auto result = parser.Parse(args);

    Show(result);

    return 0;
}
