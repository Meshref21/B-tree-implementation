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
    Node<Type, Ord> *child[Ord + 1];
};

template <typename Type, int Ord>
struct BTree
{
private:

    void split(InternalNode<Type, Ord> *parent, int child_index)
    {
        Node<Type, Ord> *node = parent->child[child_index];
        int median = Ord / 2;

        Node<Type, Ord> *right;
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
            InternalNode<Type, Ord> *left_internal_node = static_cast<InternalNode<Type, Ord> *>(node);
            InternalNode<Type, Ord> *right_internal_node = static_cast<InternalNode<Type, Ord> *>(right);
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

public:

    Node<Type, Ord> *root;

    BTree()
    {
        root = nullptr;
    }

    void Insert(Type value)
    {

        // tree is empty
        if (root == nullptr)
        {
            LeafNode<Type, Ord> *New_Leaf_Node = new LeafNode<Type, Ord>;
            New_Leaf_Node->is_leaf = true;
            New_Leaf_Node->key[0] = value;
            New_Leaf_Node->num_keys = 1;

            root = New_Leaf_Node;
            return;
        }

        // case2 root is full
        if (root->num_keys == Ord)
        {
            InternalNode<Type, Ord> *new_root = new InternalNode<Type, Ord>;
            new_root->is_leaf = false;
            new_root->num_keys = 0;
            new_root->child[0] = root;
            root = new_root;
            split(new_root, 0);
        }

        Node<Type, Ord> *current = root;
        while (!current->is_leaf)
        {
            // cast to access internal nodes childs
            InternalNode<Type, Ord> *internal = static_cast<InternalNode<Type, Ord> *>(current);

            int i = 0;
            while (i < current->num_keys && value > current->key[i])
            {
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
        while (i < current->num_keys && value > current->key[i])
        {
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

    void Delete()
    {
        return;
    }

    bool Search(Type key)
    {
        if (root == nullptr)
        {
            cout << "Not Found " << key << endl;
            return false;
        }

        Node<Type, Ord> *current = root;
        while (current != nullptr)
        {
            int i = 0;
            while (i < current->num_keys && key > current->key[i])
            {
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

            InternalNode<Type, Ord> *internal = static_cast<InternalNode<Type, Ord> *>(current);
            current = internal->child[i];
        }

        cout << "Not Found " << key << endl;
        return false;
    }

    void Display(Node<Type, Ord> *node, int depth)
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
            InternalNode<Type, Ord> *in = static_cast<InternalNode<Type, Ord> *>(node);
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
}
