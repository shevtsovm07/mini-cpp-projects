#include "archive.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include <cstdio>
#include <filesystem>

const int kFullHammingBitsAmount = 21;
const int kTwoBytes = 16;
const int kFourBytes = 4;
const int kBitsInByte = 8;
const int kBitsInTwoBytes = 16;
const int kBitsInThreeBytes = 24;
const int kMaxBufferSize = 4096;
const int kBytesPerBlock = 2;
const uint8_t kByteMask = 0xFF;

bool WriteData(std::ofstream& out, const void* data, size_t size) {
  out.write(static_cast<const char*>(data), size);
  return static_cast<bool>(out);
}

bool ReadData(std::ifstream& in, void* data, size_t size) {
  in.read(static_cast<char*>(data), size);
  return static_cast<bool>(in);
}

void WriteUint32(std::ofstream& out, uint32_t value) {
  uint8_t bytes[kFourBytes];
  bytes[0] = (value >> kBitsInThreeBytes) & kByteMask;
  bytes[1] = (value >> kBitsInTwoBytes) & kByteMask;
  bytes[2] = (value >> kBitsInByte) & kByteMask;
  bytes[3] = value & kByteMask;
  WriteData(out, bytes, kFourBytes);
}

uint32_t ReadUint32(std::ifstream& in) {
  uint8_t bytes[kFourBytes];
  if (!ReadData(in, bytes, kFourBytes)) return 0;
  return (bytes[0] << kBitsInThreeBytes) | (bytes[1] << kBitsInTwoBytes) | (bytes[2] << kBitsInByte) | bytes[3];
}

void WriteUint16(std::ofstream& out, uint16_t value) {
  uint8_t bytes[kBytesPerBlock];
  bytes[0] = (value >> kBitsInByte) & kByteMask;
  bytes[1] = value & kByteMask;
  WriteData(out, bytes, kBytesPerBlock);
}

uint16_t ReadUint16(std::ifstream& in) {
  uint8_t bytes[kBytesPerBlock];
  if (!ReadData(in, bytes, kBytesPerBlock)) return 0;
  return (bytes[0] << kBitsInByte) | bytes[1];
}

Archive::Archive() {}

Archive::Archive(const Archive& archive) {
  if (archive.arch_name) {
    int len = std::strlen(archive.arch_name);
    arch_name = new char[len + 1];
    std::strcpy(arch_name, archive.arch_name);
  } else {
    arch_name = nullptr;
  }
  action = archive.action;
  input_files = archive.input_files;
}

Archive::~Archive() {
  delete[] arch_name;
}

bool IsPowerOfTwo(int n) {
  return ((n > 0) && ((n & (n - 1)) == 0));
}

uint32_t HammingCode(uint16_t message) {
  uint32_t encoded = 0;
  int data_bit_position = 0;

  for (int pos = 1; pos <= 21; pos++) {
    if (!IsPowerOfTwo(pos)) {
      int bit_value = (message >> data_bit_position) & 1;
      if (bit_value) {
        encoded |= (1 << (pos-1));
      }
      data_bit_position++;
    }
  }


  for (int control_bit_pos = 1; control_bit_pos <= 16; control_bit_pos*=2) {
    int parity = 0;
    for (int bit_pos = 1; bit_pos <= 21; bit_pos++) {
      if (control_bit_pos & bit_pos) {
        int value = (encoded >> (bit_pos-1)) & 1;
        parity ^= value;
      }
    }
    encoded |= (parity << (control_bit_pos-1));
  }
  return encoded;
}

uint16_t HammingDecode(uint32_t encoded) {
  int syndrome = 0;
  uint16_t decoded = 0;
  for (int control_bit_pos = 1; control_bit_pos <= 16; control_bit_pos*=2) {
    int parity = 0;
    for (int bit_pos = 1; bit_pos <= 21; bit_pos++) {
      if (control_bit_pos & bit_pos) {
        int value = (encoded >> (bit_pos-1)) & 1;
        parity ^= value;
      }
    }
    int actual_control_bit = (encoded >> (control_bit_pos - 1)) & 1;
    if (parity != 0) {
        syndrome |= control_bit_pos;
    }
}
  if (syndrome != 0) {
      encoded ^= (1 << (syndrome-1));
  }
  int data_bit_position = 0;
  for (int pos = 1; pos <= 21; pos++) {
    if (!IsPowerOfTwo(pos)) {
      int bit_value = (encoded >> (pos-1)) & 1;
      if (bit_value) {
        decoded |= (1 << data_bit_position);
      }
      data_bit_position++;
    }
  }
  return decoded;
}

