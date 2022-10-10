// Copyright (C) 2020-2022 Jonathan Müller and lexy contributors
// SPDX-License-Identifier: BSL-1.0

#ifndef LEXY_ERROR_HPP_INCLUDED
#define LEXY_ERROR_HPP_INCLUDED

#include <lexy/_detail/config.hpp>
#include <lexy/grammar.hpp>
#include <lexy/input/base.hpp>

namespace lexy
{
template <typename Reader, typename Tag>
class error;

/// Type erased generic failure.
template <typename Reader>
class error<Reader, void>
{
public:
    constexpr explicit error(typename Reader::iterator pos, const char* msg) noexcept
    : _pos(pos), _end(pos), _msg(msg)
    {}
    constexpr explicit error(typename Reader::iterator begin, typename Reader::iterator end,
                             const char* msg) noexcept
    : _pos(begin), _end(end), _msg(msg)
    {}

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr const char* message() const noexcept
    {
        return _msg;
    }

    constexpr auto begin() const noexcept
    {
        return _pos;
    }
    constexpr auto end() const noexcept
    {
        return _end;
    }

private:
    typename Reader::iterator _pos;
    typename Reader::iterator _end;
    const char*               _msg;
};

/// Generic failure.
template <typename Reader, typename Tag>
class error : public error<Reader, void>
{
public:
    constexpr explicit error(typename Reader::iterator pos) noexcept
    : error<Reader, void>(pos, _detail::type_name<Tag>())
    {}
    constexpr explicit error(typename Reader::iterator begin,
                             typename Reader::iterator end) noexcept
    : error<Reader, void>(begin, end, _detail::type_name<Tag>())
    {}
};

/// Expected the literal character sequence.
struct expected_literal
{};
template <typename Reader>
class error<Reader, expected_literal>
{
public:
    constexpr explicit error(typename Reader::iterator                   pos,
                             const typename Reader::encoding::char_type* str, std::size_t index,
                             std::size_t length) noexcept
    : _pos(pos), _str(str), _idx(index), _length(length)
    {}

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr auto string() const noexcept -> const typename Reader::encoding::char_type*
    {
        return _str;
    }

    constexpr std::size_t index() const noexcept
    {
        return _idx;
    }

    constexpr std::size_t length() const noexcept
    {
        return _length;
    }

    constexpr auto character() const noexcept
    {
        return _str[_idx];
    }

private:
    typename Reader::iterator                   _pos;
    const typename Reader::encoding::char_type* _str;
    std::size_t                                 _idx, _length;
};

/// Expected the given keyword.
/// Unlike expected_literal, this one looks at the following characters as well.
struct expected_keyword
{};
template <typename Reader>
class error<Reader, expected_keyword>
{
public:
    constexpr explicit error(typename Reader::iterator begin, typename Reader::iterator end,
                             const typename Reader::encoding::char_type* str, std::size_t length)
    : _begin(begin), _end(end), _str(str), _length(length)
    {}

    constexpr auto position() const noexcept
    {
        return _begin;
    }

    constexpr auto begin() const noexcept
    {
        return _begin;
    }
    constexpr auto end() const noexcept
    {
        return _end;
    }

    constexpr auto string() const noexcept -> const typename Reader::encoding::char_type*
    {
        return _str;
    }

    constexpr std::size_t length() const noexcept
    {
        return _length;
    }

private:
    typename Reader::iterator                   _begin;
    typename Reader::iterator                   _end;
    const typename Reader::encoding::char_type* _str;
    std::size_t                                 _length;
};

/// Expected a character of the specified character class.
struct expected_char_class
{};
template <typename Reader>
class error<Reader, expected_char_class>
{
public:
    constexpr explicit error(typename Reader::iterator pos, const char* name) noexcept
    : _pos(pos), _name(name)
    {}

    constexpr auto position() const noexcept
    {
        return _pos;
    }

    constexpr const char* name() const noexcept
    {
        return _name;
    }

private:
    typename Reader::iterator _pos;
    const char*               _name;
};

template <typename Input, typename Tag>
using error_for = error<input_reader<Input>, Tag>;
} // namespace lexy

namespace lexy
{
template <typename Input>
using _detect_parent_input = decltype(LEXY_DECLVAL(Input).parent_input());

template <typename Production, typename Input>
class error_context;

/// Contains information about the context of an error, production is type-erased.
template <typename Input>
class error_context<void, Input>
{
public:
    constexpr explicit error_context(lexy::production_info production, const Input& input,
                                     typename input_reader<Input>::iterator pos) noexcept
    : _input(&input), _pos(pos), _production(production.name)
    {}

    /// The input.
    constexpr const auto& input() const noexcept
    {
        if constexpr (_detail::is_detected<_detect_parent_input, Input>)
            return _input->parent_input();
        else
            return *_input;
    }

    /// The name of the production where the error occurred.
    const char* production() const noexcept
    {
        return _production;
    }

    /// The starting position of the production.
    constexpr auto position() const noexcept
    {
        return _pos;
    }

private:
    const Input*                           _input;
    typename input_reader<Input>::iterator _pos;
    const char*                            _production;
};

/// Contains information about the context of an error.
template <typename Production, typename Input>
class error_context : public error_context<void, Input>
{
public:
    constexpr explicit error_context(const Input&                           input,
                                     typename input_reader<Input>::iterator pos) noexcept
    : error_context(input, Production{}, pos)
    {}
    constexpr explicit error_context(Production production, const Input& input,
                                     typename input_reader<Input>::iterator pos) noexcept
    : error_context<void, Input>(production, input, pos)
    {}

    // We override production to make it static and constexpr.
    static LEXY_CONSTEVAL const char* production()
    {
        return production_name<Production>();
    }
};

template <typename Input>
error_context(production_info, const Input&, typename input_reader<Input>::iterator)
    -> error_context<void, Input>;
} // namespace lexy

#endif // LEXY_ERROR_HPP_INCLUDED

