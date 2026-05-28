#include <gtest/gtest.h>
#include <circular_buffer.h>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <type_traits>
#include <initializer_list>

// Проверка корректности последовательного обхода буфера с использованием итераторов
TEST(MyBufferTests, IteratorTraversal) {
  circular_buffer<int> cb(5);
  for (int i = 0; i < 5; i++) {
    cb.push_back(i * 10);
  }
  
  int count = 0;
  for(auto it = cb.begin(); it != cb.end(); it++) {
    EXPECT_EQ(*it, count * 10);
    count++;
  }
}

// Проверка выброса исключения в случае инсерта, превышающего вместимость
TEST(MyBufferTests, InsertMoreThanCapacity) {
  circular_buffer<int> cb(5);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);
  EXPECT_THROW(cb.insert(cb.cbegin() + 1, 3, 4), std::length_error);
}

// Проверка вставки диапазона элементов из другого контейнера в середину буфера
TEST(MyBufferTests, RangeInsert) {
  circular_buffer<int> cb(10);
  cb.push_back(1);
  cb.push_back(5);
  std::vector<int> v = {2, 3, 4};
  cb.insert(cb.cbegin() + 1, v.begin(), v.end());
  
  EXPECT_EQ(cb.size(), 5);
  EXPECT_EQ(cb[2], 3);
}

// Проверка удаления диапазона элементов и полной очистки контейнера
TEST(MyBufferTests, EraseAndClear) {
  circular_buffer<int> cb(5);
  cb.assign({1, 2, 3, 4, 5});
  cb.erase(cb.cbegin() + 1, cb.cbegin() + 3); 
  EXPECT_EQ(cb.size(), 3);
  cb.clear();
  EXPECT_TRUE(cb.empty());
}

// Проверка работы контейнера с типами данных, не имеющими конструктора по умолчанию
struct special_obj {
  int val;
  special_obj(int v) : val(v) {}
};

TEST(MyBufferTests, NoDefaultCtor) {
  circular_buffer<special_obj> cb(2);
  cb.push_back(special_obj(42));
  EXPECT_EQ(cb.front().val, 42);
}

// Проверка логики кольцевого переноса при операциях добавления и удаления
TEST(CircularBufferTest, WrapAroundPushPop) {
  circular_buffer<int> cb(3);
  cb.push_back(1);
  cb.push_back(2);
  cb.push_back(3);
  
  cb.pop_front(); 
  cb.push_back(4); 
  
  EXPECT_EQ(cb.front(), 2);
  EXPECT_EQ(cb.back(), 4);
  
  cb.push_front(10); 
  EXPECT_EQ(cb.front(), 10);
  EXPECT_EQ(cb.size(), 3);
}

// Проверка методов массового заполнения и изменения размера буфера
TEST(CircularBufferTest, AssignAndResize) {
  circular_buffer<int> cb(10);
  
  cb.assign(5, 42);
  ASSERT_EQ(cb.size(), 5);
  EXPECT_EQ(cb[0], 42);
  EXPECT_EQ(cb[4], 42);
  
  cb.resize(2);
  EXPECT_EQ(cb.size(), 2);
  
  cb.resize(4, 7);
  EXPECT_EQ(cb.size(), 4);
  EXPECT_EQ(cb[3], 7);
  
  EXPECT_THROW(cb.resize(11), std::length_error);
}

// Проверка арифметики итераторов произвольного доступа 
TEST(CircularBufferTest, IteratorArithmetic) {
  circular_buffer<int> cb(5);
  cb.assign({10, 20, 30, 40, 50});
  
  auto it = cb.begin();
  EXPECT_EQ(*(it + 3), 40);
  EXPECT_EQ(*(cb.end() - 1), 50);
  
  it += 2;
  EXPECT_EQ(*it, 30);
  EXPECT_EQ(it[1], 40); 
  
  auto it2 = cb.begin() + 4;
  EXPECT_EQ(it2 - it, 2); 
  EXPECT_TRUE(it < it2);
}

// Проверка генерации исключений при некорректном доступе к данным или операциях с пустым буфером
TEST(CircularBufferTest, ExceptionHandling) {
  circular_buffer<int> cb(2);
  EXPECT_THROW(cb.at(0), std::out_of_range);
  EXPECT_THROW(cb.pop_back(), std::out_of_range);
  
  cb.push_back(1);
  EXPECT_NO_THROW(cb.at(0));
  EXPECT_THROW(cb.at(1), std::out_of_range);
}

// Проверка корректности конструктора копирования
TEST(CircularBufferTest, CopyAndMove) {
  circular_buffer<int> cb1(3);
  cb1.assign({1, 2, 3});
  
  circular_buffer<int> cb2(cb1); 
  EXPECT_EQ(cb1, cb2);
};