void Archive::CreateArchive(const std::vector<std::string>& input_files) {
  if (!arch_name) {
    std::cout << "Archive name not set" << std::endl;
    return;
  }

  std::ofstream out(arch_name, std::ios::binary);
  if (!out) {
    std::cout << "Failed to open archive file" << std::endl;
    return;
  }

  int files_amount = input_files.size();
  for (int file = 0; file < files_amount; file++) {
    std::string filename = input_files[file];
    std::ifstream input(filename, std::ios::binary);
    if (!input) {
      std::cout << "File not opened: " << filename << std::endl;
      continue;
    }

    uint16_t name_length = filename.size();
    WriteUint16(out, name_length);
    

    for (size_t i = 0; i < name_length; i += kBytesPerBlock) {
      uint16_t block = static_cast<uint8_t>(filename[i]);
      if (i + 1 < name_length) {
        block |= (static_cast<uint8_t>(filename[i + 1]) << kBitsInByte);
      }
      uint32_t encoded = HammingCode(block);
      WriteUint32(out, encoded);
    }

    input.seekg(0, std::ios::end);
    uint32_t data_length = input.tellg();
    input.seekg(0, std::ios::beg);
    WriteUint32(out, data_length);


    char buffer[kBytesPerBlock];
    while (input.read(buffer, kBytesPerBlock) || input.gcount() > 0) {
      uint16_t block = static_cast<uint8_t>(buffer[0]);
      if (input.gcount() == kBytesPerBlock) {
        block |= (static_cast<uint8_t>(buffer[1]) << kBitsInByte);
      }
      uint32_t encoded = HammingCode(block);
      WriteUint32(out, encoded);
    }
    input.close();
  }
  out.close();
}

void Archive::FilesList() {
  std::ifstream in(arch_name, std::ios::binary);
  if (!in) {
    return;
  }

  while (true) {
    uint16_t name_length = ReadUint16(in);
    if (!in) break;


    std::vector<uint8_t> decoded_name;
    size_t name_blocks = (name_length + 1) / kBytesPerBlock;
    for (size_t i = 0; i < name_blocks; i++) {
      uint32_t encoded = ReadUint32(in);
      if (!in) break;
      uint16_t block = HammingDecode(encoded);
      decoded_name.push_back(block & kByteMask);
      if (decoded_name.size() < name_length) {
        decoded_name.push_back((block >> kBitsInByte) & kByteMask);
      }
    }
    std::string filename(decoded_name.begin(), decoded_name.end());

    uint32_t data_length = ReadUint32(in);
    if (!in) break;


    size_t blocks_count = (data_length + 1) / kBytesPerBlock;
    for (size_t i = 0; i < blocks_count; i++) {
      ReadUint32(in);
      if (!in) break;
    }
    
    std::cout << filename << std::endl;
  }
  in.close();
}

void Archive::ExtractFile() {
  std::ifstream in(arch_name, std::ios::binary);
  if (!in) return;

  while (true) {
    uint16_t name_length = ReadUint16(in);
    if (!in) break;
    

    std::vector<uint8_t> decoded_name;
    size_t name_blocks = (name_length + 1) / kBytesPerBlock;
    for (size_t name_block_idx = 0; name_block_idx < name_blocks; name_block_idx++) {
      uint32_t encoded = ReadUint32(in);
      if (!in) break;
      uint16_t block = HammingDecode(encoded);
      decoded_name.push_back(block & kByteMask);
      if (decoded_name.size() < name_length) {
        decoded_name.push_back((block >> kBitsInByte) & kByteMask);
      }
    }
    std::string filename(decoded_name.begin(), decoded_name.end());

    uint32_t data_length = ReadUint32(in);
    if (!in) break;

    size_t blocks_count = (data_length + 1) / kBytesPerBlock;
    std::vector<uint32_t> encoded_blocks(blocks_count);
    for (size_t block = 0; block < blocks_count; block++) {
      encoded_blocks[block] = ReadUint32(in);
      if (!in) break;
    }

    bool extract_this_file = input_files.empty();
    if (!extract_this_file) {
      for (size_t file_idx = 0; file_idx < input_files.size(); file_idx++) {
        if (input_files[file_idx] == filename) {
          extract_this_file = true;
          break;
        }
      }
    }

    if (extract_this_file) {
      std::vector<uint8_t> decoded_data;
      for (size_t decode_block_idx = 0; decode_block_idx < blocks_count; decode_block_idx++) {
        uint16_t block = HammingDecode(encoded_blocks[decode_block_idx]);
        decoded_data.push_back(block & kByteMask);
        if (decoded_data.size() < data_length) {
          decoded_data.push_back((block >> kBitsInByte) & kByteMask);
        }
      }

      if (decoded_data.size() != data_length) {
        decoded_data.resize(data_length);
      }
      
      std::string output_name = filename;
      std::filesystem::path outputPath(output_name);
      if (outputPath.has_parent_path()) {
        std::filesystem::create_directories(outputPath.parent_path());
      }
      std::ofstream out_file(output_name, std::ios::binary);
      out_file.write(reinterpret_cast<char*>(decoded_data.data()), decoded_data.size());
      out_file.close();
    }
  }
  in.close();
}

