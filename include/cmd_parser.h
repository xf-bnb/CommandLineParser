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
        is_unique  = 0x04           // key 是唯一的
    };

    enum state_t {
        state_ok,                   // ok
        error_unrecognized,         // 无法识别的指令
        error_duplicated,           // 重复的指令
        error_missing_required,     // 缺少必需的指令
        error_missing_value,        // 缺少参数
        error_value_type,           // 参数类型错误
        error_non_unique            // 指令过多
    };

    struct option_t
    {
        value_t vt{ vt_nothing };
        mode_t mt{ mt_none };

        option_t(value_t t = vt_nothing, mode_t r = mt_none) : vt(t), mt(r) { }
    };

    class Parser;

    class result_t
    {
        friend class Parser;

        using variant_t = std::variant<nullptr_t, bool, int, unsigned int, double, string_t>;

        state_t state{ state_ok };
        string_t msg{ "ok" };
        pair_t<string_t, string_t> hint;
        map_t<string_t, string_t> k_map;
        map_t<string_t, variant_t> v_map;

        void _set_error(state_t s, const string_t& key)
        {
            state = s;
            hint.first = key;

            switch (s)
            {
            case state_t::error_unrecognized:
                msg = "unrecognized parameter: " + key;
                break;
            case state_t::error_duplicated:
                hint.second = k_map[key];
                msg = "repeat paramter: " + hint.first + " and " + hint.second;
                break;
            case state_t::error_missing_required:
                msg = "the parameter " + key + " must be specified but not found.";
                break;
            case state_t::error_missing_value:
                msg = "parameter " + key + " must have a value.";
                break;
            case state_t::error_non_unique:
                msg = "extra parameter: " + key;
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
                state = state_t::error_duplicated;
                hint = *iter;
                msg = "repeat paramter: " + hint.first + " and " + hint.second;
                return false;
            }

            if (!k_map.empty() && mode_t::is_unique == (mode_t::is_unique & opt.mt))
            {
                state = state_t::error_non_unique;
                hint.first = key;
                msg = "parameter " + key + " can\'t be specified with other parameters.";
                return false;
            }

            return true;
        }

        template<typename _Type>
        void _add_value(const string_t& key, const _Type& value, const set_t<string_t>& keys)
        {
            v_map.emplace(std::make_pair(key, value));
            for (auto k : keys) k_map.emplace(std::make_pair(k, key));
        }

    public:

        result_t() = default;

        result_t(state_t s, const string_t& text, const pair_t<string_t, string_t>& h)
            : state(s), msg(text), hint(h)
        {

        }

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

        auto get() const { return v_map; }
    };

    class Parser
    {
        struct less_t;
       
        unsigned int group_id{ 0 };
        map_t<string_t, unsigned int> key_map;
        map_t<unsigned int, pair_t<set_t<string_t>, option_t>> opt_map;

    public:

        Parser(const list_t<pair_t<set_t<string_t>, option_t>>& options)
        {
            for (auto opt : options) AddOption(opt);
        }

        unsigned int AddOption(const pair_t<set_t<string_t>, option_t>& option)
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

        unsigned int RemoveOption(const set_t<string_t>& keys)
        {
            unsigned int n = 0;
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
            unsigned int index = 0;

            mode_t mt(mt_none);

            for (auto arg : args)
            {
            }

            return result_t();
        }

        result_t Parse(const char* const argv[], int from, int to) const
        {
            return Parse(list_t<string_t>(argv + from, argv + to));
        }

    private:

        struct less_t {
            bool operator()(const string_t& a, const string_t& b) const {
                return b.size() < a.size();
            }
        };

        using parse_func = unsigned int (Parser::*)(result_t&, const string_t&, const std::set<string_t, less_t>&, string_t&) const;

        template<unsigned int n>
        result_t _Parse(const list_t<string_t>& args, const parse_func(&funcs)[n]) const
        {
            result_t result;
            string_t key;
            unsigned int index(0);
            auto keys = _Keys();

            for (auto arg : args)
            {
                if (index < n)
                    index = (this->*funcs[index])(result, arg, keys, key);
                else
                    break;
            }


            return result;
        }

        bool _Parse(result_t& result, const string_t& arg, const std::set<string_t, less_t>& keys, pair_t<string_t, unsigned int>& v) const
        {
            for (auto key : keys)
            {
                if (_start_with(arg, key))
                {
                    if (arg.size() == key.size())
                    {
                        if (result._check_key(key, _Option(key)))
                        {
                            v.first = key;
                            v.second = 1;
                            return true;
                        }

                        return false;
                    }
                    else
                    {
                        if ((key.size() + 1) < arg.size() && '=' == arg[key.size()])
                        {
                            option_t opt = _Option(key);
                            if (value_t::vt_nothing != opt.vt)
                            {
                                if (result._check_key(key, opt))
                                {
                                    switch (opt.vt)
                                    {
                                    case value_t::vt_string:
                                        result._add_value(key, arg.substr(key.size() + 1), _Keys(key));
                                        return true;
                                    default:
                                        break;
                                    }
                                }

                                return false;
                            }
                        }

                        result._set_error(state_t::error_unrecognized, arg);
                        return false;
                    }
                }
            }

            return false;
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

        static bool _start_with(const string_t& a, const string_t& b)
        {
            return 0 == a.compare(0, b.size(), b);
        }

    };

}

