## CommandLineParser
一个基于C++17实现的轻量级命令行参数解析器

##### Version: 1.0.3

### Integration
将位于`include`目录中的 [xf_cmd_parser.h](include/xf_cmd_parser.h) 头文件拷贝到自己的项目中即可。

### Usage
* 包含头文件:
```C++
#include "xf_cmd_parser.h"
```
* 定义解析器对象并添加参数解析规则:
```C++
xf::cmd::Parser parser;
// or xf::cmd::Parser parser({ {{"-s", "--status"}, {xf::cmd::value_t::vt_bool, false, true, true}} });
parser.AddOption({{"-s", "--status"}, {xf::cmd::value_t::vt_bool, false, true, true}});
```
* 对命令行参数进行解析并获取结果：
```C++
auto result = parser.Parse(argv);
bool status = result.get<bool>("--status");
```

### Example
* 对`main`函数的参数进行解析
```C++
#include <iostream>
#include "xf_cmd_parser.h"

int main(int argc, char *argv[])
{
    xf::cmd::Parser parser(
        { {{"-i", "--input"}, {xf::cmd::value_t::vt_string, false, true, true}},
          {{"-c", "--config"}, {xf::cmd::value_t::vt_integer, false, true, false}},
          {{"-s", "-x", "--status"}, {xf::cmd::value_t::vt_boolean, false, true, false}} });

    auto result = parser.Parse(argv, 1, argc);

    if (result) // if (result.is_valid())
    {
        std::string input = result.get<std::string>("--input" /* or "-i" */);
        int config = result.get("--config" /* or "-i" */, 2);
        bool status = result.get("--status" /* or "-s" "-x" */, true);
        // ...
    }
    else
    {
        std::cout << "code: " << result.code() << std::endl;
        std::cout << result.info() << std::endl;
    }

    return 0;
}
```
更多示例参见 [example.cpp](example/example.cpp)

### Design
解析器设计用于对一组命令行参数按照指定的规则进行解析，规则包括：
* 命令行参数的格式，以下格式都是合法的：
```
-a
-ab
--abc value
--abc=value
```
* 参数值的类型(参见枚举`value_t`)
```C++
vt_string   // 字符串
vt_integer  // 有符号整数
vt_unsigned // 无符号整数
vt_float    // 浮点数
vt_boolean  // 布尔类型
vt_nothing  // 参数没有值
```
* 参数和值出现的规则，包括：
  * 参数是否具有唯一性，意味着命令行中一旦指定了该参数，将不能再指定其他任何参数。
  * 参数是否必须，即必须指定的参数，但如果已经指定了具有唯一性的参数，可以不指定。
  * 参数必须有值，即指定参数的同时必须为其指定一个值，否则可以不指定。

#### 举例说明
* 假设规定命令行参数中可能包含：
  * 参数名：`--input`，值类型：`string`，此参数必须在命令行中指定，同时也必须指定值。
  * 参数名：`--config`，值类型：`integer`，此参数必须在命令行中指定，但是值可选。
  * 参数名：`--status`，值类型：`bool`，可以不用在命令行中指定该参数，如果指定，其值也是可选的。
* 那么不同的情形解析结果如下：
  ```
  01 "--input /etc/dir/ --config 7 --status true" // ok: 参数名正确，对应的值类型也正确
  02 "--input /etc/dir/ --config=7 --status=true" // ok: 支持[key=value]的格式
  03 "--config 7 --input /etc/dir/ --status=true" // ok: 参数顺序可任意排列
  04 "--input /etc/dir/ --config 7"               // ok: --status 可以不指定
  05 "--input /etc/dir/ --config 7 --status"      // ok: --status 的值可以不指定
  06 "--input /etc/dir/ --config 7 --status="     // error: 在具有[key=value]格式的时候必须指定值
  07 "--input /etc/dir/ --status true"            // error: --config 必须要指定
  08 "--input /etc/dir/ --config 7 --status 1"    // error: --status 参数值类型错误
  09 "--output --config 7 --status true"          // error: 不能识别的参数 --output
  10 "--config 7 --status=true --input"           // error: 必须为参数 --input 指定一个值
  11 "--input --config 7 --status=true"           // error: 不能识别的参数 7
  12 "--input=/etc/dir/ --config --output"        // error: 参数 --config 值类型错误
  ```
  * 在情形11中，由于参数`--input`指定必须有值，因此紧跟其后的`--config`被视作为`--input`对应的值，则`7`被判定为不能识别的参数名。
  * 在情形12中，由于参数`--config`的值是可选的，因此优先判定后面的`--output`是否是有效的参数名，如果是，则判定`--config`没有指定值，如果不是，则将`--output`作为`--config`的值，但类型错误。

