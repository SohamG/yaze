#include "ROM.h"

namespace yaze {
namespace Application {
namespace Utils {

void ROM::LoadFromFile(const std::string& path) {
  std::cout << "filename: " << path << std::endl;
  std::ifstream stream(path, std::ios::in | std::ios::binary);

  if (!stream.good()) {
    std::cout << "failure reading file" << std::endl;
    return;
  }

  std::vector<char> contents((std::istreambuf_iterator<char>(stream)),
                             std::istreambuf_iterator<char>());

  for (auto i : contents) {
    int value = i;
    std::cout << "working_rom_: " << value << std::endl;
  }

  std::cout << "file size: " << contents.size() << std::endl;
}

int ROM::SnesToPc(int addr) {
  if (addr >= 0x808000) {
    addr -= 0x808000;
  }
  int temp = (addr & 0x7FFF) + ((addr / 2) & 0xFF8000);
  return (temp + 0x0);
}

void ROM::Write(int addr, byte value) { working_rom_[addr] = value; }

void ROM::WriteLong(int addr, int value) {
  working_rom_[addr] = (byte)(value & 0xFF);
  working_rom_[addr + 1] = (byte)((value >> 8) & 0xFF);
  working_rom_[addr + 2] = (byte)((value >> 16) & 0xFF);
}

void ROM::WriteShort(int addr, int value) {
  working_rom_[addr] = (byte)(value & 0xFF);
  working_rom_[addr + 1] = (byte)((value >> 8) & 0xFF);
}

int ROM::ReadLong(int addr) {
  return ((working_rom_[addr + 2] << 16) + (working_rom_[addr + 1] << 8) +
          working_rom_[addr]);
}

Tile16 ROM::ReadTile16(int addr) {
  ushort t1 = (ushort)((working_rom_[addr + 1] << 8) + working_rom_[addr]);
  ushort t2 = (ushort)((working_rom_[addr + 3] << 8) + working_rom_[addr + 2]);
  ushort t3 = (ushort)((working_rom_[addr + 5] << 8) + working_rom_[addr + 4]);
  ushort t4 = (ushort)((working_rom_[addr + 7] << 8) + working_rom_[addr + 6]);
  return Tile16((unsigned long)((t1 << 48) + (t2 << 32) + (t3 << 16) + t4));
}

ushort ROM::ReadShort(int addr) {
  return (ushort)((working_rom_[addr + 1] << 8) + working_rom_[addr]);
}

short ROM::ReadRealShort(int addr) {
  return (short)((working_rom_[addr + 1] << 8) + working_rom_[addr]);
}

ushort ROM::ReadByte(int addr) { return (ushort)(working_rom_[addr]); }

short ROM::ReadReverseShort(int addr) {
  return (short)((working_rom_[addr] << 8) + working_rom_[addr + 1]);
}

}  // namespace Utils
}  // namespace Application
}  // namespace yaze