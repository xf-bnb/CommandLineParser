#include <iostream>

#include "../include/cmd_parser.h"

void Show(const xf::cmd::Parser::result_t& result)
{
    std::cout << "code: " << result.code() << ", msg: " << result.message() << std::endl;
    for (auto item : result.args())
        std::cout << item.first << ": " << item.second << std::endl;
}


int main()
{
    using v_t = xf::cmd::value_t;
    using m_t = xf::cmd::mode_t;
    using opt_t = xf::cmd::option_t;

    xf::cmd::Parser parser(
        { {{"-a", "--aa"}, {v_t::vt_boolean, m_t::k_required | m_t::v_required}},
          {{"-ab", "--bb"}, {v_t::vt_integer, m_t::k_required}},
          {{"-abc", "--cc"}, {v_t::vt_nothing, m_t::k_required | m_t::v_required}},
          {{"-abcd", "--dd"}, {v_t::vt_unsigned, m_t::v_required}}, 
          {{"-x", "--xx"}, {v_t::vt_string, m_t::k_required | m_t::v_required}},
          {{"-y", "--yy"}, {v_t::vt_unsigned, m_t::k_required}},
          {{"-z", "--zz"}, {v_t::vt_string, m_t::mt_none | m_t::k_unique}} });

    const char* args[] = { "-a", "true", "--bb", "123", "-c", "-d=8", "--xx", "Frank", "--yy", "Xiong", "--zz" };
    // const char* args[] = { /*"-abcd=10",*/ "-abc", "-ab", "-a", "true", "--yy", "100", "--zz", "--xx" };

    auto result = parser.Parse(args);

    Show(result);

    return 0;
}
