#include <filesystem>
#include <cstdlib>

#include "mcctp.h"

mcctp::TexturePackField ParseFlagsFromArgs(int argc, char** argv);
std::filesystem::path GetTexturePackPath(void);
void ExitWithError(const char* msg, int err);

int main(int argc, char** argv) {
	// Initialize mcctp
	const auto texture_pack_path = GetTexturePackPath();
	const mcctp::TexturePackField flag_field = ParseFlagsFromArgs(__argc, __argv);
	
	mcctp::Initialize(texture_pack_path);
	mcctp::ctx::Instance()->SetField(flag_field);

	// Memory Map texture packs
	bool ok = mcctp::MemoryMapTexturePacks();
	if (!ok) ExitWithError("Error on MemoryMapTexturePacks()", -1);

	// Index texture packs
	ok = mcctp::IndexTexturePacks();
	if (!ok) ExitWithError("Error on IndexTexturePacks()", -1);

	// Dump texture packs
	const auto dump_format_flags = mcctp::DumpFormatFlags::Native;
	const auto compression_flags = mcctp::DumpCompressionFlags::None;
	ok = mcctp::DumpTexturePacks();
	if (!ok) ExitWithError("Error on DumpTexturePacks()", -1);

	return 0;
}

mcctp::TexturePackField ParseFlagsFromArgs(int argc, char** argv)
{
	mcctp::TexturePackField field = mcctp::TexturePackFlags::None;

	std::cout << "argc: " << argc << std::endl;
	for (int i = 1; i < argc; ++i) {
		if (argv[i] != nullptr) {
			std::string arg(argv[i]);
			std::cout << arg << std::endl;
			if (mcctp::BasenameToFlag.find(arg) != mcctp::BasenameToFlag.end()) {
				auto flag = mcctp::BasenameToFlag.at(arg.c_str());
				field.SetFlag(flag);
			}
		}
	}
	if (argc > 1) field.UnsetFlag(mcctp::TexturePackFlags::None);
	return field;
}

std::filesystem::path GetTexturePackPath(void)
{
	static const auto path(std::filesystem::path(__FILE__).
		parent_path().parent_path().parent_path() / "ui/texturepacks");

	return path;
}

void ExitWithError(const char* msg, int err)
{
	std::cerr << msg << std::endl;
	std::exit(err);
}
