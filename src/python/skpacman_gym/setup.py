from setuptools import setup,find_packages

setup(
    name="skpacman_gym",
    version="0.0.1",
    install_requires=["gymnasium==0.29.1"],
    package_data={'skpacman_gym': ['envs/lib/**']},
    packages=find_packages(),
    include_package_data=True,
)
