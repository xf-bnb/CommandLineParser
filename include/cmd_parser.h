#pragma once

#include <regex>
#include <variant>
#include <string>
#include <vector>
#include <set>
#include <map>


namespace xf::cmd
{
    using string_t = std::string;
    using size_type = std::size_t;

    template<typename _Type>
    using set_t = std::set<_Type>;

    template<typename _Type>
    using list_t = std::vector<_Type>;

    template<typename _KeyType, typename _ValueType>
    using pair_t = std::pair<_KeyType, _ValueType>;

    template<typename _KeyType, typename _ValueType>
    using map_t = std::map<_KeyType, _ValueType>;

    enum value_t {
        vt_nothing, vt_string, vt_integer, vt_unsigned, vt_float, vt_boolean
    };

    enum mode_t {
        mt_none    = 0x00,
        v_required = 0x01,          // value 是必需的
        k_required = 0x02,          // key 是必需的
        is_unique  = 0x04           // key 是独占的
    };

    enum state_t {
        state_ok,                   // ok
        error_nothing,              // 没有任何参数
        error_unrecognized,         // 无法识别的参数
        error_duplicated,           // 重复的参数
        error_missing_required,     // 缺少必需的参数
        error_missing_value,        // 缺少参数
        error_redundant_value,      // 多余的参数值
        error_value_type,           // 参数值类型错误
        error_non_unique            // 参数唯一性冲突
    };

    struct option_t
    {
        value_t vt{ vt_nothing };
        mode_t mt{ mt_none };

        option_t(value_t t = vt_nothing, mode_t r = mt_none) : vt(t), mt(r) { }

        bool check_mode(unsigned int t) const { return (t == (t & mt)); }
    };

    class Parser
    {
    public:

        class result_t
        {
            friend class Parser;

            using variant_t = std::variant<nullptr_t, bool, int, unsigned int, double, string_t>;

            state_t state;
            string_t msg;
            pair_t<string_t, string_t> hint;
            map_t<string_t, string_t> k_map;
            map_t<string_t, variant_t> v_map;

            result_t(state_t s, const string_t& text) : state(s), msg(text) { }

        public:

            bool is_valid() const { return (state_ok == state); }
            bool is_existing(const string_t& key) const { return k_map.find(key) != k_map.end(); }
            bool has_value(const string_t& key) const { return is_existing(key); }
            state_t get_state() const { return state; }
            const string_t& get_msg() const { return msg; }

            operator bool() const { return is_valid(); }
            operator const string_t& () const { return get_msg(); }

            template<typename _Type>
            _Type get(const string_t& key) const
            {
                if (is_existing(key))
                {
                    return std::get<_Type>(v_map.at(k_map.at(key)));
                }
                else
                {
                    throw string_t("the key: " + key + " not existing !");
                }
            }

            template<typename _Type>
            _Type get(const string_t& key, const _Type& value) const
            {
                try
                {
                    return get<_Type>(key);
                }
                catch (const std::exception& e)
                {
                    return value;
                }
            }

            const map_t<string_t, variant_t>& get() const { return v_map; }

        private:

            void _set_error(state_t s, const string_t& key)
            {
                state = s;
                hint.first = key;

                switch (s)
                {
                case state_t::error_unrecognized:
                    msg = R"(error: unrecognized parameter ")" + key + R"(")";
                    break;
                case state_t::error_duplicated:
                    hint.second = k_map[key];
                    msg = R"(error: repeat paramter ")" + hint.first + R"(" and ")" + hint.second + R"(")";
                    break;
                case state_t::error_missing_required:
                    msg = R"(error: the parameter ")" + key + R"(" must be specified but not found.)";
                    break;
                case state_t::error_missing_value:
                    msg = R"(error: parameter ")" + key + R"(" must have a value.)";
                    break;
                case state_t::error_redundant_value:
                    msg = R"(error: the parameter ")" + key + R"(" doesn't require value.)";
                    break;
                case state_t::error_value_type:
                    msg = R"(error: value-type error of parameter ")" + key + R"(")";
                    break;
                case state_t::error_non_unique:
                    msg = R"(error: parameter ")" + key + R"(" can't be specified with other parameters.)";
                    break;
                default:
                    break;
                }
            }

            bool _check_key(const string_t& key, const option_t& opt)
            {
                auto iter = k_map.find(key);
                if (iter != k_map.end())
                {
                    _set_error(state_t::error_duplicated, key);
                    return false;
                }

                if (!k_map.empty() && opt.check_mode(mode_t::is_unique))
                {
                    _set_error(state_t::error_non_unique, key);
                    return false;
                }

                return true;
            }

