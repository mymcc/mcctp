import pytest
import os
from tests.helpers.helpers import execute_script
from tests.helpers.helpers import clean_artifacts


@pytest.fixture(scope="session", autouse=False)
def root():
    root: str = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    return root


@pytest.fixture(scope="session", autouse=True)
def setup(root):
    install7z_script: str = root + "/scripts/install7z.bat"
    execute_script(install7z_script)

    unzip_script: str = root + "/scripts/unzip_textures.bat"
    execute_script(unzip_script)

    yield

    clean_artifacts(root)

    clean_unzips: str = root + "/scripts/clean_bins.bat"
    execute_script(clean_unzips)
