#include "my_map.cpp"
#include <iostream>
#include <string>
#include <utility>

int main()
{
	my_map<int, std::string> example;
	std::pair<int, std::string> z;
	while (true)
	{
		std::cout << "x="; if (not (std::cin >> z.first)) return 0;
		std::cout << "y="; if (not (std::cin >> z.second)) return 0;
		example.insert(z);
		example.print();
		std::cout << std::endl;
	}
	return 0;
}