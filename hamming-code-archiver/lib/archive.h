#pragma once
#include <vector>
#include <string>

class Archive {
public:
    enum class Actions {
      kNoAction,
      kCreate,
      kFilesList,
      kExtract,
      kAppend,
      kDelete,
      kMerge
    };

  Archive();
  Archive(const Archive& archive);
  ~Archive();

  void CreateArchive(const std::vector<std::string>& input_files);
  void FilesList();
  void ExtractFile();
  void AppendFile();
  void DeleteFile(const std::string& filename_to_delete);
  void Merge();

  void ChangeAction(Actions action);
  Actions GetAction() const;

  void ChangeName(const char* name);
  const char* GetName() const;

  void AddInputFile(const std::string& filename);
  const std::vector<std::string>& GetInputFiles() const;

private:
  char* arch_name = nullptr;
  Actions action = Actions::kNoAction;
  std::vector<std::string> input_files;
};
