class audio {
	unsigned long ask_freq = 0;
	unsigned long real_freq;
	bool want_stereo = true;
	bool has_stereo = false;
	bool opened = false;
	int mix_percent = 0;
public:
	~audio();
	void open();
	void set_mix(int mix);
	void set_freq(unsigned long freq);
	void set_stereo(bool s);
	void output(int32_t left, int32_t right, int n);
	void flush();
	void discard();
	void sync(std::function<void()>, std::function<void()>);
};
