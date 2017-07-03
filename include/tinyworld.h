// Copyright (c) 2017 david++
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef _COMMON_TINTYWORLD_H
#define _COMMON_TINTYWORLD_H

#include <functional>
#include <iomanip>
#include <iostream>


#define TINY_NAMESPACE_BEGIN // namespace tiny {
#define TINY_NAMESPACE_END   // }

//
// 利用构造函数执行一些初始化代码的技巧
//
struct RunOnceHelper {
    RunOnceHelper(std::function<void()> func) {
        func();
    }
};

#define RUN_ONCE(tagname) \
        void reg_func_##tagname(); \
        RunOnceHelper reg_obj_##tagname(reg_func_##tagname); \
        void reg_func_##tagname()

//
// 输出二进制(仿照`hexdump -C`命令)
//
inline void hexdump(const std::string &hex, std::ostream &os = std::cout) {
    size_t line = 0;
    size_t left = hex.size();

    while (left) {
        size_t linesize = (left >= 16 ? 16 : left);
        std::string linestr = hex.substr(line * 16, linesize);

        os << std::hex << std::setw(8) << std::setfill('0') << line * 16 << " ";
        for (size_t i = 0; i < linestr.size(); ++i) {
            if (i % 8 == 0)
                os << " ";

            int v = (uint8_t) linestr[i];
            os << std::hex << std::setw(2) << std::setfill('0') << v << " ";
        }

        os << " " << "|";
        for (size_t i = 0; i < linestr.size(); ++i) {
            if (std::isprint(linestr[i]))
                os << linestr[i];
            else
                os << '.';
        }
        os << "|" << std::endl;

        left -= linesize;
        line++;
    }
}

#endif // _COMMON_TINTYWORLD_H