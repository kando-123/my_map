#pragma once
#include <exception>
#include <memory>
#include <utility>

enum class error_t
{
	rotation_on_nullptr,
	right_rotation_impossible,
	left_rotation_impossible,
	wrong_direction
};

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
		default:
			return "Unknown problem.";
		}
	}
};

using colour_t = int8_t;
const colour_t RED = 0, BLACK = 1;
using dir_t = int8_t;
const dir_t LEFT = 0, RIGHT = 1, ROOT = 2;

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
	dir_t which_child(std::shared_ptr<node> point);
	void rotation(std::shared_ptr<node> point, dir_t dir);
	void correction(std::shared_ptr<node> point);
	void print_node(std::shared_ptr<node> point, unsigned& level, dir_t& dir);
public:
	my_map();
	my_map(my_map&& other);
	void insert(value_type value);
	void print();
};

template<class key_type, class mapped_type>
dir_t my_map<key_type, mapped_type>::which_child(std::shared_ptr<node> point)
{
	if (point->parent == nullptr)
		return ROOT;
	if (point->parent->child[LEFT] == point)
		return LEFT;
	return RIGHT;
}

template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::correction(std::shared_ptr<node> point)
{
	while (true)
	{
		if (point->colour == BLACK)
			return;
		if (point == root)
		{
			point->colour = BLACK;
			return;
		}
		if (point->parent->colour == RED)
		{
			dir_t point_dir = which_child(point), parent_dir = which_child(point->parent);
			std::shared_ptr<node> parent = point->parent, grandparent = parent->parent,
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
			point = grandparent;
		}
	}
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
	if (point = root)
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
		if (value.first < point->data.first)
		{
			if (point->child[LEFT] == nullptr)
			{
				point->child[LEFT] = std::shared_ptr<node>(new node(value, point));
				point = point->child[LEFT];
				break;
			}
			else
				point = point->child[LEFT];
		}
		else if (value.first > point->data.first)
		{
			if (point->child[RIGHT] == nullptr)
			{
				point->child[RIGHT] = std::shared_ptr<node>(new node(value, point));
				point = point->child[RIGHT];
				break;
			}
			else
				point = point->child[RIGHT];
		}
		else
			return;
	}
	if (point->parent->colour == RED)
		correction(point);
}
#include <iostream>
template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::print_node(std::shared_ptr<node> point, unsigned& level, dir_t& dir)
{
	for (unsigned i = 1; i < level; ++i)
		std::cout << "      ";
	if (dir == LEFT)
		std::cout << "(l) ";
	else if (dir == RIGHT)
		std::cout << "(r) ";
	if (point == nullptr)
	{
		std::cout << "nul" << std::endl;
		--level;
	}
	else
	{
		std::cout << "(" << point->data.first << ", " << point->data.second << "); ";
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