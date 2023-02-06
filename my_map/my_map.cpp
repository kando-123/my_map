#pragma once
#include <exception>
#include <fstream>
#include <memory>
#include <utility>

#include <iostream>

/// Passed as argument to the constructor of exception class 'my_exc'.
enum class error_t
{
	rotation_on_nullptr,
	right_rotation_impossible,
	left_rotation_impossible,
	wrong_direction,
	empty_map,
	non_empty_map,
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
		case error_t::non_empty_map:
			return "Attempt of performing an action inexecutable on a non empty tree.";
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
const colour_t RED = 0, BLACK = 1, DOUBLE_BLACK = 2;
using dir_t = int8_t;
const dir_t LEFT = 0, RIGHT = 1, ROOT = 2;

/** Implementation of a map as a red-black tree.
* @param key_type - the type used as the key
* @mapped_type - the type of data assigned to the keys
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
	size_t number_of_nodes;
	inline dir_t which_child(std::shared_ptr<node> point);
	void rotation(std::shared_ptr<node> point, dir_t dir);
	std::shared_ptr<node> predecessor(std::shared_ptr<node> point);
	std::shared_ptr<node> successor(std::shared_ptr<node> point);
	void print_node(std::shared_ptr<node> point, unsigned& level, unsigned& black_depth, dir_t& dir);


public:
	my_map();
	my_map(my_map&& other);
	void insert(value_type value);
private:
	void resolve_double_black(std::shared_ptr<node>& point);
	void erase(std::shared_ptr<node>& point);
public:
	void erase(const key_type& key);
	value_type max();
	value_type min();
	mapped_type& at(key_type key);
private:
	void serialize(std::shared_ptr<node> point, std::ofstream& file);
public:
	void serialize(const std::string& name);
	void deserialize(const std::string& name);
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

/** Checks the node indicated by 'point' whether it is its parent's right or left child.
* @param std::shared_ptr<my_map<key_type, mapped_type>::node> point - the shared pointer to the node to be checked
* @return constant value of type int8_t: LEFT (0) or RIGHT (1) denoting which child the node is
* or ROOT (2) if the node has no parent
*/
template<class key_type, class mapped_type>
inline dir_t my_map<key_type, mapped_type>::which_child(std::shared_ptr<node> point)
{
	return
		point != root ? (point->parent->child[LEFT] == point ? LEFT : RIGHT) : ROOT;
}

/** Performs a rotation on the node pointed by 'point' in direction 'dir'.
* @param std::shared_ptr<my_map<key_type, mapped_type>::node> point - the pointer to the node the rotation should be performed on
* @param int8_t dir - the direction of rotation: LEFT (0) or RIGHT (1)
*/
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

/** Returns the pointer to predecessor of the node indicated by 'point', i.e. that one that has the greatest key that is lesser than this node's key.
* @param std::shared_ptr<my_map<key_type, mapped_type>::node> point - the pointer to the node whose predecessor should be found
* @return a shared pointer of type std::shared_ptr<my_map<key_type, mapped_type>::node> to the predecessor
*/
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

/** Returns the pointer to successor of the node indicated by 'point', i.e. that one that has the least key that is greater than this node's key.
* @param std::shared_ptr<my_map<key_type, mapped_type>::node> point - the pointer to the node whose successor should be found
* @return a shared pointer of type std::shared_ptr<my_map<key_type, mapped_type>::node> to the successor
*/
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

/// Internal class of my_map<key_type, mapped_type> used to store nodes' data
template<class key_type, class mapped_type>
my_map<key_type, mapped_type>::node::node()
{
	data = std::make_pair(key_type(), mapped_type());
	colour = RED;
	parent = child[LEFT] = child[RIGHT] = nullptr;
}

/** Constructor of class my_map<key_type, mapped_type>::node. The colour is assigned to RED (0) and the children to nullptr.
* @param std::pair<key_type, mapped_type> _data - the data to be stored in the node
* @param std::shared_ptr<my_map<key_type, mapped_type>::node> _parent - the pointer to the node's parent, nullptr by default
*/
template<class key_type, class mapped_type>
my_map<key_type, mapped_type>::node::node(value_type _data, std::shared_ptr<node> _parent) : data(_data), parent(_parent)
{
	colour = RED;
	child[LEFT] = nullptr;
	child[RIGHT] = nullptr;
}

/// Constructor of class my_map<key_type, mapped_type>. Assigns the root with nullptr.
template<class key_type, class mapped_type>
my_map<key_type, mapped_type>::my_map()
{
	root = nullptr;
	number_of_nodes = 0;
}

/** Move constructor of class my_map<key_type, mapped_type>. Takes over the contents of 'other'.
* @param my_map<key_type, mapped_type>&& other - an rvalue map whose contents should be taken
*/
template<class key_type, class mapped_type>
my_map<key_type, mapped_type>::my_map(my_map&& other)
{
	root = other.root;
	other.root = nullptr;
	number_of_nodes = other.number_of_nodes;
}

/** Performs insertion into the map.
* @param std::pair<key_type, mapped_type> value
* @return void
*/
template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::insert(value_type value)
{
	if (root == nullptr)
	{
		root = std::shared_ptr<node>(new node(value));
		root->colour = BLACK;
		++number_of_nodes;
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
				++number_of_nodes;
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

/** Resolves the node marked as double black.
* @param std::shared_ptr<my_map<key_type, mapped_type>::node>& point - shared pointer to the double black node
* @return void
*/
template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::resolve_double_black(std::shared_ptr<node>& point)
{
	if (point == nullptr or point->colour != DOUBLE_BLACK)
		return;
	if (point == root)
	{
		// case 1
		std::cout << "Case 1" << std::endl;
		root->colour = BLACK;
		return;
	}
	dir_t that = which_child(point);
	std::shared_ptr<node> parent = point->parent, sibling = parent->child[1 - that];
	if (sibling == nullptr or sibling->colour == BLACK)
	{
		if (sibling->child[1 - that] == nullptr or sibling->child[1 - that]->colour == BLACK)
		{
			if (parent->colour == BLACK)
			{
				if (sibling->child[that] == nullptr or sibling->child[that]->colour == BLACK)
				{
					// case 3
					std::cout << "Case 3" << std::endl;
					point->colour = BLACK;
					parent->colour = DOUBLE_BLACK;
					sibling->colour = RED;
					resolve_double_black(parent);
				}
				else
				{
					// case 5
					std::cout << "Case 5" << std::endl;
					sibling->child[that]->colour = BLACK;
					sibling->colour = RED;
					rotation(sibling, 1 - that);
					resolve_double_black(point);
				}
			}
			else
			{
				// case 4
				std::cout << "Case 4" << std::endl;
				point->colour = BLACK;
				parent->colour = BLACK;
				sibling->colour = RED;
				return;
			}
		}
		else
		{
			// case 6
			std::cout << "Case 6" << std::endl;
			rotation(parent, that);
			point->colour = BLACK;
			sibling->colour = parent->colour;
			parent->colour = BLACK;
			sibling->child[1 - that]->colour = BLACK;
			parent->colour = BLACK;
			return;
		}
	}
	else
	{
		// case 2
		std::cout << "Case 2" << std::endl;
		rotation(parent, that);
		point->colour = BLACK;
		sibling->colour = BLACK;
		parent->colour = RED;
		resolve_double_black(point);
	}
}

/** Erases the node indicated by 'point'.
* @param std::shared_ptr<my_map<key_type, mapped_type>::node>& point - shared pointer to the double black node
* @return void
*/
template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::erase(std::shared_ptr<node>& point)
{
	if (point->child[LEFT] == nullptr)
	{
		if (point->child[RIGHT] == nullptr) // no children
		{
			if (point == root)
			{
				root.reset();
				point.reset();
			}
			else
			{
				if (point->colour == BLACK)
				{
					point->colour = DOUBLE_BLACK;
					resolve_double_black(point);
				}
				point->parent->child[which_child(point)] = nullptr;
				point.reset();
			}
		}
		else // right child only
		{
			if (point == root)
			{
				root = point->child[RIGHT];
				point->child[RIGHT]->parent = nullptr;
				point.reset();
			}
			else
			{
				if (point->child[RIGHT]->colour == RED)
				{
					point->child[RIGHT]->colour = BLACK;
					point->parent->child[which_child(point)] = point->child[RIGHT];
					point->child[RIGHT]->parent = point->parent;
					point.reset();
				}
				else
				{
					point->child[RIGHT]->colour = DOUBLE_BLACK;
					point->parent->child[which_child(point)] = point->child[RIGHT];
					point->child[RIGHT]->parent = point->parent;
					resolve_double_black(point->child[RIGHT]);
					point.reset();
				}
			}
		}
	}
	else
	{
		if (point->child[RIGHT] == nullptr) // left child only
		{
			if (point == root)
			{
				root = point->child[LEFT];
				point->child[LEFT]->parent = nullptr;
				point.reset();
			}
			else
			{
				if (point->child[LEFT]->colour == RED)
				{
					point->child[LEFT]->colour = BLACK;
					point->parent->child[which_child(point)] = point->child[LEFT];
					point->child[LEFT]->parent = point->parent;
					point.reset();
				}
				else
				{
					point->child[LEFT]->colour = DOUBLE_BLACK;
					point->parent->child[which_child(point)] = point->child[LEFT];
					point->child[LEFT]->parent = point->parent;
					resolve_double_black(point->child[LEFT]);
					point.reset();
				}
			}
		}
		else // both children
		{
			std::shared_ptr<node> replacer = predecessor(point);
			std::swap(replacer->data, point->data);
			erase(replacer);
		}
	}
}

/** Erases the node that holds 'key' as key if such is present in the tree.
* @param const key_type& key - the key of the node to be erased
* @return void
*/
template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::erase(const key_type& key)
{
	if (root == nullptr)
		return;
	std::shared_ptr<node> point = root;
	while (key != point->data.first)
	{
		if (key < point->data.first)
		{
			if (point->child[LEFT] == nullptr)
				return;
			else
				point = point->child[LEFT];
		}
		else
		{
			if (point->child[RIGHT] == nullptr)
				return;
			else
				point = point->child[RIGHT];
		}
	}
	erase(point);
}

/** Returns the pair of the maximal key and its mapped value.
* @return pair of type std::pair<key_type, mapped_type> of the maximal key and its mapped value
*/
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

/** Returns the pair of the minimal key and its mapped value.
* @return pair of type std::pair<key_type, mapped_type> of the minimal key and its mapped value
*/
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

/** Accesses the value assigned to key 'key'. Throws if such key is not present.
* @param key_type key - the key that to which the value is assigned whose reference should be accessed
* @return the value of type mapped_type mapped to the key
*/
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
void my_map<key_type, mapped_type>::serialize(std::shared_ptr<node> point, std::ofstream& file)
{
	if (point != nullptr)
	{
		file << point->data.first << ' ' << point->data.second << '\n';
		if (point->child[LEFT] != nullptr)
			serialize(point->child[LEFT], file);
		if (point->child[RIGHT] != nullptr)
			serialize(point->child[RIGHT], file);
	}
}

/** Serializes the contents of the map to the output file stream.
* @param ofstream& file - the output file stream the contents are serialized to
* @return void
*/
// Under development.
template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::serialize(const std::string& name)
{
	std::ofstream file;
	file.open(name, std::ios::out);
	if (file.good())
		serialize(root, file);
	file.close();
}

template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::deserialize(const std::string& name)
{
	std::ifstream file;
	file.open(name, std::ios::out);
	if (file.good())
	{
		value_type value;
		while ((file >> value.first) and (file >> value.second))
			insert(value);
	}
	file.close();
}

/** Returns the information whether the map is empty.
* @return true if the map is empty, false otherwise
*/
template<class key_type, class mapped_type>
bool my_map<key_type, mapped_type>::empty()
{
	return !root;
}

/** Prints information about the node and its children to the console.
* @param std::shared_ptr<my_map<key_type, mapped_type>::node> point - the pointer to the node to be printed
* @param unsigned& level - the number of the node's ancestors
* @param int8_t dir - information wheter the node is the left (LEFT) or the right child (RIGHT)
* @return void
*/
template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::print_node(std::shared_ptr<node> point, unsigned& level, unsigned& black_depth, dir_t& dir)
{
	for (unsigned i = 1; i < level; ++i)
		std::cout << "      ";
	if (dir == LEFT)
		std::cout << " <l> ";
	else if (dir == RIGHT)
		std::cout << " <r> ";
	if (point == nullptr)
	{
		std::cout << "nul, " << black_depth + 1 << std::endl;
		--level;
	}
	else
	{
		std::cout << "[" << point->data.first << ", " << point->data.second << "], ";
		if (point->colour == RED)
			std::cout << "red" << std::endl;
		else if (point->colour == BLACK)
			std::cout << "black" << std::endl;
		else
			std::cout << "double_black" << std::endl;
		dir = LEFT;
		if (point->colour == BLACK)
			++black_depth;
		print_node(point->child[LEFT], ++level, black_depth, dir);
		dir = RIGHT;
		print_node(point->child[RIGHT], ++level, black_depth, dir);
		--level;
		if (point->colour == BLACK)
			--black_depth;
	}
}

/** Prints the contents of the map to the console.
* @return void
*/
template<class key_type, class mapped_type>
void my_map<key_type, mapped_type>::print()
{
	std::cout << "root: ";
	unsigned depth = 0, black_height = 0;
	dir_t which = ROOT;
	print_node(root, depth, black_height, which);
}
