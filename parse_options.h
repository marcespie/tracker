/* parse_options.h */

#include <variant>

using VALUE = std::variant<long, const char *>;

struct option {
	const char *optiontext;
	char type;
	unsigned long def_scalar;
	const char *def_string;
	int multi;
	option(const char *optiontext_, char type_,
	    unsigned long def_scalar_ =0,
	    const char *def_string_ =nullptr,
	    int multi_ =0): 
		optiontext{optiontext_},
		type{type_},
		def_scalar{def_scalar_},
		def_string{def_string_},
		multi{multi_}
		{}
};

struct option_set {
	struct option *options;
	int number;
	VALUE *args;
	long get_long(int n) const 
	{
		return std::get<long>(args[n]);
	}
	const char *get_string(int n) const 
	{
		return std::get<const char *>(args[n]);
	}
};
