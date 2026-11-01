#include <iostream>
#include <vector>
#include <string>
using namespace std;


template <typename Type, int Ord>
struct Node
{
    bool is_leaf;
    int num_keys;
    Type key[Ord];
};

template <typename Type, int Ord>
struct LeafNode : public Node<Type, Ord>
{
};

template <typename Type, int Ord>
struct InternalNode : public Node<Type, Ord>
{
    Node<Type, Ord>* child[Ord + 1];
};

template <typename Type, int Ord>
struct BTree
{
private:

    void split(InternalNode<Type, Ord>* parent, int child_index)
    {
        Node<Type, Ord>* node = parent->child[child_index];
        int median = Ord / 2;

        Node<Type, Ord>* right;
        if (node->is_leaf)
        {
            right = new LeafNode<Type, Ord>;
        }
        else
        {
            right = new InternalNode<Type, Ord>;
        }

        right->is_leaf = node->is_leaf;
        right->num_keys = node->num_keys - median - 1;

        // move to the new node
        for (int i = 0; i < right->num_keys; i++)
        {
            right->key[i] = node->key[median + 1 + i];
        }

        // move the childs also
        if (!node->is_leaf)
        {
            InternalNode<Type, Ord>* left_internal_node = static_cast<InternalNode<Type, Ord>*>(node);
            InternalNode<Type, Ord>* right_internal_node = static_cast<InternalNode<Type, Ord>*>(right);
            for (int i = 0; i <= right->num_keys; i++)
            {
                right_internal_node->child[i] = left_internal_node->child[median + i + 1];
            }
        }

        Type median_key = node->key[median];
        node->num_keys = median;

        for (int j = parent->num_keys; j > child_index; --j)
        {
            parent->key[j] = parent->key[j - 1];
        }
        for (int j = parent->num_keys + 1; j > child_index + 1; --j)
        {
            parent->child[j] = parent->child[j - 1];
        }

        parent->key[child_index] = median_key;
        parent->child[child_index + 1] = right;
        parent->num_keys += 1;
    }

    // minimum number of keys in any non-root node
    static const int MIN_KEYS = (Ord - 1) / 2;

    // returns the largest key in the subtree (go to the rightmost leaf)
    Type get_predecessor(Node<Type, Ord>* node)
    {
        while (!node->is_leaf)
        {
            InternalNode<Type, Ord>* in = static_cast<InternalNode<Type, Ord>*>(node);
            node = in->child[in->num_keys];
        }
        return node->key[node->num_keys - 1];
    }

    // child[idx] is underfull: take one key from the left sibling through the parent
    void borrow_from_left(InternalNode<Type, Ord>* parent, int idx)
    {
        Node<Type, Ord>* child = parent->child[idx];
        Node<Type, Ord>* left = parent->child[idx - 1];

        // case 1: make room at the front of the child
        for (int j = child->num_keys; j > 0; j--)
        {
            child->key[j] = child->key[j - 1];
        }

        // case 2: internal nodes also shift children and take the left sibling's last child
        if (!child->is_leaf)
        {
            InternalNode<Type, Ord>* c = static_cast<InternalNode<Type, Ord>*>(child);
            InternalNode<Type, Ord>* l = static_cast<InternalNode<Type, Ord>*>(left);
            for (int j = child->num_keys + 1; j > 0; j--)
            {
                c->child[j] = c->child[j - 1];
            }
            c->child[0] = l->child[left->num_keys];
        }

        // case 3: separator moves down, left sibling's last key moves up
        child->key[0] = parent->key[idx - 1];
        parent->key[idx - 1] = left->key[left->num_keys - 1];
        child->num_keys += 1;
        left->num_keys -= 1;
    }

    // child[idx] is underfull: take one key from the right sibling through the parent
    void borrow_from_right(InternalNode<Type, Ord>* parent, int idx)
    {
        Node<Type, Ord>* child = parent->child[idx];
        Node<Type, Ord>* right = parent->child[idx + 1];

        // case 1: separator moves down to the end of the child
        child->key[child->num_keys] = parent->key[idx];

        // case 2: internal nodes take the right sibling's first child
        if (!child->is_leaf)
        {
            InternalNode<Type, Ord>* c = static_cast<InternalNode<Type, Ord>*>(child);
            InternalNode<Type, Ord>* r = static_cast<InternalNode<Type, Ord>*>(right);
            c->child[child->num_keys + 1] = r->child[0];
        }

        // case 3: right sibling's first key moves up to the parent
        parent->key[idx] = right->key[0];

        // case 4: close the gap in the right sibling (keys, then children if internal)
        for (int j = 0; j < right->num_keys - 1; j++)
        {
            right->key[j] = right->key[j + 1];
        }
        if (!right->is_leaf)
        {
            InternalNode<Type, Ord>* r = static_cast<InternalNode<Type, Ord>*>(right);
            for (int j = 0; j < right->num_keys; j++)
            {
                r->child[j] = r->child[j + 1];
            }
        }

        child->num_keys += 1;
        right->num_keys -= 1;
    }