            template<typename _Type>
            void _add_value(const string_t& key, const _Type& value, const set_t<string_t>& keys)
            {
                v_map.emplace(std::make_pair(key, variant_t(value)));
                for (auto k : keys) k_map.emplace(std::make_pair(k, key));
            }

            template<>
            void _add_value<nullptr_t>(const string_t& key, const nullptr_t& value, const set_t<string_t>& keys)
            {
                _add_value(key, keys);
            }

            void _add_value(const string_t& key, const string_t& value, const set_t<string_t>& keys, const option_t& opt)
            {
                switch (opt.vt)
                {
                case value_t::vt_string:
                    _add_value(key, value, keys);
                    break;
                case value_t::vt_boolean:
                    _add_value(key, ('t' == value[0] || 'T' == value[0]), keys);
                    break;
                case value_t::vt_float:
                    _add_value(key, std::stod(value), keys);
                    break;
                case value_t::vt_integer:
                    _add_value(key, std::stoi(value), keys);
                    break;
                case value_t::vt_unsigned:
                    _add_value(key, unsigned int(std::stoul(value)), keys);
                    break;
                default:
                    _add_value(key, nullptr, keys);
                    break;
                }
            }

            void _add_value(const string_t& key, const set_t<string_t>& keys)
            {
                for (auto k : keys) k_map.emplace(std::make_pair(k, key));
            }

        };

        unsigned int group_id{ 0 };
        map_t<string_t, unsigned int> key_map;
        map_t<unsigned int, pair_t<set_t<string_t>, option_t>> opt_map;

        enum { on_key, on_value, on_opt, parse_error };

    public:

        Parser() = default;

        Parser(const list_t<pair_t<set_t<string_t>, option_t>>& options)
        {
            for (auto opt : options) AddOption(opt);
        }

        size_type AddOption(const pair_t<set_t<string_t>, option_t>& option)
        {
            if (!option.first.empty())
            {
                ++group_id;

                opt_map.emplace(std::make_pair(group_id, option));

                for (auto key : option.first)
                {
                    auto iter = key_map.find(key);
                    if (iter != key_map.end())
                    {
                        _RemoveOption(iter->second, key);
                        iter->second = group_id;
                    }
                    else
                    {
                        key_map.emplace(std::make_pair(key, group_id));
                    }
                }
            }

            return option.first.size();
        }

        size_type RemoveOption(const set_t<string_t>& keys)
        {
            size_type n = 0;
            for (auto key : keys)
            {
                auto iter = key_map.find(key);
                if (iter != key_map.end())
                {
                    _RemoveOption(iter->second, key);
                    key_map.erase(iter);
                    ++n;
                }
            }

            return n;
        }

        result_t Parse(const list_t<string_t>& args) const
        {
            if (args.empty())
                return result_t(state_t::error_nothing, R"(error: don't get any parameter.)");

            return _Parse(args, { &Parser::_OnKey, &Parser::_OnValue, &Parser::_OnOptional });
        }

        result_t Parse(const char* const argv[], int from, int to) const
        {
            return Parse(list_t<string_t>(argv + from, argv + to));
        }

    private:

        struct less_t {
            bool operator()(const string_t& a, const string_t& b) const {
                return ((a.size() == b.size()) ? (a < b) : (b.size() < a.size()));
            }
        };

        using _parse_func_type = size_type (Parser::*)(result_t&, const string_t&, const std::set<string_t, less_t>&, string_t&, option_t&) const;

        template<size_type n>
        result_t _Parse(const list_t<string_t>& args, const _parse_func_type(&_parse_functions)[n]) const
        {
            string_t key;
            option_t opt;
            size_type index(on_key);
            auto keys = _Keys();
            result_t result(state_t::state_ok, "ok");

            for (auto arg : args)
            {
                if (index < n)
                    index = (this->*_parse_functions[index])(result, arg, keys, key, opt);
                else
                    break;
            }

            switch (index)
            {
            case on_opt:
                result._add_value(key, _Keys(key));
            case on_key:
                _CheckResult(result);
                break;
            case on_value:
                result._set_error(state_t::error_missing_value, key);
                break;
            default:
                break;
            }

            return result;
        }

        size_type _OnKey(result_t& result, const string_t& arg, const std::set<string_t, less_t>& keys, string_t& k, option_t& opt) const
        {
            for (auto key : keys)
            {
                if (_start_with(arg, key))
                {
                    if (_is_perfect_match(arg, key))
                        return _OnPerfectMatch(result, key, k, opt);

                    if (_is_equation(arg, key))
                        return _OnEquation(result, key, arg.substr(key.size() + 1), _Option(key));

                    break;
                }
            }

            result._set_error(state_t::error_unrecognized, arg);
            return parse_error;
        }

