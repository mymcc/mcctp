import pytest

from tests.helpers.helpers import execute_script
from tests.helpers.helpers import get_actual_file_count
from tests.helpers.helpers import get_expected_file_count


def run_test_native(root: str, texture_pack_flag: str) -> bool:
    native_exe: str = root + "/tests/bin/Release-windows-x86_64/test_native/test_native.exe"
    return execute_script(native_exe, texture_pack_flag)


@pytest.mark.parametrize("texture_pack_flag",
                         [
                             ("none"),
                             ("controllertexturepack"),
                             ("emblemstexturepack"),
                             ("globaluitexturepack"),
                             ("hoppertexturepack"),
                             ("ingamechapterpack"),
                             ("largeavatartexturepack"),
                             ("levelstexturepack"),
                             ("loadingtexturepack"),
                             ("mainmenuandcampaigntexturepack"),
                             ("mainmenutexturepack"),
                             ("medalstexturepack"),
                             ("skullstexturepack"),
                             ("spmappreviewtexturepack"),
                             ("all"),
                         ]
                         )
def test_native_output(root, texture_pack_flag):
    assert (run_test_native(root, texture_pack_flag))
    assert get_expected_file_count(
        texture_pack_flag) == get_actual_file_count(root, texture_pack_flag)
