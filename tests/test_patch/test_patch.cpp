#include "mcctp.h"

#include "..\helpers\helpers.h"

int main(int argc, char** argv) {
  // Initialize mcctp
  
	const auto texture_pack_path = GetTexturePackPath();
	const std::vector<Patch> patches = ParsePatchesFromArgs(__argc, __argv);
    //const std::vector<Patch> patches = {{"ANGER", "BANDANA"}};
	mcctp::Initialize(texture_pack_path, mcctp::TexturePackFlags::All);
    //mcctp::Initialize(texture_pack_path, mcctp::TexturePackFlags::MainMenuAndCampaign);
    
	// Memory Map texture packs
	bool ok = mcctp::MemoryMapTexturePacks();
	if (!ok) ExitWithError("Error on MemoryMapTexturePacks()", -1);
    
	// Index texture packs
	ok = mcctp::IndexTexturePacks();
	if (!ok) ExitWithError("Error on IndexTexturePacks()", -1);
    
	//ok = PatchAndWriteBins(patches);
	//if (!ok) ExitWithError("Error on PatchAndWriteBins()", -1);

	auto *inst = mcctp::ctx::Instance();
	
	mcctp::PatchMap patch_map = ParsePatchMapFromPatches(patches);
    PrintPatchMap(patch_map);
    mcctp::Result result = inst->SetCurrentPatchMap(patch_map);
        std::cout << "Result: " << mcctp::ResultTypeToString.at(result.GetType()) 
			<< " Msg: " << result.what() << std::endl;

	inst->InjectCurrentPatchMapAndExport(mcctp::TexturePackInjectFlags::None);
	return 0;
}