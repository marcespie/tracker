#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H
/* parse_options.h */
/*
 * Copyright (c) 2019 Marc Espie <espie@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <variant>
#include <unordered_map>
#include <vector>

using VALUE = std::variant<long, const char *>;

class option_init {
public:
	const char *optiontext;
	char type;
	unsigned long def_scalar;
	const char *def_string;
	const char *multi;
	option_init(const char *optiontext_, char type_,
	    unsigned long def_scalar_ =0,
	    const char *def_string_ =nullptr,
	    const char *multi_ =nullptr): 
		optiontext{optiontext_},
		type{type_},
		def_scalar{def_scalar_},
		def_string{def_string_},
		multi{multi_}
	{
	}

};

class option {
public:
	char type;
	unsigned long def_scalar;
	const char *def_string;
	const char *multi;
	VALUE arg;
	void finish_setup(const char* optiontext);
	option()
	{
	}
	option(const option_init& i):
	    type{i.type},
	    def_scalar{i.def_scalar},
	    def_string{i.def_string},
	    multi{i.multi}
	{
		finish_setup(i.optiontext);
	}
};

using parm_function = std::function<void(const char *)>;
class option_set {
public:
	std::unordered_map<const char *, option> options;
	option& operator[](const char* s)
	{
		return options[s];
	}
	option_set(std::initializer_list<option_init> l)
	{
		add(l);
	}
	void add(std::initializer_list<option_init> l)
	{
		for (auto& i: l)
			options[i.optiontext] = i;
	}
	long get_long(const char *t)
	{
		return std::get<long>(options[t].arg);
	}
	const char *get_string(const char *t)
	{
		return std::get<const char *>(options[t].arg);
	}
	template<class iterator>
	void parse(iterator b, iterator e, parm_function what_to_do);
	int do1(const char* text, const char* arg);
};

extern std::vector<char *>string2args(char *);

#endif
