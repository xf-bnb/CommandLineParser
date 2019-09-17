#pragma once

#include <string>
#include <vector>
#include <set>

namespace xf
{

    using string_t = std::string;
    
    template<typename _Type>
    using list_t = std::vector<_Type>;

    class Rule
    {
    public:

        enum ValueType {
            Nothing, String, Integer, Float, Boolean
        };


        list_t<string_t> keys;
        ValueType vt;
        bool required;


        Rule(const std::set<string_t>& k, ValueType t, bool r)
            : keys(k.begin(), k.end()), vt(t), required(r)
        { }
    };


    class Parser
    {





    };

    class Result
    {
        string_t key;


        bool IsExisting(const string_t& key) const { return false; }

        template<typename _Type>
        _Type Get(const string_t& key) const
        {
            if (IsExisting(key))
            {
                return _Type();
            }
            else
            {
                throw string_t("the key: " + key + " not existing !");
            }
        }


        template<typename _Type>
        _Type Get(const string_t& key, const _Type& value) const
        {
            try
            {
                return Get<_Type>(key);
            }
            catch (const std::exception& e)
            {
                return value;
            }
        }
    };
}

