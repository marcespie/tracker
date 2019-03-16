/* open.h */

class exfile {
public:
	exfile(): handle{nullptr} {}
	
	bool open(const char* fname, const char* path);
	int getc();
	size_t tell() const;
	unsigned long read(void *, size_t, unsigned long);
	template<typename T>
	unsigned  long read(T* p, unsigned long n)
	{
		return read(static_cast<void *>(p), sizeof(*p), n);
	}
	void rewind();
	~exfile();

private:
	exfile(const exfile&) = delete;
	FILE *handle;	/* the real Mc Coy */
};