// Проверка реверсивных итераторов
TEST(MyBufferTests, ReverseIterators) {
  circular_buffer<int> cb(3);
  cb.assign({1, 2, 3});
  auto reverse_iterator = cb.rbegin();
  EXPECT_EQ(*reverse_iterator, 3);
  EXPECT_EQ(*(++reverse_iterator), 2);
  EXPECT_EQ(*(++reverse_iterator), 1);
  EXPECT_EQ(++reverse_iterator, cb.rend());
}

// Проверка константности доступа (const объекты)
TEST(MyBufferTests, ConstAccess) {
  const circular_buffer<int> cb = {10, 20};
  EXPECT_EQ(cb.front(), 10);
  EXPECT_EQ(cb.back(), 20);
  EXPECT_EQ(cb[1], 20);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(*cb.begin())>>);
}

// Проверка самоприсваивания
TEST(MyBufferTests, SelfAssignment) {
  circular_buffer<int> cb(5);
  cb.assign({1, 2, 3});
  cb = cb; 
  EXPECT_EQ(cb.size(), 3);
  EXPECT_EQ(cb[0], 1);
  EXPECT_EQ(cb[2], 3);
}

// Проверка работы при нулевой емкости
TEST(MyBufferTests, ZeroCapacity) {
  circular_buffer<int> cb(0);
  EXPECT_TRUE(cb.empty());
  EXPECT_EQ(cb.size(), 0);
  EXPECT_THROW(cb.push_back(1), std::length_error);
  EXPECT_EQ(cb.begin(), cb.end());
}

// Проверка swap 
TEST(MyBufferTests, SwapBuffers) {
  circular_buffer<int> cb1(3), cb2(3);
  cb1.assign({1, 1, 1});
  cb2.assign({2, 2, 2});
  cb1.swap(cb2);
  EXPECT_EQ(cb1[0], 2);
  EXPECT_EQ(cb2[0], 1);
}

// Проверка insert в начало, середину и конец
TEST(MyBufferTests, InsertPositions) {
  circular_buffer<int> cb(10);
  cb.push_back(1);
  cb.insert(cb.cbegin(), 0);
  cb.insert(cb.cend(), 2);
  cb.insert(cb.cbegin() + 1, 11);
  ASSERT_EQ(cb.size(), 4);
  EXPECT_EQ(cb[0], 0);
  EXPECT_EQ(cb[1], 11);
  EXPECT_EQ(cb[2], 1);
  EXPECT_EQ(cb[3], 2);
}

// Проверка erase 
TEST(MyBufferTests, Erase) {
  circular_buffer<int> cb(5);
  cb.assign({1, 2, 3});
  cb.erase(cb.cbegin() + 1);
  EXPECT_EQ(cb.size(), 2);
  EXPECT_EQ(cb[1], 3);
}

// Проверка операторов сравнения
TEST(MyBufferTests, Comparisons) {
  circular_buffer<int> cb1 = {1, 2, 3};
  circular_buffer<int> cb2 = {1, 2, 3};
  circular_buffer<int> cb3 = {1, 2, 4};
  EXPECT_TRUE(cb1 == cb2);
  EXPECT_TRUE(cb1 != cb3);
  EXPECT_TRUE(cb1 < cb3);
}


// Проверка метода at() на границы
TEST(MyBufferTests, AtMethodBounds) {
  circular_buffer<int> cb(3);
  cb.push_back(100);
  EXPECT_EQ(cb.at(0), 100);
  EXPECT_THROW(cb.at(1), std::out_of_range);
  EXPECT_THROW(cb.at(5), std::out_of_range);
}

// Проверка работы с алгоритмом сортировки
TEST(STLCompatibility, SortAlgorithm) {
  circular_buffer<int> cb(5);
  cb.assign({50, 10, 40, 20, 30});
  
  std::sort(cb.begin(), cb.end());
  
  EXPECT_EQ(cb[0], 10);
  EXPECT_EQ(cb[4], 50);
  EXPECT_TRUE(std::is_sorted(cb.begin(), cb.end()));
}


// Проверка копирования данных (std::copy) в другой контейнер
TEST(STLCompatibility, CopyToVector) {
  circular_buffer<int> cb(3);
  cb.assign({7, 8, 9});
  
  std::vector<int> target(3);
  std::copy(cb.begin(), cb.end(), target.begin());
  
  EXPECT_EQ(target[0], 7);
  EXPECT_EQ(target[2], 9);
}

// Проверка алгоритмов reverse и fill
TEST(STLCompatibility, ReverseAndFill) {
  circular_buffer<int> cb(4);
  cb.assign({1, 2, 3, 4});
  

  std::reverse(cb.begin(), cb.end());
  EXPECT_EQ(cb[0], 4);
  EXPECT_EQ(cb[3], 1);
  

  std::fill(cb.begin(), cb.end(), 8);
  for(int val : cb) {
    EXPECT_EQ(val, 8);
  }
}

