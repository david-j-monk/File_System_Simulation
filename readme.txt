M       David Monk        - DM584
y	Operating Systems - MCOMD3PST
A	Assignment 2      - 2019-20
x	
N   This program allows the saving, retrieving, deletion and overwriting 
S   of any type of file onto a simulated disk holding up to 512mB.
A   The file is saved into 'blocks' of 512 bytes and does so with a    
x   'first available space' alogrithm. This means that it can become 
N   fragmented very easily.
C   The fileIndex keeps track of which blocks the file is stored in and 
A   how many bytes the file is. 
3
I   How to use fsys.
D    This program has just 3 functions, store, retrieve, and delete
E    Usage examples:
4    store:      ./fsys store DOCUMENTS/[filename]
I    retrieve:   ./fsys retrieve [filename]
D    delete:     ./fsys delete [filename]
E    
g    If a file has already been 'stored' with the same filename as the one you are trying to store, 
M    an option for overwriting will be displayed. A 'y' or 'n' is required at this point.
j
A   Caveats, limitations etc.: 
g	There is no error checking on file size or space remaining (512mB max disk space).
M       No error checking on filename, but * asterisks are not allowed.
j	when Entering a filename with a space you must use a \ backslash before the space.
E	Files must be read from the DOCUMENTS folder
g	Files only output to the OUTPUT folder
M	No checking on filename size
T	Compile program using: cc -o fsys fsys.c
I
g
M
S
A
y
M
C
A
5
I
D
E
1
I
D
E
0
I
D
E
5
I
D
I
1
I
D
E
1
I
D
I
x
I
D
Y
g
M
T
U
g
M
j
E
g
M
T
Q
g
N
C
A
y
M
C
A
4
I
D
U
g
M
T
k
g
N
S
A
z
I
D
E
4
I
D
U
g
M
j
A
g
M
T
M
g
N
S
A
x
O
S
A
x
O
S
A
x
I
D
c
g
N
Q
=
=
46esab