        size_type _OnValue(result_t& result, const string_t& arg, const std::set<string_t, less_t>& keys, string_t& k, option_t& opt) const
        {
            return _OnValueEx(result, k, arg, opt);
        }

        size_type _OnOptional(result_t& result, const string_t& arg, const std::set<string_t, less_t>& keys, string_t& k, option_t& opt) const
        {
            for (auto key : keys)
            {
                if (_start_with(arg, key))
                {
                    if (_is_perfect_match(arg, key))
                    {
                        result._add_value(k, _Keys(k));
                        return _OnPerfectMatch(result, key, k, opt);
                    }

                    if (_is_equation(arg, key))
                    {
                        result._add_value(k, _Keys(k));
                        return _OnEquation(result, key, arg.substr(key.size() + 1), _Option(key));
                    }

                    break;
                }
            }

            return _OnValue(result, arg, keys, k, opt);
        }

        size_type _OnPerfectMatch(result_t& result, const string_t& key, string_t& k, option_t& opt) const
        {
            opt = _Option(key);
            if (!result._check_key(key, opt))
                return parse_error;

            k = key;
            if (value_t::vt_nothing == opt.vt)
            {
                result._add_value(key, nullptr, _Keys(key));
                return on_key;
            }

            return (opt.check_mode(mode_t::k_required) ? on_value : on_opt);
        }

        size_type _OnEquation(result_t& result, const string_t& key, const string_t& value, const option_t& opt) const
        {
            if (!result._check_key(key, opt))
                return parse_error;

            if (value_t::vt_nothing == opt.vt)
            {
                result._set_error(state_t::error_redundant_value, key);
                return parse_error;
            }

            return _OnValueEx(result, key, value, opt);
        }

        size_type _OnValueEx(result_t& result, const string_t& key, const string_t& value, const option_t& opt) const
        {
            if (_check_type(value, opt))
            {
                result._add_value(key, value, _Keys(key), opt);
                return on_key;
            }

            result._set_error(state_t::error_value_type, key);
            return parse_error;
        }

        const option_t& _Option(const string_t& key) const
        {
            return opt_map.at(key_map.at(key)).second;
        }

        const set_t<string_t>& _Keys(const string_t& key) const
        {
            return opt_map.at(key_map.at(key)).first;
        }

        std::set<string_t, less_t> _Keys() const
        {
            std::set<string_t, less_t> keys;
            for (auto v : key_map)
                keys.emplace(v.first);

            return keys;
        }

        bool _RemoveOption(unsigned int id, const string_t& key)
        {
            auto iter = opt_map.find(id);
            if (iter != opt_map.end())
            {
                iter->second.first.erase(key);
                if (iter->second.first.empty())
                    opt_map.erase(iter);

                return true;
            }

            return false;
        }

        bool _CheckResult(result_t& result) const
        {
            if (state_t::state_ok == result.get_state())
            {
                if (_Option(result.k_map.cbegin()->first).check_mode(mode_t::is_unique))
                    return true;

                for (auto opt : opt_map)
                {
                    if (opt.second.second.check_mode(mode_t::k_required))
                    {
                        const string_t& key(*opt.second.first.cbegin());
                        if (!result.is_existing(key))
                        {
                            result._set_error(state_t::error_missing_required, key);
                            return false;
                        }
                    }
                }

                return true;
            }

            return false;
        }

        static bool _is_perfect_match(const string_t& text, const string_t& key)
        {
            return (text.size() == key.size());
        }

        static bool _is_equation(const string_t& text, const string_t& key)
        {
            return ((key.size() + 1) < text.size() && '=' == text[key.size()]);
        }

        static bool _start_with(const string_t& a, const string_t& b)
        {
            return 0 == a.compare(0, b.size(), b);
        }

        static bool _check_type(const string_t& value, const option_t& opt)
        {
            switch (opt.vt)
            {
            case value_t::vt_string:
                return (!value.empty());
            case value_t::vt_boolean:
                return std::regex_match(value, std::regex("[Tt]rue|[Ff]alse|TRUE|FALSE"));
            case value_t::vt_float:
                return std::regex_match(value, std::regex("[+-]?(0|[1-9][1-9]*)([.][0-9]+)?"));
            case value_t::vt_integer:
                return std::regex_match(value, std::regex("[+-]?(0|[1-9][1-9]*)"));
            case value_t::vt_unsigned:
                return std::regex_match(value, std::regex("0|[1-9][1-9]*"));
            default:
                return false;
            }
        }
    };

}
