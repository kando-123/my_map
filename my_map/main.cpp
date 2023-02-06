#include "my_map.cpp"
#include <iostream>
#include <string>
#include <utility>

int main()
{
	my_map<int, std::string> example;
	std::pair<int, std::string> element;
	std::string text;
	char choice = 0;
	bool quit = false;
	do {
		std::cout << "> ";
		std::cin >> choice;
		switch (choice)
		{
		case 'p':
			example.print();
			break;
		case 'i':
			std::cout << "key = "; if (not (std::cin >> element.first)) return 0;
			std::cout << "value = "; if (not (std::cin >> element.second)) return 0;
			example.insert(element);
			std::cout << std::endl;
			break;
		case 'm':
			element = example.min();
			std::cout << "min: [" << element.first << ", " << element.second << "]" << std::endl << std::endl;
			break;
		case 'M':
			element = example.max();
			std::cout << "max: [" << element.first << ", " << element.second << "]" << std::endl << std::endl;
			break;
		case 'a':
			std::cout << "key = "; if (not (std::cin >> element.first)) return 0;
			try
			{
				element.second = example.at(element.first);
				std::cout << "map.at(" << element.first << ") = " << element.second << std::endl << std::endl;
			}
			catch (my_exc e)
			{
				std::cout << e.what() << std::endl << std::endl;
			}
			break;
		case 'e':
			std::cout << "key = "; std::cin >> element.first;
			example.erase(element.first);
			std::cout << std::endl;
			break;
		case 's':
			getchar();
			std::cout << "file name = ";
			std::getline(std::cin, text);
			example.serialize(text);
			break;
		case 'd':
			getchar();
			std::cout << "file name = ";
			std::getline(std::cin, text);
			example.deserialize(text);
			break;
		case 'r':
			element.first = rand();
			element.second = "rand";
			example.insert(element);
			break;
		case 'q':
			quit = true;
		}
	} while (not quit);
	return 0;
}