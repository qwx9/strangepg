typedef struct mt19937_64 mt19937_64;

struct mt19937_64 {
	unsigned long long mt[312];
	int mti;
};

void init_genrand64(mt19937_64 *context, unsigned long long seed);
void init_by_array64(mt19937_64 *context, unsigned long long init_key[], unsigned long long key_length);
unsigned long long genrand64_int64(mt19937_64 *context);
long long genrand64_int63(mt19937_64 *context);
double genrand64_real1(mt19937_64 *context);
double genrand64_real2(mt19937_64 *context);
double genrand64_real3(mt19937_64 *context);
