/* parse_options.h */

struct option {
	const char *optiontext;
	char type;
	unsigned long def_scalar;
	const char *def_string;
	int multi;
};

struct option_set {
	struct option *options;
	int number;
	VALUE *args;
};

