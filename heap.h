#ifndef HEAP_H
#define HEAP_H

#include <cassert>
#include <climits>
#include <cstdlib>
#include <functional>
#include <stack>

#define EMPTY nullptr
#define DEFAULT_HEAP_SIZE 0

typedef enum { L, R } Direction;

/* Nodes used in the heap. */
template <typename T> class Node {
public:
  /* The data the user wants to store. */
  T data;

  /* The node's children. */
  Node *left, *right;

  /****************************************************************************/

  /* Intantiates a leaf. */
  Node(T data);

  /* Intantiates an internal node. */
  Node(T data, Node *left, Node *right);

  /* Returns the children corresponding to the given direction. */
  Node *get_child(Direction d);

  /* Inserts a children as left or right child, depending on the direction. */
  void insert(Direction d, Node<T> *n);

  /* Deletes the node and his children, recursively. */
  ~Node();
};

template <typename T> class Heap {
public:
  /* Creates a new empty (default: min) heap. */
  Heap();

  /* Creates a new empty heap that uses compare(a, b) to prioritise elements.
   * @param compare Should return true if priority(a) < priority(b).*/
  Heap(bool (*compare)(T, T));

  /* Gets the number of nodes in the heap. */
  uint get_size();

  /* Inserts a new node in the heap. */
  void insert(Node<T> *node);

  /* Deletes and returns the top element. */
  T pop_top();

  /* Returns the top element. */
  T get_top();

  /* Deletes the heap and his tree. */
  ~Heap();

private:
  /* The heap binary-tree. */
  Node<T> *tree;

  /* The number of nodes in the heap binary-tree. */
  uint size;

  /* The function used to compare two elements. */
  std::function<bool(T, T)> compare;

  /****************************************************************************/

  /* Get the directions to get to the last leaf in a
   * quasi-perfect binary-tree of size n.
   */
  std::stack<Direction> get_dirlist(uint n);
  std::stack<Direction> _get_dirlist(uint n, std::stack<Direction>);

  /* Insert a node as leaf after following a list of directions. */
  void insert_leaf(Node<T> *node, uint size, std::stack<Direction> dirlist);
  void _insert_leaf(Node<T> *tree, Node<T> *to_insert,
                    std::stack<Direction> dirlist);

  /* Bubble-up a leaf until it restablishes the heap property. */
  void bubble_up(std::stack<Direction> dirlist);
  void _bubble_up(Node<T> *node, std::stack<Direction> dirlist);

  /* Swap two nodes of the heap. */
  void swap_nodes(Node<T> *a, Node<T> *b);

  /* Deletes from the heap and returns the leaf located at this path. */
  Node<T> pop_leaf(Node<T> *node, std::stack<Direction> dirlist);

  /* Bubbles down the root node until the heap property is restablished. */
  void bubble_down(Node<T> *node);
};

/* NODE ***********************************************************************/

template <typename T> Node<T>::Node(T data) {
  this->data = data;
  left = right = EMPTY;
}

template <typename T> Node<T>::Node(T data, Node *left, Node *right) {
  this->data = data;
  this->left = left;
  this->right = right;
}

template <typename T> Node<T> *Node<T>::get_child(Direction direction) {
  return direction == L ? left : right;
}

template <typename T> void Node<T>::insert(Direction direction, Node<T> *node) {
  direction == L ? left = node : right = node;
}

template <typename T> Node<T>::~Node() {
  if (left != EMPTY)
    delete (left);
  if (right != EMPTY)
    delete (right);
}

/* HEAP ***********************************************************************/

template <typename T> Heap<T>::Heap() {
  compare = [](T a, T b) { return a > b; };
  size = DEFAULT_HEAP_SIZE;
  tree = EMPTY;
}

template <typename T> Heap<T>::Heap(bool compare(T, T)) {
  this->compare = compare;
  size = DEFAULT_HEAP_SIZE;
  tree = EMPTY;
}

template <typename T> void Heap<T>::insert(Node<T> *node) {
  uint size = ++this->size;
  std::stack<Direction> dirlist = get_dirlist(size);
  insert_leaf(node, size, dirlist);
  if (size > 1)
    bubble_up(dirlist);
}

