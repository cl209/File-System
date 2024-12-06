File System Hash Table Implementation
Description
A file system component implementation using hash tables, developed as Project 4 for CMSC 341. The system efficiently manages file storage and retrieval using various probing methods for collision resolution.
Core Features

Hash table-based file system
Multiple collision handling methods:

Linear probing
Quadratic probing
Double hashing


Dynamic table resizing
Incremental rehashing (25% per operation)
Lazy deletion support

Functions

File insertion
File removal
File search
Block number updates
Collision policy changes
Load factor management
Deleted ratio tracking

Technical Specifications

Table size: Prime numbers between MINPRIME and MAXPRIME
Rehashing triggers:

Load factor > 0.5
Deleted ratio > 80%


Files identified by name and block number
Block numbers must be within [DISKMIN-DISKMAX]

Project Files

filesys.h - Header file with class definitions
filesys.cpp - Implementation file
mytest.cpp - Test cases
