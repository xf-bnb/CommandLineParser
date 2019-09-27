#include <iostream>

#include "../include/cmd_parser.h"


void Show(const xf::cmd::Parser::result_t& result)
{
    std::cout << "code: " << result.get_state() << ", msg: " << result.get_msg() << std::endl;
    
}


int main()
{
    using v_t = xf::cmd::value_t;
    using m_t = xf::cmd::mode_t;
    using opt_t = xf::cmd::option_t;

    xf::cmd::Parser parser(
        { {{"-a", "--aa"}, {v_t::vt_boolean, m_t::k_required | m_t::v_required}},
          {{"-b", "--bb"}, {v_t::vt_integer, m_t::k_required | m_t::v_required}},
          {{"-c", "--cc"}, {v_t::vt_unsigned, m_t::k_required | m_t::v_required}}, 
          {{"-d", "--dd"}, {v_t::vt_float, m_t::v_required}}, 
          {{"-x", "--xx"}, {v_t::vt_string, m_t::k_required | m_t::v_required}},
          {{"-y", "--yy"}, {v_t::vt_string, m_t::k_required}},
          {{"-z", "--zz"}, {v_t::vt_string, m_t::mt_none}} });


    const char* args[] = { "-a", "true", "--bb", "123", "-c=7", "-d=1.2", "--xx", "Frank", "--yy", "Xiong", "--zz" };

    auto result = parser.Parse(args);

    Show(result);

    return 0;
}

