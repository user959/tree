#include <iostream>
#include <initializer_list>
#include <algorithm>

template<class ValueType>
struct node {
    const ValueType key;
    int height;
    size_t size;
    node* parent;
    node* left;
    node* right;

    node(ValueType key) :
       key(key),
       height(1),
       size(1),
       parent(nullptr),
       left(nullptr),
       right(nullptr) {}

    node(const node<ValueType>& other) :
       key(other.key),
       height(other.height),
       size(other.size),
       parent(other.parent),
       left(other.left),
       right(other.right) {}
};

template<typename ValueType>
class SetIterator {
public:
    typedef node<ValueType>* tree;
    typedef SetIterator<ValueType> iterator;

private:
    tree t_;
    size_t tree_size_;
    size_t cnt_;

    tree find_min(tree t) const {
        return t->left ? find_min(t->left) : t;
    }

    tree find_max(tree t) const {
        return t->right ? find_max(t->right) : t;
    }

public:
    explicit SetIterator(tree t = nullptr, size_t tree_size = 0, size_t cnt = 1)
        : t_(t),
          tree_size_(tree_size),
          cnt_(cnt)
          {}

    bool operator==(const SetIterator& other) {
        return cnt_ == other.cnt_;
    }

    bool operator!=(const SetIterator& other) {
        return !(*this == other);
    }

    iterator& operator++() {
        cnt_ = std::min(cnt_ + 1, tree_size_ + 1);
        if (cnt_ > tree_size_)
            return *this;

        if (t_->right) {
            t_ = find_min(t_->right);
        } else if (t_->parent->left == t_) {
            t_ = t_->parent;
        } else {
            tree prev = t_;
            t_ = t_->parent;
            while (!t_->right || t_->right == prev) {
                prev = t_;
                t_ = t_->parent;
            }
        }
        return *this;
    }

    iterator operator++(int) {
        iterator tmp(t_, cnt_);
        ++*this;
        return tmp;
    }

    iterator& operator--() {
        if (cnt_ == 1)
            return *this;
        --cnt_;
        if (cnt_ == tree_size_)
            return *this;

        if (t_->left) {
            t_ = find_max(t_->left);
        } else if (t_->parent->right == t_) {
            t_ = t_->parent;
        } else {
            tree prev = t_;
            t_ = t_->parent;
            while (!t_->left || t_->left == prev) {
                prev = t_;
                t_ = t_->parent;
            }
        }
        return *this;
    }

    iterator operator--(int) {
        iterator tmp(t_, cnt_);
        --*this;
        return tmp;
    }

    const ValueType operator*() const {
        return t_->key;
    }

    const ValueType* operator->() const {
        return &t_->key;
    }
};

template<class ValueType>
class Set {
public:
    typedef node<ValueType>* tree;
    typedef SetIterator<ValueType> iterator;

private:
    tree root_;

    int height(tree t) const {
        return t ? t->height : 0;
    }

    size_t size(tree t) const {
        return t ? t->size : 0;
    }

    int balance_factor(tree t) const {
        return height(t->right) - height(t->left);
    }

    void set_height(tree t) {
        t->height = std::max(height(t->left), height(t->right)) + 1;
    }

    void set_parent(tree t) {
        if (t->right)
            t->right->parent = t;
        if (t->left)
            t->left->parent = t;
    }

    void set_size(tree t) {
        t->size = size(t->left) + size(t->right) + 1;
    }

    void set_all(tree t) {
        if (!t)
            return;
        set_parent(t);
        set_height(t);
        set_size(t);
    }

    tree left_rotate(tree t) {
        tree p = t->right;
        t->right = p->left;
        p->left = t;
        set_all(t);
        set_all(p);
        return p;
    }

    tree right_rotate(tree t) {
        tree p = t->left;
        t->left = p->right;
        p->right = t;
        set_all(t);
        set_all(p);
        return p;
    }

    tree find_min(tree t) const {
        return t->left ? find_min(t->left) : t;
    }

    tree find_max(tree t) const {
        return t->right ? find_max(t->right) : t;
    }

