#pragma once

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <ostream>
#include <fstream>
#include <tuple>
#include <memory>

#include <codecvt>
#include <locale>

#include "GL/glew.h"
#include "GL/wglew.h"

#include "fpng.h"

namespace mcctp {
struct FileMapping {
private:
  HANDLE hFile = INVALID_HANDLE_VALUE;
  HANDLE hMap = NULL;
  LPVOID lpMap = NULL;

public:
  explicit FileMapping(std::wstring path);

  FileMapping(FileMapping&& other) noexcept;
  FileMapping& operator=(FileMapping&& other) noexcept;

  FileMapping(const FileMapping &) = delete;
  FileMapping &operator=(const FileMapping &) = delete;

  ~FileMapping();

  HANDLE GetFile(void) const { return hFile; }
  HANDLE GetFileMap(void) const { return hMap; }
  LPVOID GetMapView(void) const { return lpMap; }

private:
    bool TryCreateMapping(std::wstring path);
};

static constexpr uint8_t TexturePackCount = 13;

enum TexturePackFlags {
  Controller = 1 << 0,
  Emblems = 1 << 1,
  GlobalUI = 1 << 2,
  Hopper = 1 << 3,
  InGameChapter = 1 << 4,
  LargeAvatar = 1 << 5,
  Levels = 1 << 6,
  Loading = 1 << 7,
  MainMenuAndCampaign = 1 << 8,
  MainMenu = 1 << 9,
  Medals = 1 << 10,
  Skulls = 1 << 11,
  SPMapPreview = 1 << 12,
  All = (Controller | Emblems | GlobalUI | Hopper | InGameChapter | LargeAvatar | Levels | Loading |
         MainMenuAndCampaign | MainMenu | Medals | Skulls | SPMapPreview),
  None = 1 << 13,
};

static const std::unordered_map<TexturePackFlags, const char *> FlagToBasename{
    {TexturePackFlags::Controller, "controllertexturepack"},
    {TexturePackFlags::Emblems, "emblemstexturepack"},
    {TexturePackFlags::GlobalUI, "globaluitexturepack"},
    {TexturePackFlags::Hopper, "hoppertexturepack"},
    {TexturePackFlags::InGameChapter, "ingamechapterpack"},
    {TexturePackFlags::LargeAvatar, "largeavatartexturepack"},
    {TexturePackFlags::Levels, "levelstexturepack"},
    {TexturePackFlags::Loading, "loadingtexturepack"},
    {TexturePackFlags::MainMenuAndCampaign, "mainmenuandcampaigntexturepack"},
    {TexturePackFlags::MainMenu, "mainmenutexturepack"},
    {TexturePackFlags::Medals, "medalstexturepack"},
    {TexturePackFlags::Skulls, "skullstexturepack"},
    {TexturePackFlags::SPMapPreview, "spmappreviewtexturepack"},
};

enum class ResourceFormat {
    A8R8G8B8,
    DXT1,
    DXT3,
    DXT5,
    INVALID,
};

struct TexturePackBlock {
  size_t Key;
  size_t Size;
  size_t Offset;

  friend std::ostream& operator<<(std::ostream& os, const TexturePackBlock& block);
};

/* PERM == Resource data is in .perm.bin | TEMP == Resource data is in .temp.bin */
enum class TexturePackType { 
    PERM, 
    TEMP, 
    INVALID 
};

struct TexturePackResource {
  std::string Name;
  char * Origin;
  size_t Offset;
  size_t Size;
  int32_t Width;
  int32_t Height;
  ResourceFormat Format;
  TexturePackType PackType;

