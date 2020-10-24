[![AzureDevOps CI](https://dev.azure.com/inaos/inac/_apis/build/status/inac-ci?branchName=develop)](https://dev.azure.com/inaos/inac/_build/latest?definitionId=9&branchName=develop) [![codecov](https://codecov.io/gh/inaos/inac/branch/master/graph/badge.svg?token=sEKLQQ2aeE)](https://codecov.io/gh/inaos/inac)

# INAOS Common C Library

The INAOS Common C Library is a collection of header files and library routines 
used to implement common operations, such as input/output, character string 
handling,  memory, error and event handling. This library is designed and 
optimized for singled threaded applications and is used as common base for 
all INAOS programs/libraries written in C.

High level objectives:

* Be minimal but complete (keep it simple)
* Low complexity
* Low resource consumption/High performance
* Ease of maintenance, testing, benchmarking, debugging
* Cross platform
* Fully documented

To learn more about INAOS Common C Library, please see the Manual.

## Releasing

### Creating a Hotfix

1. Create milestone (e.g. v1.0.2)
2. Create issues for bugs
3. git branch from master (e.g. v1.0.2)
4. Adjust version in CMakeLists.txt, increment patch level (e.g. 1.0.1 to 1.0.2)
5. Commit CMakeLists.txt changes before fixing bug(s)
6. Fix bug(s)
7. Create pull request
8. Run CI (automatic)
9. git merge master
10. Cherry-Pick all commits except CMake version changes to develop branch
11. Delete hotfix branch v1.0.2
12. Close bugs
13. Close milestone

### Creating a minor release

1. Create milestone (e.g. v1.1)
2. Create issues
3. Create feature branch from develop (e.g. WIP_123)
4. Merge feature branch back to develop
5. Delete feature branch
6. Create pull request
7. Run CI (automatic)
8. git merge master
9. Adjust CMakeLists.txt, increment minor version (e.g. 1.2.0)