void Archive::AppendFile() {
  std::ofstream out(arch_name, std::ios::binary | std::ios::app);
  int files_amount = input_files.size();
  for (int file = 0; file < files_amount; file++) {
    std::string filename = input_files[file];
    std::ifstream input(filename, std::ios::binary);
    if (!input) {
      std::cout << "File not opened" << std::endl;
      continue;
    }

    uint16_t name_length = filename.size();
    WriteUint16(out, name_length);
    

    for (size_t i = 0; i < name_length; i += kBytesPerBlock) {
      uint16_t block = static_cast<uint8_t>(filename[i]);
      if (i + 1 < name_length) {
        block |= (static_cast<uint8_t>(filename[i + 1]) << kBitsInByte);
      }
      uint32_t encoded = HammingCode(block);
      WriteUint32(out, encoded);
    }

    input.seekg(0, std::ios::end);
    uint32_t data_length = input.tellg();
    input.seekg(0, std::ios::beg);
    WriteUint32(out, data_length);


    char buffer[kBytesPerBlock];
    while (input.read(buffer, kBytesPerBlock) || input.gcount() > 0) {
      uint16_t block = static_cast<uint8_t>(buffer[0]);
      if (input.gcount() == kBytesPerBlock) {
        block |= (static_cast<uint8_t>(buffer[1]) << kBitsInByte);
      }
      uint32_t encoded = HammingCode(block);
      WriteUint32(out, encoded);
    }
    input.close();
  }
  out.close();
}

void Archive::DeleteFile(const std::string& filename_to_delete) {
  std::ifstream in(arch_name, std::ios::binary);
  if (!in) {
    return;
  }
  std::ofstream out("temp.bin", std::ios::binary);
  if (!out) {
    return;
  }

  while (true) {
    uint16_t name_length = ReadUint16(in);
    if (!in) break;


    std::vector<uint8_t> decoded_name;
    size_t name_blocks = (name_length + 1) / kBytesPerBlock;
    for (size_t i = 0; i < name_blocks; i++) {
      uint32_t encoded = ReadUint32(in);
      if (!in) break;
      uint16_t block = HammingDecode(encoded);
      decoded_name.push_back(block & kByteMask);
      if (decoded_name.size() < name_length) {
        decoded_name.push_back((block >> kBitsInByte) & kByteMask);
      }
    }
    std::string filename(decoded_name.begin(), decoded_name.end());

    uint32_t data_length = ReadUint32(in);
    if (!in) break;

    size_t blocks_count = (data_length + 1) / kBytesPerBlock;
    std::vector<uint32_t> encoded_blocks(blocks_count);
    for (size_t i = 0; i < blocks_count; i++) {
      encoded_blocks[i] = ReadUint32(in);
      if (!in) break;
    }
    
    if (filename == filename_to_delete) {
      continue;
    }

    WriteUint16(out, name_length);
    for (size_t i = 0; i < name_blocks; i++) {
      uint16_t block = static_cast<uint8_t>(decoded_name[i * kBytesPerBlock]);
      if (i * kBytesPerBlock + 1 < name_length) {
        block |= (static_cast<uint8_t>(decoded_name[i * kBytesPerBlock + 1]) << kBitsInByte);
      }
      uint32_t encoded = HammingCode(block);
      WriteUint32(out, encoded);
    }
    WriteUint32(out, data_length);
    for (size_t i = 0; i < blocks_count; i++) {
      WriteUint32(out, encoded_blocks[i]);
    }
  }
  in.close();
  out.close();
  std::remove(arch_name);
  std::rename("temp.bin", arch_name);
}

void Archive::Merge() {
  if (input_files.empty()) {
    std::cout << "No archives to merge" << std::endl;
    return;
  }

  std::ofstream out("temp_merge.bin", std::ios::binary);
  if (!out) {
    std::cout << "Failed to create temp file" << std::endl;
    return;
  }

  int archives_count = input_files.size();
  for (int idx = 0; idx < archives_count; idx++) {
    std::string arch_to_merge = input_files[idx];
    std::ifstream in(arch_to_merge, std::ios::binary);
    if (!in) {
      std::cout << "Cannot open archive" << std::endl;
      continue;
    }
    
    char buffer[kMaxBufferSize];
    while (in.read(buffer, sizeof(buffer)) || in.gcount() > 0) {
      out.write(buffer, in.gcount());
    }
    in.close();
  }
  out.close();
  std::remove(arch_name);
  std::rename("temp_merge.bin", arch_name);
}

void Archive::ChangeAction(Actions new_action) {
  action = new_action;
}

Archive::Actions Archive::GetAction() const {
  return action;
}

void Archive::ChangeName(const char* name) {
  delete[] arch_name;
  int len = std::strlen(name);
  arch_name = new char[len + 1];
  std::strcpy(arch_name, name);
  arch_name[len] = '\0';
}

const char* Archive::GetName() const {
  return arch_name;
}

void Archive::AddInputFile(const std::string& filename) {
  input_files.push_back(filename);
}

const std::vector<std::string>& Archive::GetInputFiles() const {
  return input_files;
}