    // merge child[idx] and child[idx + 1] into child[idx]; the separator comes down from the parent
    void merge(InternalNode<Type, Ord>* parent, int idx)
    {
        Node<Type, Ord>* left = parent->child[idx];
        Node<Type, Ord>* right = parent->child[idx + 1];

        // case 1: separator moves down to the end of the left node
        left->key[left->num_keys] = parent->key[idx];

        // case 2: copy all keys of the right node after it
        for (int j = 0; j < right->num_keys; j++)
        {
            left->key[left->num_keys + 1 + j] = right->key[j];
        }

        // case 3: internal nodes also copy the children of the right node
        if (!left->is_leaf)
        {
            InternalNode<Type, Ord>* l = static_cast<InternalNode<Type, Ord>*>(left);
            InternalNode<Type, Ord>* r = static_cast<InternalNode<Type, Ord>*>(right);
            for (int j = 0; j <= right->num_keys; j++)
            {
                l->child[left->num_keys + 1 + j] = r->child[j];
            }
        }

        // case 4: update the key count (left keys + separator + right keys)
        left->num_keys += right->num_keys + 1;

        // case 5: remove the separator and the right child pointer from the parent
        for (int j = idx; j < parent->num_keys - 1; j++)
        {
            parent->key[j] = parent->key[j + 1];
        }
        for (int j = idx + 1; j < parent->num_keys; j++)
        {
            parent->child[j] = parent->child[j + 1];
        }
        parent->num_keys -= 1;

        // case 6: free the right node
        if (right->is_leaf)
        {
            delete static_cast<LeafNode<Type, Ord>*>(right);
        }
        else
        {
            delete static_cast<InternalNode<Type, Ord>*>(right);
        }
    }

    // fix child[idx] if it has fewer than MIN_KEYS keys: borrow first, merge if borrowing is impossible
    void fix_child(InternalNode<Type, Ord>* parent, int idx)
    {
        // case 1: child is fine, nothing to do
        if (parent->child[idx]->num_keys >= MIN_KEYS)
        {
            return;
        }

        // case 2: left sibling has a spare key
        if (idx > 0 && parent->child[idx - 1]->num_keys > MIN_KEYS)
        {
            borrow_from_left(parent, idx);
        }
        // case 3: right sibling has a spare key
        else if (idx < parent->num_keys && parent->child[idx + 1]->num_keys > MIN_KEYS)
        {
            borrow_from_right(parent, idx);
        }
        // case 4: no spare keys, merge with the right sibling
        else if (idx < parent->num_keys)
        {
            merge(parent, idx);
        }
        // case 5: no right sibling, merge with the left sibling
        else
        {
            merge(parent, idx - 1);
        }
    }

    // recursive delete: remove from the subtree, then fix any underflow on the way back up
    // (the caller already checked with Search that the value exists in the tree)
    void remove(Node<Type, Ord>* node, Type value)
    {
        // index of the first key >= value
        int i = 0;
        while (i < node->num_keys) // style fix: moved '&& value > node->key[i]' inside loop as break
        {
            if (value <= node->key[i]) break;
            i++;
        }

        // case 1: leaf node, remove the key (the parent fixes any underflow)
        if (node->is_leaf)
        {
            for (int j = i; j < node->num_keys - 1; j++)
            {
                node->key[j] = node->key[j + 1];
            }
            node->num_keys -= 1;
            return;
        }

        InternalNode<Type, Ord>* internal = static_cast<InternalNode<Type, Ord>*>(node);

        // case 2: the key is in this internal node, replace it with its predecessor and delete the predecessor below
        if (i < node->num_keys && node->key[i] == value)
        {
            Type pred = get_predecessor(internal->child[i]);
            node->key[i] = pred;
            remove(internal->child[i], pred);
        }
        // case 3: the key is not here, go down into the right child
        else
        {
            remove(internal->child[i], value);
        }

        // after returning, fix the child we went into if it became underfull
        fix_child(internal, i);
    }

public:

    Node<Type, Ord>* root;

    BTree()
    {
        root = nullptr;
    }

