#include "my_map.cpp"
#include <iostream>
#include <string>
#include <utility>

int main()
{
	my_map<int, std::string> example;
	std::pair<int, std::string> z;
	char c = 0;
	bool quit = false;
	do {
		std::cin >> c;
		switch (c)
		{
		case 'i':
			std::cout << "x="; if (not (std::cin >> z.first)) return 0;
			std::cout << "y="; if (not (std::cin >> z.second)) return 0;
			example.insert(z);
			example.print();
			std::cout << std::endl;
			break;
		case 'm':
			z = example.min();
			std::cout << "min: [" << z.first << ", " << z.second << "]" << std::endl << std::endl;
			break;
		case 'M':
			z = example.max();
			std::cout << "max: [" << z.first << ", " << z.second << "]" << std::endl << std::endl;
			break;
		case 'a':
			std::cout << "x="; if (not (std::cin >> z.first)) return 0;
			try
			{
				z.second = example.at(z.first);
				std::cout << "map.at(" << z.first << ") = " << z.second << std::endl << std::endl;
			}
			catch (my_exc e)
			{
				std::cout << e.what() << std::endl << std::endl;
			}
			break;
		case 'e':
			std::cout << "x="; std::cin >> z.first;
			example.erase(z.first);
			example.print();
			std::cout << std::endl;
			break;
		case 'q':
			quit = true;
		}
	} while (not quit);
	return 0;
}