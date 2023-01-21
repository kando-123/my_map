#pragma once
#include <exception>
#include <fstream>
#include <memory>
#include <utility>

#include <iostream>

// to do
// deletion
// serialisation
//   ? order of serialisation to avoid recolouring ?
// operator=

// report

/// Passed as argument to the constructor of exception class 'my_exc'.
enum class error_t
{
	rotation_on_nullptr,
	right_rotation_impossible,
	left_rotation_impossible,
	wrong_direction,
	empty_map,
	out_of_range,
	bad_file,
	no_predecessor,
	no_successor
};

/// Exception class.
class my_exc : public std::exception
{
	error_t problem;
public:
	my_exc(error_t _problem) : problem(_problem) {}
	const char* what()
	{
		switch (problem)
		{
		case error_t::rotation_on_nullptr:
			return "Rotation on the null pointer.";
		case error_t::right_rotation_impossible:
			return "Right rotation impossible.";
		case error_t::left_rotation_impossible:
			return "Left rotation impossible.";
		case error_t::wrong_direction:
			return "Wrong direction. Only LEFT (0) and RIGHT (1) are valid.";
		case error_t::empty_map:
			return "Attempt of performing an action inexecutable on an empty tree.";
		case error_t::out_of_range:
			return "Out of range.";
		case error_t::bad_file:
			return "Error about the file.";
		case error_t::no_predecessor:
			return "No predecessor.";
		case error_t::no_successor:
			return "No successor.";
		default:
			return "Unknown problem.";
		}
	}
};

using colour_t = int8_t;
const colour_t RED = 0, BLACK = 1;
using dir_t = int8_t;
const dir_t LEFT = 0, RIGHT = 1, ROOT = 2;

/** Implementation of a map as a red-black tree.
 @param key_type - the type used as the key
 @mapped_type - the type of data assigned to the keys
*/
template <class key_type, class mapped_type>
class my_map
{
	using value_type = std::pair<key_type, mapped_type>;
	struct node
	{
		value_type data;
		colour_t colour;
		std::shared_ptr<node> parent, child[2];
		node();
		node(value_type _data, std::shared_ptr<node> _parent = nullptr);
	};
	std::shared_ptr<node> root;
	inline dir_t which_child(std::shared_ptr<node> point);
	void rotation(std::shared_ptr<node> point, dir_t dir);
	std::shared_ptr<node> predecessor(std::shared_ptr<node> point);
	std::shared_ptr<node> successor(std::shared_ptr<node> point);

	void print_node(std::shared_ptr<node> point, unsigned& level, dir_t& dir);

public:
	my_map(); // default
	my_map(my_map& other); // copy
	my_map(my_map&& other); // move
	void insert(value_type value);
	void erase(const key_type& key);
	value_type max();
	value_type min();
	mapped_type& at(key_type key);
	void serialize(std::ofstream& file);
	bool empty();

	void print();

	class iterator
	{
		std::shared_ptr<node> my_node;
		iterator(std::shared_ptr<node> _my_node) : my_node(_my_node) {}
	public:
		iterator() {}
		value_type operator*() { return my_node->data; }
		iterator& operator++()
		{
			try { my_node = successor(my_node); }
			catch (...) { my_node = nullptr; }
			return *this;
		}
		iterator& operator--()
		{
			try { my_node = predecessor(my_node); }
			catch (...)
			{
				my_node = root;
				while (my_node->child[RIGHT] != nullptr)
					my_node = my_node->child[RIGHT];
			}
			return *this;
		}
		// iterator operator++(int); ?
		// iterator operator--(int); ?
		bool operator=(value_type value) { *my_node = value; }
		bool operator=(iterator other) { my_node = other.my_node; }
		bool operator==(iterator other) { return my_node == other.my_node; }
		bool operator!=(iterator other) { return my_node == other.my_node; }
		iterator begin() const
		{
			if (root == nullptr) return iterator(nullptr);
			std::shared_ptr<node> ptr = root;
			while (ptr.child[LEFT] != nullptr) ptr = ptr->child[LEFT];
			return iterator(ptr);
		}
		iterator end() const { return iterator(nullptr); }
	};
};

template<class key_type, class mapped_type>
inline dir_t my_map<key_type, mapped_type>::which_child(std::shared_ptr<node> point)
{
	/*if (point->parent == nullptr)
		return ROOT;
	if (point->parent->child[LEFT] == point)
		return LEFT;
	return RIGHT;*/
	return
		point != root ? (point->parent->child[LEFT] == point ? LEFT : RIGHT) : ROOT;
}