  friend std::ostream& operator<<(std::ostream& os, const TexturePackResource& res);
};

typedef std::tuple<FileMapping, FileMapping, TexturePackType> TexturePack;
static const FileMapping &GetPerm(const TexturePack &tp) { return std::get<0>(tp); }
static const FileMapping &GetTemp(const TexturePack &tp) { return std::get<1>(tp); }
static   TexturePackType  GetType(const TexturePack &tp) { return std::get<2>(tp); }

typedef std::variant<TexturePackResource, TexturePackBlock> TexturePackEntry;

typedef std::unordered_map<std::string, TexturePackEntry> TexturePackIndex;

struct TexturePackField {
private:
  uint16_t Value;

public:
  TexturePackField(uint16_t value = TexturePackFlags::All) : Value(value) {}
  void SetFlag(TexturePackFlags flags) { Value |= (int)flags; }
  void UnsetFlag(TexturePackFlags flags) { Value &= ~(int)flags; }
  void FlipFlag(TexturePackFlags flags) { Value ^= (int)flags; }
  bool HasFlag(TexturePackFlags flags) { return (Value & (int)flags) == (int)flags; }
  bool HasAnyFlag(TexturePackFlags flags) { return (Value & (int)flags) != 0; }
  operator uint16_t() const { return Value; }
  uint16_t to_uint16() const { return Value; }
};

enum class DumpFormatFlags {
    Native,
    EncodeToPNG
};

enum class DumpCompressionFlags {
    Fastest,
    Smallest,
    None,
};

class ctx {
public:
  static ctx* Instance();
  
  void SetField(uint16_t field = (uint16_t)TexturePackFlags::All) {
      m_Field = (TexturePackField)field;
  }
  TexturePackField GetField(void) const { return m_Field; }
  void SetPathPrefix(std::filesystem::path path_prefix) { m_PathPrefix = path_prefix; }

  bool MemoryMapTexturePack(TexturePackFlags flag, std::wstring texture_pack_basename);
  void UnmapTexturePack(TexturePackFlags flag);
  size_t GetMappedTexturePackCount(void) const { return m_TexturePackFileMap.size(); }
  bool IsTexturePackMapped(TexturePackFlags flag) const;

  bool _IndexTexturePack(TexturePackFlags flag);
  bool IndexTexturePack(TexturePackFlags flag);
  size_t GetIndexedTexturePackCount(void) const { return m_TexturePackIndexMap.size(); }
  bool IsTexturePackIndexed(TexturePackFlags flag) const; 

  bool DumpTexturePack(TexturePackFlags flag, DumpFormatFlags format_flag = DumpFormatFlags::Native,
                       DumpCompressionFlags compression_flag = DumpCompressionFlags::None);
  bool DeleteTexturePackDump(TexturePackFlags flag);

  std::vector<TexturePackResource> GetResourcesFromTexturePack(TexturePackFlags flag) const;
  GLuint RenderDDSToTexture(TexturePackResource res);
  bool ShareWithWGLContext(HGLRC hrc = wglGetCurrentContext());

private:
  void InitializeGLContext(void);

private:
  TexturePackField m_Field;
  std::filesystem::path m_PathPrefix;
  std::unordered_map<TexturePackFlags, TexturePack> m_TexturePackFileMap;
  std::unordered_map<TexturePackFlags, TexturePackIndex> m_TexturePackIndexMap;

  HDC m_hDC;
  HGLRC m_hRC;
  bool m_ClientHasWGLContext;
  GLuint m_OutputVertexShader;
  GLuint m_OutputFragmentShader;
  GLuint m_OutputProgram;
};

void Initialize(std::filesystem::path path, TexturePackFlags flags = TexturePackFlags::All);

bool MemoryMapTexturePacks(void);
void UnmapTexturePacks(void);

bool IndexTexturePacks(void);
bool MemoryMapAndIndexTexturePacks(void);

bool DumpTexturePacks(DumpFormatFlags format_flag = DumpFormatFlags::Native,
                      DumpCompressionFlags compression_flag = DumpCompressionFlags::None);

void ClearTexturePackDumps(void);

std::stringstream BuildDDSHeaderForResource(TexturePackResource res);
GLuint RenderDDSToTexture(TexturePackResource res);
bool ShareWithWGLContext(HGLRC hrc = wglGetCurrentContext());
} // namespace mcctp