    void Insert(Type value)
    {

        // tree is empty
        if (root == nullptr)
        {
            LeafNode<Type, Ord>* New_Leaf_Node = new LeafNode<Type, Ord>;
            New_Leaf_Node->is_leaf = true;
            New_Leaf_Node->key[0] = value;
            New_Leaf_Node->num_keys = 1;

            root = New_Leaf_Node;
            return;
        }

        // case2 root is full
        if (root->num_keys == Ord)
        {
            InternalNode<Type, Ord>* new_root = new InternalNode<Type, Ord>;
            new_root->is_leaf = false;
            new_root->num_keys = 0;
            new_root->child[0] = root;
            root = new_root;
            split(new_root, 0);
        }

        Node<Type, Ord>* current = root;
        while (!current->is_leaf)
        {
            // cast to access internal nodes childs
            InternalNode<Type, Ord>* internal = static_cast<InternalNode<Type, Ord>*>(current);

            int i = 0;
            while (i < current->num_keys) // style fix: moved '&& value > current->key[i]' inside loop as break
            {
                if (value <= current->key[i]) break;
                i++;
            }

            // DUPLICATES
            if (i < current->num_keys && current->key[i] == value)
            {
                return;
            }

            // SPLIT
            if (internal->child[i]->num_keys == Ord)
            {
                split(internal, i);
                if (internal->key[i] == value)
                {
                    return;
                }
                if (value > internal->key[i])
                {
                    i++;
                }
            }
            current = internal->child[i];
        }

        int i = 0;
        while (i < current->num_keys) // style fix: moved '&& value > current->key[i]' inside loop as break
        {
            if (value <= current->key[i]) break;
            i++;
        }

        if (i < current->num_keys && current->key[i] == value)
        {
            return;
        }

        // SHIFT elements one step
        for (int j = current->num_keys; j > i; --j)
        {
            current->key[j] = current->key[j - 1];
        }
        current->key[i] = value;
        current->num_keys += 1;
    }

    void Delete(Type value)
    {
        // case 1: the key is not in the tree (or the tree is empty), nothing to delete
        if (!Search(value))
        {
            return;
        }

        remove(root, value);

        // case 2: the root became empty
        if (root->num_keys == 0)
        {
            // case 2a: root is a leaf, the tree is now empty
            if (root->is_leaf)
            {
                delete static_cast<LeafNode<Type, Ord>*>(root);
                root = nullptr;
            }
            // case 2b: root is internal, its only child becomes the new root (height decreases)
            else
            {
                InternalNode<Type, Ord>* old_root = static_cast<InternalNode<Type, Ord>*>(root);
                root = old_root->child[0];
                delete old_root;
            }
        }
    }

    bool Search(Type key)
    {
        if (root == nullptr)
        {
            cout << "Not Found " << key << endl;
            return false;
        }

        Node<Type, Ord>* current = root;
        while (current != nullptr)
        {
            int i = 0;
            while (i < current->num_keys) // style fix: moved '&& key > current->key[i]' inside loop as break
            {
                if (key <= current->key[i]) break;
                i++;
            }

            if (i < current->num_keys && key == current->key[i])
            {
                cout << "Found " << key << endl;
                return true;
            }

            if (current->is_leaf)
            {
                cout << "Not Found " << key << endl;
                return false;
            }

            InternalNode<Type, Ord>* internal = static_cast<InternalNode<Type, Ord>*>(current);
            current = internal->child[i];
        }

        cout << "Not Found " << key << endl;
        return false;
    }

    void Display(Node<Type, Ord>* node, int depth)
    {
        if (!node)
        {
            return;
        }

        cout << string(depth * 4, ' ');

        for (int i = 0; i < node->num_keys; i++)
        {
            cout << node->key[i] << ' ';
        }

        cout << '\n';

        if (!node->is_leaf)
        {
            InternalNode<Type, Ord>* in = static_cast<InternalNode<Type, Ord>*>(node);
            for (int i = 0; i <= node->num_keys; i++)
            {
                Display(in->child[i], depth + 1);
            }
        }
    }

};

int main()
{
    BTree<int, 3> btree;
    btree.Insert(2);
    btree.Insert(1);
    btree.Insert(3);
    btree.Insert(7);
    btree.Insert(0);
    btree.Insert(10);
    btree.Insert(18);
    btree.Insert(20);
    btree.Insert(21);
    btree.Display(btree.root, 0);

    btree.Search(10);
    btree.Search(99);
    btree.Search(3);

    btree.Delete(10);
    btree.Display(btree.root, 0);
    btree.Delete(99);
    btree.Display(btree.root, 0);
    btree.Delete(3);
    btree.Display(btree.root, 0);
    btree.Delete(18);
    btree.Display(btree.root, 0);
}