template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::rotation(std::shared_ptr<node> point, dir_t dir)
{
	if (point == nullptr)
		throw my_exc(error_t::rotation_on_nullptr);
	if (dir != LEFT and dir != RIGHT)
		throw my_exc(error_t::wrong_direction);
	if (point->child[1 - dir] == nullptr)
	{
		if (dir == LEFT)
			throw my_exc(error_t::left_rotation_impossible);
		else
			throw my_exc(error_t::right_rotation_impossible);
	}
	if (point == root)
	{
		root = point->child[1 - dir];
		root->parent = nullptr;
		point->child[1 - dir] = root->child[dir];
		if (point->child[1 - dir] != nullptr)
			point->child[1 - dir]->parent = point;
		root->child[dir] = point;
		point->parent = root;
	}
	else
	{
		std::shared_ptr<node> ancestor = point->parent;
		dir_t that = which_child(point);
		ancestor->child[that] = point->child[1 - dir];
		ancestor->child[that]->parent = ancestor;
		point->child[1 - dir] = ancestor->child[that]->child[dir];
		if (point->child[1 - dir] != nullptr)
			point->child[1 - dir]->parent = point;
		ancestor->child[that]->child[dir] = point;
		point->parent = ancestor->child[that];
	}
}

template<class key_type, class mapped_type>
std::shared_ptr<typename my_map<key_type, mapped_type>::node> my_map<key_type, mapped_type>::predecessor(std::shared_ptr<node> point)
{
	if (point->child[LEFT] != nullptr)
	{
		point = point->child[LEFT];
		while (point->child[RIGHT] != nullptr)
			point = point->child[RIGHT];
		return point;
	}
	else
	{
		while (true)
		{
			if (point->parent == nullptr)
				throw my_exc(error_t::no_predecessor);
			dir_t that = which_child(point);
			point = point->parent;
			if (that == RIGHT)
				return point;
		}
	}
}

template<class key_type, class mapped_type>
std::shared_ptr<typename my_map<key_type, mapped_type>::node> my_map<key_type, mapped_type>::successor(std::shared_ptr<node> point)
{
	if (point->child[RIGHT] != nullptr)
	{
		point = point->child[RIGHT];
		while (point->child[LEFT] != nullptr)
			point = point->child[LEFT];
		return point;
	}
	else
	{
		while (true)
		{
			if (point->parent == nullptr)
				throw my_exc(error_t::no_successor);
			dir_t that = which_child(point);
			point = point->parent;
			if (that == LEFT)
				return point;
		}
	}
}

template<class key_type, class mapped_type>
my_map<key_type, mapped_type>::node::node()
{
	data = std::make_pair(key_type(), mapped_type());
	colour = RED;
	parent = child[LEFT] = child[RIGHT] = nullptr;
}

template<class key_type, class mapped_type>
my_map<key_type, mapped_type>::node::node(value_type _data, std::shared_ptr<node> _parent) : data(_data), parent(_parent)
{
	colour = RED;
	child[LEFT] = nullptr;
	child[RIGHT] = nullptr;
}

template<class key_type, class mapped_type>
my_map<key_type, mapped_type>::my_map()
{
	root = nullptr;
}

template<class key_type, class mapped_type>
my_map<key_type, mapped_type>::my_map(my_map&& other)
{
	root = other.root;
	other.root = nullptr;
}

template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::insert(value_type value)
{
	if (root == nullptr)
	{
		root = std::shared_ptr<node>(new node(value));
		root->colour = BLACK;
		return;
	}
	std::shared_ptr<node> point = root;
	while (true)
	{
		if (value.first != point->data.first)
		{
			dir_t that = (value.first < point->data.first ? LEFT : RIGHT);
			if (point->child[that] == nullptr)
			{
				point->child[that] = std::shared_ptr<node>(new node(value, point));
				point = point->child[that];
				break;
			}
			else
				point = point->child[that];
		}
		else
			return;
	}
	if (point->parent->colour == RED)
	{
		while (true)
		{
			//if (point->colour == BLACK) // seems
			//	return; //                    useless
			if (point->parent == nullptr)
			{
				point->colour = BLACK;
				return;
			}
			if (point->parent->colour == RED)
			{
				dir_t point_dir = which_child(point),
					parent_dir = which_child(point->parent);
				std::shared_ptr<node> parent = point->parent,
					grandparent = parent->parent,
					uncle = grandparent->child[1 - parent_dir];
				if (uncle == nullptr or uncle->colour == BLACK)
				{
					if (point_dir != parent_dir)
					{
						rotation(parent, 1 - point_dir);
						point.swap(parent);
					}
					rotation(grandparent, 1 - parent_dir);
					parent->colour = BLACK;
					grandparent->colour = RED;
					return;
				}
				parent->colour = BLACK;
				uncle->colour = BLACK;
				grandparent->colour = RED;
				point.swap(grandparent);
			}
			else
				return;
		}
	}
}

