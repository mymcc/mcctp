-- external.lua
IncludeDir = {}
IncludeDir["mcctp"]  = "../include"
IncludeDir["imgui"]  = "../external/imgui"
IncludeDir["fpng"] = "../external/fpng/src/"

LibraryDir = {}

group "external"
   include "external/imgui"
   include "external/fpng"
group ""

group "core"
   include "src/"
   include "include/"
group ""