# Facility Arrangement Solver

## Brief

A tool to arrange *subjects* (such as departments, machines, etc.) within the given
facility layout. Subjects can be categorised into *groups* or *types*. Subjects within
one type share common properties. Facility workflow is modelled via interactions between
different types of subjects. For more delails, see the manual *(will be available as soon
as time allows)*.

> The tool is in the early stage of development.

## External dependencies

This tool relies on the following external packages:

* [CPLEX](https://www.ibm.com/products/ilog-cplex-optimization-studio)
	* Proprietary software by IBM.
	* Not supplied together with the source code in this repository, acquisition of
	licensed copy on your own is required.

## Compilation

Unfortunately, CPLEX requires the use of
[bloody MSVC](https://community.ibm.com/community/user/datascience/communities/community-home/digestviewer/viewthread?MessageKey=efd94ad6-215c-42f2-9491-2cb9f45a93da&CommunityKey=ab7de0fd-6f43-47a9-8261-33578a231bb7)
to properly compile on Windows which might cause additional troubles to you. Install
[MSVC build tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
if you don't want to install Visual Studio.

On Linux, the use of G++ is sufficient.

To get more details about the compilation, navigate to `/tools` folder.
