#include <cassert>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include "doublelist.hpp"

namespace {

struct Tracked {
    static int alive;
    int id;

    Tracked() : id(0) { ++alive; }
    explicit Tracked(int v) : id(v) { ++alive; }
    Tracked(const Tracked& other) : id(other.id) { ++alive; }
    Tracked(Tracked&& other) noexcept : id(other.id) {
        other.id = -1;
        ++alive;
    }
    Tracked& operator=(const Tracked& other) {
        id = other.id;
        return *this;
    }
    Tracked& operator=(Tracked&& other) noexcept {
        id = other.id;
        other.id = -1;
        return *this;
    }
    ~Tracked() { --alive; }

    bool operator==(const Tracked& other) const { return id == other.id; }
};

int Tracked::alive = 0;

std::string capture_display(const DoubleList<int>& list, bool reversed) {
    std::ostringstream oss;
    std::streambuf *old_buf = std::cout.rdbuf(oss.rdbuf());
    if (reversed) {
        list.display_reversed();
    } else {
        list.display();
    }
    std::cout.rdbuf(old_buf);
    return oss.str();
}

void assert_list_matches(const DoubleList<int>& list, const std::string& forward_expected,
                         const std::string& reversed_expected) {
    assert(capture_display(list, false) == forward_expected);
    assert(capture_display(list, true) == reversed_expected);
}

void test_empty_and_clear() {
    DoubleList<int> list;
    assert(list.empty());
    assert(!list.full());
    assert(list.size() == 0);
    assert(list.index(10) == -1);
    assert_list_matches(list, "\n", "\n");

    list.clear();
    assert(list.empty());
    assert(list.size() == 0);
}

void test_push_pop_front_back() {
    DoubleList<int> list;

    list.push_front(20);
    assert(!list.empty());
    assert(list.size() == 1);
    assert(list.front() == 20);
    assert(list.back() == 20);
    assert_list_matches(list, "20 \n", "20 \n");

    list.push_front(10);
    list.push_back(30);
    list.push_back(40);
    assert(list.size() == 4);
    assert(list.front() == 10);
    assert(list.back() == 40);
    assert_list_matches(list, "10 20 30 40 \n", "40 30 20 10 \n");

    list.pop_front();
    assert(list.size() == 3);
    assert(list.front() == 20);
    assert(list.back() == 40);
    assert_list_matches(list, "20 30 40 \n", "40 30 20 \n");

    list.pop_back();
    assert(list.size() == 2);
    assert(list.front() == 20);
    assert(list.back() == 30);
    assert_list_matches(list, "20 30 \n", "30 20 \n");

    list.pop_back();
    assert(list.size() == 1);
    assert(list.front() == 20);
    assert(list.back() == 20);
    assert_list_matches(list, "20 \n", "20 \n");

    list.pop_front();
    assert(list.empty());
    assert(list.size() == 0);
    assert_list_matches(list, "\n", "\n");

    list.push_back(99);
    assert(list.size() == 1);
    assert(list.front() == 99);
    assert(list.back() == 99);
    list.pop_back();
    assert(list.empty());

    list.push_front(1);
    list.push_back(2);
    list.clear();
    assert(list.empty());
    assert(list.size() == 0);
    list.push_back(7);
    assert(list.size() == 1);
    assert(list.front() == 7);
    assert(list.back() == 7);
}

void test_copy_and_move() {
    DoubleList<int> empty_list;
    DoubleList<int> copied_empty(empty_list);
    assert(copied_empty.empty());

    DoubleList<int> orig;
    orig.push_back(1);
    orig.push_back(2);
    orig.push_back(3);

    DoubleList<int> copy(orig);
    assert(copy.size() == 3);
    assert_list_matches(copy, "1 2 3 \n", "3 2 1 \n");

    copy[1] = 200;
    copy.push_back(4);
    orig.pop_front();
    assert_list_matches(orig, "2 3 \n", "3 2 \n");
    assert_list_matches(copy, "1 200 3 4 \n", "4 3 200 1 \n");

    DoubleList<int> assigned;
    assigned.push_back(999);
    assigned = orig;
    assert_list_matches(assigned, "2 3 \n", "3 2 \n");

    DoubleList<int>& assigned_ref = assigned;
    assigned = assigned_ref;
    assert_list_matches(assigned, "2 3 \n", "3 2 \n");

    assigned = empty_list;
    assert(assigned.empty());

    DoubleList<int> moved(std::move(copy));
    assert(moved.size() == 4);
    assert_list_matches(moved, "1 200 3 4 \n", "4 3 200 1 \n");
    assert(copy.empty());
    assert(copy.size() == 0);

    copy.push_front(55);
    assert(copy.size() == 1);
    assert(copy.front() == 55);

    DoubleList<int> move_target;
    move_target.push_back(88);
    move_target = std::move(moved);
    assert_list_matches(move_target, "1 200 3 4 \n", "4 3 200 1 \n");
    assert(moved.empty());

    DoubleList<int>& move_target_ref = move_target;
    move_target = std::move(move_target_ref);
    assert_list_matches(move_target, "1 200 3 4 \n", "4 3 200 1 \n");
}

void test_indexed_operations() {
    DoubleList<int> list;

    list.insert(0, 20);
    assert(list.size() == 1);
    assert(list[0] == 20);

    int v10 = 10;
    list.insert(0, v10);
    list.insert(list.size(), 40);
    int v30 = 30;
    list.insert(2, v30);
    assert_list_matches(list, "10 20 30 40 \n", "40 30 20 10 \n");

    const DoubleList<int>& cref = list;
    assert(cref.front() == 10);
    assert(cref.back() == 40);
    assert(cref[0] == 10);
    assert(cref[1] == 20);
    assert(cref[2] == 30);
    assert(cref[3] == 40);

    list[2] = 35;
    assert(list[2] == 35);
    assert_list_matches(list, "10 20 35 40 \n", "40 35 20 10 \n");

    list.push_back(20);
    assert(list.index(10) == 0);
    assert(list.index(20) == 1);
    assert(list.index(40) == 3);
    assert(list.index(999) == -1);

    list.erase(2);
    assert_list_matches(list, "10 20 40 20 \n", "20 40 20 10 \n");

    list.erase(0);
    assert_list_matches(list, "20 40 20 \n", "20 40 20 \n");

    list.erase(list.size() - 1);
    assert_list_matches(list, "20 40 \n", "40 20 \n");

    list.erase(1);
    assert(list.size() == 1);
    assert(list.front() == 20);
    assert(list.back() == 20);

    list.erase(0);
    assert(list.empty());
    assert(list.size() == 0);
}

void test_strings_and_move_only_like_behavior() {
    DoubleList<std::string> words;
    std::string a = "alfa";
    std::string b = "beta";
    std::string c = "gamma";

    words.push_front(a);
    words.push_back(std::move(b));
    words.insert(1, std::move(c));

    assert(words.size() == 3);
    assert(words[0] == "alfa");
    assert(words[1] == "gamma");
    assert(words[2] == "beta");
    assert(words.index("gamma") == 1);
    assert(words.index("delta") == -1);
}

void test_lifetime_no_leaks() {
    assert(Tracked::alive == 0);
    {
        DoubleList<Tracked> list;
        list.push_back(Tracked(1));
        list.push_front(Tracked(2));
        list.insert(1, Tracked(3));
        assert(Tracked::alive == 3);

        DoubleList<Tracked> copy(list);
        assert(Tracked::alive == 6);

        copy.erase(1);
        assert(Tracked::alive == 5);

        DoubleList<Tracked> moved(std::move(copy));
        assert(Tracked::alive == 5);
        assert(copy.empty());

        moved.pop_front();
        moved.pop_back();
        assert(Tracked::alive == 3);

        list.clear();
        assert(Tracked::alive == 0);
    }
    assert(Tracked::alive == 0);
}

}

int main() {
    test_empty_and_clear();
    test_push_pop_front_back();
    test_copy_and_move();
    test_indexed_operations();
    test_strings_and_move_only_like_behavior();
    test_lifetime_no_leaks();

    DoubleList<int> demo;
    demo.push_back(11);
    demo.push_back(22);
    demo.push_back(33);
    std::cout << "Wszystkie testy zakonczone sukcesem!\n";
    std::cout << "Przykladowa lista (przod): ";
    demo.display();
    std::cout << "Przykladowa lista (wstecz): ";
    demo.display_reversed();

    return 0;
}
