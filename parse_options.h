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
#include <map>

using VALUE = std::variant<long, const char *>;

struct option {
	const char *optiontext;
	char type;
	unsigned long def_scalar;
	const char *def_string;
	const char *multi;
	VALUE arg;
	void finish_setup();
	option(const char *optiontext_, char type_,
	    unsigned long def_scalar_ =0,
	    const char *def_string_ =nullptr,
	    const char *multi_ =nullptr): 
		optiontext{optiontext_},
		type{type_},
		def_scalar{def_scalar_},
		def_string{def_string_},
		multi{multi_}
		{
			finish_setup();
		}

};

struct option_set {
	std::map<const char *, option *> options;
	option& operator[](const char* s)
	{
		return *options[s];
	}
	template<class T>
	option_set(T b, T e)
	{
		for (auto i = b; i != e; ++i)
			options[i->optiontext] = i;
	}
	long get_long(const char *t)
	{
		return std::get<long>(options[t]->arg);
	}
	const char *get_string(const char *t)
	{
		return std::get<const char *>(options[t]->arg);
	}
	void parse(int argc, char *argv[], void (*what_to_do) (const char *arg));
	int do1(char* text, char* arg);
};
