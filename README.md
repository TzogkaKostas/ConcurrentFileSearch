# FileParallelSearching


The purpose of application is to execute queries in a binary log file and to sort the results.
The application creates new child processes. Process children compose a process hierarchy
which is in the form of a binary tree. This tree has internal nodes as well
leaf nodes.
The processes of the binary process tree perform the search for records in the file. Leaf nodes take over the search for records
in parts of the file, while the internal nodes compose the results they get from children
and promote it to their parents. The final results are sorted after the search process is completed.

# Execution

The application can be called in the following strict manner on the command line:

./myfind –h Height –d Datafile -p Pattern -s

where

• myfind is the executable(can be created by "makefile"),

• Datafile is the binary data entry file.

• Height is the depth of the full binary search tree to be created, with a maximum permitted depth value = 5.

The minimum depth allowed is 1 which means a node splitter-merger and two searcher guys searching each half file, depth = 2
means a splitter-merger node with two kids splitter-merger each having two kids searcher
who can search the 1/4 of the file each, etc.

• Pattern is the (sub-) string we look for in the (binary) data file (any field).

• The appearance of the -s flag indicates that searchers nodes search for parts of the file that have not
the same number of records.
The non-appearance of the -s flag indicates that all searchers take over parts of the Datafile that
have the same number of records.
All of the above flags can be displayed in any order.
