-- external.lua
IncludeDir = {}
IncludeDir["mcctp"]  = "../include"
IncludeDir["imgui"]  = "../external/imgui"

LibraryDir = {}

group "external"
   include "external/imgui"
group ""

group "core"
   include "src/"
   include "include/"
group ""