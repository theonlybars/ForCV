#include <iostream>
#include <string>
#include <utility>
#include <memory>
#include <functional>
#include <climits>
#include <vector>

using std::vector; 
using std::shared_ptr; 
using std::make_shared; 
using std::unique_ptr; 
using std::make_unique; 

template<class charT> class ForwardAddressTable;
template<class charT> class HashTable; 
template<class charT, class AddressTable, charT sentinel> class Builder;
template<class charT, class AddressTable, charT sentinel> class TreeVisitor; 
template<class charT, class AddressTable, charT sentinel> class TreeIterator; 

template<class charT, class AddressTable, charT sentinel>
class SuffixTree {
    public:
        
        class Node; 
        class Location;
        
        SuffixTree() { root_ = make_shared<Node>(0, Node::kLastSymbolIndex); }
        SuffixTree(const SuffixTree&) = delete;
        SuffixTree& operator= (const SuffixTree&) = delete;
        ~SuffixTree() = default; // It would be generated without this line but that way we know for sure the destructor is default

        bool Substring(const std::basic_string<charT>& pattern) const; 
        void Traverse(TreeVisitor<charT, AddressTable, sentinel>& visitor, 
                      TreeIterator<charT, AddressTable, sentinel>& iterator) const;
        
        typedef TreeIterator<charT, AddressTable, sentinel> Iterator; 
        Location GiveSuffixLink(const Iterator* iterator); 
        Location GiveDownLocation(const Iterator* iterator, const charT edge_symbol); 
        Location GetUpLocation(const Iterator* iterator); 
        Location GetDownNode(const Iterator* iterator, const charT edge_symbol); 
        Location GetUpNode(const Iterator* iterator); 

        charT operator[](const int position) const { return string_[position]; }
        int Size() const { return string_.size(); }
        bool Empty() const { return string_.empty(); }

        Location Root() { return Location(root_); } 


        class Node { 

                typedef SuffixTree<charT, AddressTable, sentinel>::Location Location;
                Node() = delete; // Nodes created with use std::make_shared<Node> (...) only within Builder::Build method
                explicit Node(const int start, const int end) : start_(start), end_(end) {}
                explicit Node(const int start, const int end, shared_ptr<Node> parent,
                     Location suffix_transition) : start_(start), end_(end),
                     parent_(parent), suffix_transition_(suffix_transition) {}

                Node(const Node&) = delete;
                Node& operator= (const Node&) = delete;

                shared_ptr<Node> DownNode(const charT edge_symbol) const; 
                Location DownLocation(const charT edge_symbol) const; 
                Location SuffixTransition() const{ return suffix_transition_; }
                shared_ptr<Node> Parent() const { return parent_; }

                bool Leaf(); 

                static constexpr int kLastSymbolIndex = INT_MAX;
                int start_, end_;
                shared_ptr<Node> parent_;
                Location suffix_transition_;
                AddressTable descendants_;

                friend class SuffixTree<charT, AddressTable, sentinel>; 
                friend class Builder<charT, AddressTable, sentinel>; 

            public:

                int Start() { return start_; }
                int End() { return end_; }

        }; // Node

        class Location {

            typedef SuffixTree<charT, AddressTable, sentinel>::Node Node;

            shared_ptr<Node> node_;
            int margin_;
            charT start_symbol_;
            
            Location(shared_ptr<Node> node, int margin, charT start_symbol) :
            node_(node), margin_(margin), start_symbol_(start_symbol) {}

            Location(shared_ptr<Node> node) : node_(node), margin_(0) {}
            Location() = default; 
            
            bool IsNode() const; 

            friend class SuffixTree<charT, AddressTable, sentinel>; 
            friend class Builder<charT, AddressTable, sentinel>; 

            public: 

                void Accept(TreeVisitor<charT, AddressTable, sentinel>& visitor) { visitor.VisitLocation(this); }
                int Start();
                int End();
        }; //Location

    private:
        
        Location DownLocation(Location location, const charT edge_symbol) const; 
        Location SuffixTransition(Location location) const; 
        shared_ptr<Node> UpNode(Location location) const; 

        std::basic_string<charT> string_;
        shared_ptr<Node> root_;
        
        friend class Builder<charT, AddressTable, sentinel>;

}; //SuffixTree

template<class charT, class AddressTable, charT sentinel> class Builder {
    public:

        typedef SuffixTree<charT, AddressTable, sentinel> Tree;
        typedef typename Tree::Node Node; // maybe replace it before public if allowed
        typedef typename Tree::Location Location; 

        Builder() = default;
        void Fit(std::basic_string<charT>& string);
        shared_ptr<Tree> Build();

    private:
        
        void MoveString(shared_ptr<Tree> suffix_tree);
        void AssignNewEdge(shared_ptr<Node> first_node, const charT edge_symbol, shared_ptr<Node> second_node) const; 
        shared_ptr<Node> MakeNewEdge(const int start, const int end, shared_ptr<Node> parent, Location suffix_transition) const; 
        void MakeSuffixLink(shared_ptr<Node> node, Location suffix_transition) const; 

        std::basic_string<charT> string_;

}; //Builder

template<class charT, class AddressTable, charT sentinel> class TreeVisitor { 
    public: 
        typedef SuffixTree<charT, AddressTable, sentinel> Tree; 
        typedef typename Tree::Location Location; 

        virtual void VisitLocation(Location* location) = 0; 
        
        explicit TreeVisitor(shared_ptr<Tree> suffix_tree) : suffix_tree_(suffix_tree) {}
        TreeVisitor() = default; 
        TreeVisitor& operator= (const TreeVisitor&) = delete; 
        TreeVisitor (const TreeVisitor&) = delete; 

    protected: 

        shared_ptr<Tree> suffix_tree_; 
}; // TreeVisitor

