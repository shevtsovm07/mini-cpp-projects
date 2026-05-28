#include <iostream>
#include <filesystem>
#include "adapters/BaseAdapter.h"
#include "adapters/Dir.h"
#include "adapters/Filter.h"
#include "adapters/OpenFiles.h"
#include "adapters/Out.h"
#include "adapters/Split.h"
#include "adapters/Transform.h"
#include "adapters/AggregateByKey.h"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <utility>
#include <format>

/*
Программа считает частоту слов, встречающихся во всех файлах в директории 
и выводит итоговую статистику в консоль
*/

int main(int argc, char **argv) {
  if(argc != 2)
    return 1;
  

  bool recursive = false;
  Dir(argv[1], recursive) 
    | Filter([](const std::filesystem::path& p){ return p.extension() == ".txt"; })
    | OpenFiles()
    | Split("\n ,.;")
    | Transform(
        [](const std::string& token) { 
            std::string result = token;
            std::transform(result.begin(), result.end(), result.begin(), [](char c){return std::tolower(c);});
            return result;
        })
    | AggregateByKey(
        0uz, 
        [](const std::string&, size_t& count) { ++count;},
        [](const std::string& token) { return token;}
      )
    | Transform([](const std::pair<std::string, size_t>& stat) { return std::format("{} - {}", stat.first, stat.second);})
    | Out(std::cout);

  
  return 0;
}