    tree balance(tree t) {
        if (balance_factor(t) == 2) {
            if (balance_factor(t->right) < 0)
                t->right = right_rotate(t->right);
            set_parent(t);
            return left_rotate(t);
        }
        if (balance_factor(t) == -2) {
            if (balance_factor(t->left) > 0)
                t->left = left_rotate(t->left);
            set_parent(t);
            return right_rotate(t);
        }
        return t;
    }

    tree insert_rec(tree t, ValueType key) {
        if (!t)
            return new node<ValueType>(key);

        if (t->key < key)
            t->right = insert_rec(t->right, key);
        else if (key < t->key)
            t->left = insert_rec(t->left, key);
        set_all(t);
        return balance(t);
    }

    tree erase_min(tree t) {
        if (!t->left) {
            tree tmp = t->right;
            t->right = nullptr;
            set_all(t);
            return tmp;
        }
        t->left = erase_min(t->left);
        set_all(t);
        return balance(t);
    }

    tree erase_rec(tree t, ValueType key) {
        if (!t)
            return nullptr;

        if (t->key < key) {
            t->right = erase_rec(t->right, key);
        } else if (key < t->key) {
            t->left = erase_rec(t->left, key);
        } else {
            tree l = t->left;
            tree r = t->right;
            delete t;
            if (!r)
                return l;
            tree mn = find_min(r);
            mn->right = erase_min(r);
            mn->left = l;
            set_all(mn);
            return balance(mn);
        }
        set_all(t);
        return balance(t);
    }

    void clear(tree t) {
        if (!t)
            return;
        clear(t->left);
        clear(t->right);
        delete t;
    }

    tree copy(tree t) {
        if (!t)
            return nullptr;
        tree new_t = new node<ValueType>(*t);
        new_t->left = copy(t->left);
        new_t->right = copy(t->right);
        set_all(new_t);
        return new_t;
    }

public:
    Set(const Set<ValueType>& other) : Set() {
        if (this == &other)
            return;
        clear(root_);
        root_ = nullptr;
        root_ = copy(other.root_);
        set_all(root_);
    }

    Set<ValueType>& operator=(const Set<ValueType>& other) {
        if (this != &other) {
            clear(root_);
            root_ = nullptr;
            root_ = copy(other.root_);
            set_all(root_);
        }
        return *this;
    }

    Set() : root_(nullptr) {
    }


    template<typename Iter>
    Set(Iter first, Iter last) : Set() {
        for (auto it = first; it != last; ++it)
            insert(*it);
    }

    Set(std::initializer_list<ValueType> init_list) : Set() {
        for (const auto& el : init_list)
            insert(el);
    }

    void insert(ValueType key) {
        root_ = insert_rec(root_, key);
        set_all(root_);
    }

    void erase(ValueType key) {
        root_ = erase_rec(root_, key);
        set_all(root_);
    }

    size_t size() const {
        return size(root_);
    }

    bool empty() const {
        return size(root_) == 0;
    }

    iterator begin() const {
        if (empty())
            return iterator();
        tree m = find_min(root_);
        return iterator(m, size(root_), 1);
    }

    iterator end() const {
        if (empty())
            return iterator();
        tree m = find_max(root_);
        return iterator(m, size(root_), size(root_) + 1);
    }

    iterator find(ValueType key) const {
        auto it = lower_bound(key);
        if (it == end())
            return it;
        if (*it < key || key < *it) {
            return end();
        }
        return it;
    }

    iterator lower_bound(ValueType key) const {
        tree t = root_, best = nullptr;
        size_t cnt = 0, cnt_best = 0;
        while (t) {
            if (t->key < key) {
                cnt += size(t->left) + 1;
                t = t->right;
            } else {
                best = t;
                cnt_best = cnt;
                if (key < t->key)
                    t = t->left;
                else
                    break;
            }
        }
        if (!best)
            return end();
        return iterator(best, size(root_), cnt_best + 1);
    }

    ~Set() {
        clear(root_);
    }
};