template <typename T> std::stack<Direction> Heap<T>::get_dirlist(uint n) {
  return _get_dirlist(n, {});
}

template <typename T>
std::stack<Direction> Heap<T>::_get_dirlist(uint n,
                                            std::stack<Direction> directions) {
  if (n < 2)
    return directions;
  n % 2 == 0 ? directions.push(L) : directions.push(R);
  return _get_dirlist(n / 2, directions);
}

template <typename T>
void Heap<T>::insert_leaf(Node<T> *node, uint size,
                          std::stack<Direction> dirlist) {
  if (size == 1) {
    tree = node;
    return;
  }
  _insert_leaf(tree, node, dirlist);
}

Direction stack_pop(std::stack<Direction> &stack) {
  auto ret = stack.top();
  stack.pop();
  return ret;
}

template <typename T>
void Heap<T>::_insert_leaf(Node<T> *tree, Node<T> *to_insert,
                           std::stack<Direction> dirlist) {
  assert(!dirlist.empty());
  Direction direction = stack_pop(dirlist);

  if (dirlist.empty()) {
    tree->insert(direction, to_insert);
    return;
  }

  Node<T> *child = tree->get_child(direction);
  _insert_leaf(child, to_insert, dirlist);
}

template <typename T> void Heap<T>::bubble_up(std::stack<Direction> dirlist) {
  _bubble_up(tree, dirlist);
}

template <typename T>
void Heap<T>::_bubble_up(Node<T> *node, std::stack<Direction> dirlist) {
  assert(!dirlist.empty());

  Direction direction = stack_pop(dirlist);
  Node<T> *child = node->get_child(direction);

  T child_data = child->data, root_data = node->data;

  if (dirlist.empty()) {
    if (compare(root_data, child_data))
      swap_nodes(node, child);
    return;
  }

  _bubble_up(child, dirlist);

  child_data = child->data, root_data = node->data;

  if (compare(root_data, child_data))
    swap_nodes(node, child);
}

template <typename T> void Heap<T>::swap_nodes(Node<T> *a, Node<T> *b) {
  assert(a != EMPTY && b != EMPTY);

  T temp = a->data;
  a->data = b->data;
  b->data = temp;
}

template <typename T> T Heap<T>::pop_top() {
  assert(size > 0);

  T data = tree->data;

  if (size == 1) {
    delete (tree);
    tree = EMPTY;
    return data;
  }

  auto dirlist = get_dirlist(size);
  auto leaf = pop_leaf(tree, dirlist);
  size--;
  tree->data = leaf.data;
  bubble_down(tree);

  return data;
}

template <typename T>
Node<T> Heap<T>::pop_leaf(Node<T> *node, std::stack<Direction> dirlist) {
  assert(!dirlist.empty());

  Direction direction = stack_pop(dirlist);
  Node<T> *child = node->get_child(direction);

  if (dirlist.empty()) {
    T data = node->data;
    delete (child);
    node->insert(direction, EMPTY);
    return Node<T>(data);
  }

  return pop_leaf(child, dirlist);
}

template <typename T> void Heap<T>::bubble_down(Node<T> *node) {
  Node<T> *child;
  Direction dir;
  T root_data = node->data;

  if (node->left == EMPTY)
    return;

  if (node->right == EMPTY && node->left != EMPTY) {
    T left_data = node->left->data;
    child = node->left;
    if (compare(root_data, left_data))
      swap_nodes(node, child);
    return;
  }

  T left_data = node->left->data, right_data = node->right->data;

  T comp_with_root = compare(left_data, right_data) ? right_data : left_data;

  if (!compare(root_data, comp_with_root))
    return;

  if (compare(right_data, left_data))
    dir = L;
  else
    dir = R;

  child = node->get_child(dir);
  swap_nodes(node, child);
  bubble_down(child);
}

template <typename T> uint Heap<T>::get_size() { return size; }

template <typename T> T Heap<T>::get_top() {
  assert(!(tree == EMPTY) && "ERROR: Attempting to inser in empty heap.");
  return this->tree->data;
}

template <typename T> Heap<T>::~Heap() { delete (tree); }

#endif