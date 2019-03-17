/* the small file that is used to generate an accurate vibrato table */
#include <cmath>
#include <iostream>

int 
main(void)
{
	const auto pi = 4.0 * std::atan(1.0);
	const auto amplitude = 512.0;
	const auto period = 64;

	for (int i = 0; i < period; ++i) {
		const auto th = 2.0 * pi * i / double(period);
		if (i != 0)
			std::cout << ", ";
		std::cout << std::round(std::sin(th) * amplitude);
	}
	std::cout << "\n";
}


	
