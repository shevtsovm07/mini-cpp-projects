#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring> 
#include "../lib/archive.h"
#include <fstream>

const int kFileArgLength = 7;

const char* FindName(const char* argument) {
  const char* equal_sign = strchr(argument, '=');
  if (!equal_sign) return nullptr;
  return equal_sign + 1;
}


bool Parser(int argc, char* argv[], Archive& archive) {
for (int argument = 1; argument < argc; argument++) {
  if (strcmp(argv[argument], "-c") == 0 || strcmp(argv[argument], "--create") == 0) {
    archive.ChangeAction(Archive::Actions::kCreate);
  }
  else if (strcmp(argv[argument], "-f") == 0) {
    if (argument+1 >= argc) return false;
    if (argv[argument+1][0] == '\0') return false;
    archive.ChangeName(argv[argument+1]);
    argument++;
  }
  else if (strncmp(argv[argument], "--file=", kFileArgLength) == 0) {
    const char* name;
    name = FindName(argv[argument]);
    if (name != nullptr){
      archive.ChangeName(name);
    }
  }
  else if (strcmp(argv[argument], "-l") == 0 || strcmp(argv[argument], "--list") == 0) {
    archive.ChangeAction(Archive::Actions::kFilesList);
  }
  else if (strcmp(argv[argument], "-x") == 0 || strcmp(argv[argument], "--extract") == 0) {
    archive.ChangeAction(Archive::Actions::kExtract);
  }
  else if (strcmp(argv[argument], "-a") == 0 || strcmp(argv[argument], "--append") == 0) {
    archive.ChangeAction(Archive::Actions::kAppend);
  }
  else if (strcmp(argv[argument], "-d") == 0 || strcmp(argv[argument], "--delete") == 0) {
    archive.ChangeAction(Archive::Actions::kDelete);
  }
  else if (strcmp(argv[argument], "-A") == 0 || strcmp(argv[argument], "--concatenate") == 0) {
    archive.ChangeAction(Archive::Actions::kMerge);
  }
  else {
    archive.AddInputFile(argv[argument]);
  }
}
return true;
}




int main(int argc, char* argv[]) {
    Archive archive;

    if (!Parser(argc, argv, archive)) {
        std::cout << "Error parsing arguments" << std::endl;
        return 1;
    }

    switch (archive.GetAction()) {
        case Archive::Actions::kCreate:
            archive.CreateArchive(archive.GetInputFiles());
            break;
        case Archive::Actions::kFilesList:
            archive.FilesList();
            break;
      case Archive::Actions::kExtract:
            archive.ExtractFile();
            break;
        case Archive::Actions::kAppend:
            archive.AppendFile();
            break;
        case Archive::Actions::kDelete:
            for (size_t i = 0; i < archive.GetInputFiles().size(); i++) {
            archive.DeleteFile(archive.GetInputFiles()[i]);
            }
            break;
        case Archive::Actions::kMerge:
            archive.Merge();
            break;
        default:
            std::cout << "No valid action provided" << std::endl;
            return 1;
    }

    return 0;
}