template<class charT, class AddressTable, charT sentinel> class TreeIterator { 
    public:

        typedef SuffixTree<charT, AddressTable, sentinel> Tree; 
        typedef typename Tree::Location Location; 

        TreeIterator(shared_ptr<Tree> suffix_tree, Location location) : suffix_tree_(suffix_tree), current_location_(location) {}
        TreeIterator(shared_ptr<Tree> suffix_tree) : TreeIterator(suffix_tree, suffix_tree->Root()) {}
        
        virtual void Next() = 0; 
        virtual void Previous() = 0; 

        bool IsValid() const{ return current_location_.empty(); }
        
        void ExamineNode(TreeVisitor<charT, AddressTable, sentinel>& visitor) const{ current_location_->Accept(visitor); }
        Location CurrentLocation() const{ return current_location_; }
        
    protected: 

        Location GetSuffixLink(); 
        Location GetDownLocation(charT edge_symbol); 
        Location GetUpLocation(); 
        Location GetDownNode(charT edge_symbol); 
        Location GetUpNode(); 

        shared_ptr<Tree> suffix_tree_; 
        Location current_location_; 

}; // Iterator

/*
int main() {
    std::string string;
    std::cin >> string;

    Builder<char, ForwardAddressTable<char>, '$'> builder_first;
    builder.Fit(string);
    auto suffix_tree_forward_addressing = builder_first.Build();
    Builder<char, HashTable<char>, '$'> builder_second;
    auto suffix_tree_hash_tables = builder_second.Build();
    return 0;
}

template<class charT, class AddressTable, charT sentinel>
void Builder<charT, AddressTable, sentinel>::Fit(std::basic_string<charT>& string) {

    string_ = (string.find(sentinel) == string.npos) ? std::move(string.append(1, sentinel)) : std::basic_string<charT>();
    // maybe not the best decision to leave empty string if sentinel encountered in input string, but it is very easy to implement.
    // good way to handle this problem is to create class-shell for symbols where would be bool member labeling if the symbol is
    // sentinel, but this would lead to many additional code, so here we left it as it is.

}

template<class charT, class AddressTable, charT sentinel> //something should be done with those vast signatures
void Builder<charT, AddressTable, sentinel>::MoveString(shared_ptr<SuffixTree<charT, AddressTable, sentinel>> suffix_tree) {  
    suffix_tree.string_ = std::move(string_);             
}

template<class charT, class AddressTable, charT sentinel>
shared_ptr<SuffixTree<charT, AddressTable, sentinel>> Builder<charT, AddressTable, sentinel>::Build() {

    auto suffix_tree = make_shared<Tree>();
    auto root = suffix_tree->root_;

    Builder.MoveString(suffix_tree); 
    auto& string = suffix_tree->string_;
    //moving string forward to its destination
    
    std::function<shared_ptr<Node>(charT)> function = root->descendants_; 
    // Proving if the AddressTable is valid

    charT default_char;
    Location current_location(root, 0, default_char)
    // Initial location is always a Node, but in process of lifting up the tree
    // it can turn into an edge location.

    for (int index = 0; index < string.size(); ++index) {
        charT last_symbol = string[index];
        if (current_location.node_ == root) {
            if(root->DownNode(last_symbol).node_ != AddressTable::kEmptyPointer) {
                //kEmptyPointer must be public
                current_location = root->DownLocation(last_symbol)
            } else {
                AssignNewEdge(root, last_symbol, MakeNewNode(index, Node::kLastSymbolIndex, root, Location(root, 0, default_char))); 
            }
            continue;
        }

        shared_ptr<Node> previous_node;
        while(current_location.node_->Leaf()) {
            previous_node = current_location.node_;
            current_location = previous_node->suffix_transition_;
        }
          
        while (Tree::DownLocation(current_location, last_symbol) == Location::kInvalidTransition) { 
            shared_ptr<Node> current_node; 
            if (!current_location.IsNode()) { 
                auto son = Tree::DownNode(current_location, last_symbol); // If Location is on the edge last_symbol ignored
                int new_start = son->start_;
                int new_end = new_start + current_location.margin_ - 1;
                son->start_ = new_end + 1;
                shared_ptr<Node> parent = current_location.node_;

                current_node = MakeNewNode(new_start, new_end, parent, Location()); 
            } else { 
                current_node = current_location.node_; 
            }
            
            current_location = Tree::SuffixTransition(current_location);  

            AssignNewEdge(current_node, last_symbol, MakeNewNode(index, Node::kLastSymbolIndex, current_node, Location())); 

            MakeSuffixLink(previous_node, current_node);   // Using conversion constructor Location(shared_ptr<Node>)

            if (previous_node()) {     // the same as previous_node == AddressTable::kEmptyPointer but brief 
                MakeSuffixLink(previous_node->DownNode(last_symbol), current_node->DownNode(last_symbol)); 
            }

            previous_node = current_node;
        }

        MakeSuffixLink(previous_node, current_location); 
        current_location = Tree::DownLocation(current_location, last_symbol); 
        if (previous_node()) {     
            MakeSuffixLink(previous_node->DownNode(last_symbol), current_location); 
        }
    }
}

*/
