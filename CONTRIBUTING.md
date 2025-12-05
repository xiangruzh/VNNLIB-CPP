# Setting up the project

To setup the project to begin development:

1. Clone the repository.

2. Navigate to the repository folder and then initialise the submodules using:
```bash
git submodule init
git submodule update
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

3. Build the parser for the new grammar. 
```bash
bnfc grammar/grammar.cf --cpp -o src/generated/
```

3. Add the new files and the new submodule commit, and then commit and push the changes.

4. Make the necessary changes to the core library files.


# Making a release

To make a release: 

1. Update `README.md` with new entry to compatability table. 