template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::erase(const key_type& key)
{
	if (root == nullptr)
		return;
	std::shared_ptr<node> point = root;
	while (point->data.first != key)
	{
		dir_t that = (key < point->data.first ? LEFT : RIGHT);
		if (point->child[that] == nullptr)
			return;
		point = point->child[that];
	}
	dir_t that = which_child(point);
	if (point->child[LEFT] == nullptr)
	{
		if (point->child[RIGHT] == nullptr) // no children
		{
			if (point == root)
				root = nullptr;
			else
				point->parent->child[that] = nullptr;
			point.reset();
		}
		else // only the right child
		{
			if (point == root)
			{
				root = point->child[RIGHT];
				point->child[RIGHT]->parent = nullptr;
			}
			else
			{
				point->parent->child[that] = point->child[RIGHT];
				point->child[RIGHT]->parent = point->parent;
			}
			point.reset();
		}
	}
	else
	{
		if (point->child[RIGHT] == nullptr) // only the left child
		{
			if (point == root)
			{
				root = point->child[LEFT];
				point->child[LEFT]->parent = nullptr;
			}
			else
			{
				point->parent->child[that] = point->child[LEFT];
				point->child[LEFT]->parent = point->parent;
			}
			point.reset();
		}
		else // both children
		{
			std::shared_ptr<node> replacer = predecessor(point);
			std::swap(point->data, replacer->data);
			that = which_child(replacer);
			replacer->parent->child[that] = replacer->child[RIGHT];
			if (replacer->child[RIGHT] != nullptr)
				replacer->child[RIGHT]->parent = replacer->parent;
			replacer.reset();
		}
		// Repair red-black tree properties violation.
	}
}

template<class key_type, class mapped_type>
typename my_map<key_type, mapped_type>::value_type my_map<key_type, mapped_type>::max()
{
	if (root == nullptr)
		throw my_exc(error_t::empty_map);
	std::shared_ptr<node> point = root;
	while (point->child[RIGHT] != nullptr)
		point = point->child[RIGHT];
	return point->data;
}

template<class key_type, class mapped_type>
typename my_map<key_type, mapped_type>::value_type my_map<key_type, mapped_type>::min()
{
	if (root == nullptr)
		throw my_exc(error_t::empty_map);
	std::shared_ptr<node> point = root;
	while (point->child[LEFT] != nullptr)
		point = point->child[LEFT];
	return point->data;
}

template<class key_type, class mapped_type>
mapped_type& my_map<key_type, mapped_type>::at(key_type key)
{
	if (root == nullptr)
		throw my_exc(error_t::empty_map);
	std::shared_ptr<node> point = root;
	while (true)
	{
		if (point->data.first == key)
			return point->data.second;
		dir_t that = (key < point->data.first ? LEFT : RIGHT);
		if (point->child[that] == nullptr)
			throw my_exc(error_t::out_of_range);
		else
			point = point->child[that];
	}
}

template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::serialize(std::ofstream& file)
{
	if (not file.good())
		throw my_exc(error_t::bad_file);
	// ...
}

template<class key_type, class mapped_type>
bool my_map<key_type, mapped_type>::empty()
{
	return !root;
}

template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::print_node(std::shared_ptr<node> point, unsigned& level, dir_t& dir)
{
	for (unsigned i = 1; i < level; ++i)
		std::cout << "      ";
	if (dir == LEFT)
		std::cout << " <l> ";
	else if (dir == RIGHT)
		std::cout << " <r> ";
	if (point == nullptr)
	{
		std::cout << "nul" << std::endl;
		--level;
	}
	else
	{
		std::cout << "[" << point->data.first << ", " << point->data.second << "]; ";
		if (point->colour == RED)
			std::cout << "RED" << std::endl;
		else
			std::cout << "BLACK" << std::endl;
		dir = LEFT; print_node(point->child[LEFT], ++level, dir);
		dir = RIGHT; print_node(point->child[RIGHT], ++level, dir);
		--level;
	}
}

template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::print()
{
	std::cout << "root: ";
	unsigned depth = 0;
	dir_t which = ROOT;
	print_node(root, depth, which);
}
