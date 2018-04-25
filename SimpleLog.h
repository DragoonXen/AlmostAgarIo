//
// Created by dragoon on 11/11/17.
//

#ifndef CPP_CGDK_SIMPLELOG_H
#define CPP_CGDK_SIMPLELOG_H


/**
 * MIT License
 *
 * Copyright (c) 2017 Kiselev Vladimir
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/**
 * Simple logger: when you need log, but really don't need all those log features (several files, log rotation etc)
 * Features:
 *      Formatted output using '%' signs with streams under hood with compile time checking.
 *      Header only with unnoticed compilation footprint because of simplicity.
 *
 * Sample usage:
 * int main() {
 *     LOG_ERROR("Bad error %", "happen");
 *     LOG_WARN("Some warning");
 *     LOG_INFO("Info message with %% signs");
 * }
 *
 * Compiler with c++14 support and std::put_time needed
 * For serious projects you need something more robust, and possibly fast
 */
#pragma once

#include <iostream>
#include <iomanip>
#include <ctime>

#define SIMPLELOG_STREAM std::cerr
#define SIMPLELOG_TIME_FORMAT "%d/%m/%Y %T"

namespace logger {
    namespace details {

        template<size_t N>
        constexpr size_t get_specifiers_cnt(const char (&format)[N]) {
            size_t specifiers = format[N - 1] == '%';
            for (size_t i = 0; i < N - 1; ++i) {
                if (format[i] == '%' && format[i + 1] == '%') {
                    ++i;
                    continue;
                }
                specifiers += format[i] == '%';
            }
            return specifiers;
        }

        template<std::size_t N, typename... Args>
        constexpr void check_format(Args... args) {
            static_assert(N == sizeof...(args), "Formatting specifiers count mismatch number of arguments");
        }

        inline void logline_impl(const char *format) {
            std::string str;
            for (; *format; ++format) {
                if (format[0] == '%') {
                    ++format;
                }
                str += format[0];
            }
            SIMPLELOG_STREAM << str << '\n';
        }

        template<typename T, typename... Args>
        void logline_impl(const char *format, T val, Args... args) {
            for (; *format; ++format) {
                if (format[0] == '%') {
                    if (format[1] == '%') {
                        ++format;
                    } else {
                        SIMPLELOG_STREAM << val;
                        logline_impl(format + 1, args...);
                        return;
                    }
                }
                SIMPLELOG_STREAM << format[0];
            }
        }

    } // namespace details

    template<typename... Args>
    void logline(const char *type, const char *format, Args... args) {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        SIMPLELOG_STREAM << '[' << std::put_time(&tm, SIMPLELOG_TIME_FORMAT) << "] "
                         << type << ": ";
        details::logline_impl(format, args...);
    }

} // namespace logger

#define CHECK(format, ...) logger::details::check_format<logger::details::get_specifiers_cnt(format)>(__VA_ARGS__);
#define LOG(type, format, ...) {CHECK(format, __VA_ARGS__); logger::logline(type, format, ##__VA_ARGS__); }

#define LOG_INFO(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERROR(format, ...)
#define LOG_DEBUG(format, ...)

#ifdef ENABLE_LOG_ERROR
#undef LOG_ERROR
#define LOG_ERROR(format, ...) LOG("\033[1;31m" "ERROR"   "\033[0m", format, ##__VA_ARGS__);
#endif

#ifdef ENABLE_LOG_WARN
#undef LOG_WARN
#define LOG_WARN(format, ...)  LOG("\033[1;33m" "WARNING" "\033[0m", format, ##__VA_ARGS__);
#endif

#ifdef ENABLE_LOG_INFO
#undef LOG_INFO
#define LOG_INFO(format, ...)  LOG("\033[1;34m" "INFO"    "\033[0m", format, ##__VA_ARGS__);
#endif

#ifdef ENABLE_LOG_DEBUG
#undef LOG_DEBUG
#define LOG_DEBUG(format, ...) LOG("DEBUG",                          format, ##__VA_ARGS__);
#endif


#endif //CPP_CGDK_SIMPLELOG_H
