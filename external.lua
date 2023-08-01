-- external.lua
IncludeDir = {}
IncludeDir["mcctp"]  = "../include"
IncludeDir["imgui"]  = "../external/imgui"
IncludeDir["fpng"] = "../external/fpng/src"
IncludeDir["GLEW"] = "../external/glew/include"

LibraryDir = {}

group "external"
   include "external/imgui"
   include "external/fpng"
   include "external/GLEW"
group ""

group "core"
   include "src/"
   include "include/"
group ""