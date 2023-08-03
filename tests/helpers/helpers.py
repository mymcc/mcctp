import os
import shutil
import subprocess

texture_packs = [
    "controllertexturepack",
    "emblemstexturepack",
    "globaluitexturepack",
    "hoppertexturepack",
    "ingamechapterpack",
    "largeavatartexturepack",
    "levelstexturepack",
    "loadingtexturepack",
    "mainmenuandcampaigntexturepack",
    "mainmenutexturepack",
    "medalstexturepack",
    "skullstexturepack",
    "spmappreviewtexturepack",
]

file_count_map = {
    "controllertexturepack": 47,
    "emblemstexturepack": 567,
    "globaluitexturepack": 304,
    "hoppertexturepack": 106,
    "ingamechapterpack": 494,
    "largeavatartexturepack": 236,
    "levelstexturepack": 536,
    "loadingtexturepack": 228,
    "mainmenuandcampaigntexturepack": 993,
    "mainmenutexturepack": 748,
    "medalstexturepack": 275,
    "skullstexturepack": 41,
    "spmappreviewtexturepack": 142,
    "none": 0
}


def count_total_expected_files(file_count_map: dict) -> int:
    sum: int = 0
    for (k, v) in file_count_map.items():
        sum += v
    return sum


file_count_map.update({"all": count_total_expected_files(file_count_map)})


def get_expected_file_count(texture_pack_flag: str) -> int:
    global file_count_map
    return file_count_map.get(texture_pack_flag)


def get_actual_file_count(root, texture_pack_flag: str) -> int:
    def get_texture_pack_file_count(root: str, texture_pack_flag: str):
        texture_pack_dump = root + f"/ui/texturepacks/{texture_pack_flag}"
        return len([f for f in os.listdir(texture_pack_dump) if os.path.isfile(os.path.join(texture_pack_dump, f))])

    if (texture_pack_flag != "all") and (texture_pack_flag != "none"):
        texture_pack_dump = root + f"/ui/texturepacks/{texture_pack_flag}"
        return get_texture_pack_file_count(root, texture_pack_flag)

    elif (texture_pack_flag == "all"):
        global file_count_map
        sum: int = 0
        for (k, v) in file_count_map.items():
            if (k != "all") and (k != "none"):
                sum += get_texture_pack_file_count(root, k)
        return sum

    else:
        return 0


def execute_script(script: str, *args) -> bool:
    try:
        subprocess.run([script] + list(args), check=True)
    except subprocess.CalledProcessError as e:
        print(
            f"The batch script returned a non-zero exit code: {e.returncode}")
        return False
    except Exception as e:
        print(f"An error occurred while trying to run the batch script: {e}")
        return False
    return True


def clean_artifacts(path: str) -> None:
    for texture_pack_flag in texture_packs:
        texture_pack_dump = path + f"/ui/texturepacks/{texture_pack_flag}"
        if os.path.exists(texture_pack_dump):
            shutil.rmtree(texture_pack_dump)
