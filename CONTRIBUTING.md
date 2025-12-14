# Setting up the project

To setup the project to begin development:

1. Clone the repository.

2. Navigate to the repository folder and then initialise the submodules using:
```bash
git submodule update --init --recursive
```

# Building and Testing

To build the project locally:

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure the project using CMake:
```bash
cmake ..
```
To enable parser generation (requires BNFC, Flex, and Bison), use:
```bash
cmake -DBNFC=ON ..
```

3. Build the project:
```bash
make
```

To run the tests, use the `VNNLibParser` executable against the VNNLIB test files:
```bash
for file in ../grammar/test/*.vnnlib; do
    ./bin/VNNLibParser check "$file"
done
```

# Updating the grammar

When changes are made to the [grammar](https://github.com/VNNLIB/VNNLIB-Standard/blob/main/grammar.cf) in the [VNN-LIB Standard](https://github.com/VNNLIB/VNNLIB-Standard), then the following procedure should be performed.

In order to build the grammar you must have the following tools installed:
- [BNFC](https://github.com/BNFC/bnfc) (v2.9.5)
- [Flex](https://github.com/westes/flex) (v2.6.4)
- [Bison](https://www.gnu.org/software/bison/) (3.8.2)
The versions above are known to work although others may work as well.

1. Get the required version of the grammar, by navigating into the submodule `grammar` directory, pulling and then checking out the required commit.
```bash
cd grammar
git pull
git checkout REF
cd ..
```
where `REF` is either a commit hash or a tag for the commit you want to update to.

2. Build the parser for the new grammar. 
It is recommended to use CMake to generate the parser files, as it handles patching the source files for cross-platform compatibility.
```bash
cmake -S . -B build -DBNFC=ON
cmake --build build
```

3. Add the new files and the new submodule commit, and then commit and push the changes.

4. Make the necessary changes to the core library files.


# Making a release

To make a release: 

1. Update `README.md` with a new entry to the compatibility table. 

2. Update `CHANGELOG.md` with the new version and changes.

3. Create a new Release on GitHub:
   - Tag the version (e.g., `v1.0.0`).
   - Provide a title and description.
   - Publish the release.

4. The "Build and Deploy" GitHub workflow will automatically trigger:
   - It builds the shared libraries for Linux (`libVNNLib.so`), Windows (`VNNLib.dll`), and macOS (`libVNNLib.dylib`).
   - It uploads these artifacts to the GitHub Release. 
