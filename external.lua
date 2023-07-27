-- external.lua
IncludeDir = {}
IncludeDir["imgui"]  = "../external/imgui"

LibrarayDir = {}

group "external"
   include "external/imgui"
group ""

group "core"
   include "src/"
   include "include/"
group ""