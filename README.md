# Search Engine

A simple from-scratch **Search Engine / Information Retrieval System** built using C++.

The project searches a collection of text documents and returns the most relevant documents for a user's query.

## Project Overview

A basic approach to searching would scan every document whenever a user enters a query.

This project instead builds an **Inverted Index** beforehand, allowing the system to directly identify documents containing the searched terms.

The retrieved documents are then ranked using **TF-IDF (Term Frequency-Inverse Document Frequency)**.

## Project Flow

Documents
   ↓
Text Preprocessing
   ↓
Tokenization & Normalization
   ↓
Inverted Index
   ↓
User Query
   ↓
Query Processing
   ↓
TF-IDF Scoring
   ↓
Ranking
   ↓
Top Relevant Documents

## Features

- Read and process a collection of text documents
- Convert text into tokens
- Normalize text such as converting words to lowercase
- Build an inverted index
- Search documents using keywords
- Calculate TF-IDF scores
- Rank documents based on relevance
- Return the most relevant documents
- Display document relevance scores

## Core Data Structures

### Inverted Index

The inverted index maps each term to the documents in which it occurs.

Example:

    machine → document1, document3
    learning → document1, document2, document3
    python → document2

Instead of checking every document for every query, the system can directly access the relevant documents.

### Hash Map

Hash maps are used for efficient storage and lookup of terms and document information.

### Priority Queue

A priority queue can be used to efficiently retrieve the highest-scoring documents when returning Top-K results.

## TF-IDF

TF-IDF is used to determine how important a word is to a document within the document collection.

### Term Frequency (TF)

Measures how frequently a term appears in a document.

### Inverse Document Frequency (IDF)

Gives lower importance to terms that appear in many documents and higher importance to terms that appear in fewer documents.

### TF-IDF

    TF-IDF = TF × IDF

Documents containing important query terms receive higher scores and are ranked higher.

## Example

Suppose the collection contains:

    Document 1:
    Machine learning is useful.

    Document 2:
    Deep learning uses neural networks.

    Document 3:
    Machine learning algorithms are powerful.

Query:

    machine learning

The system:

    Query
      ↓
    Tokenization
      ↓
    Inverted Index Lookup
      ↓
    Find Matching Documents
      ↓
    Calculate TF-IDF Scores
      ↓
    Rank Results
      ↓
    Display Relevant Documents

## Technologies

- C++
- Data Structures
- Algorithms
- File Handling
- Hash Maps
- Priority Queue
- String Processing
- TF-IDF

## Project Structure

    SearchEngine/
    │
    ├── main.cpp
    ├── documents/
    │   ├── doc1.txt
    │   ├── doc2.txt
    │   └── doc3.txt
    │
    └── README.md

## Basic Usage

1. Add text documents to the document collection.
2. Run the program.
3. Enter a search query.
4. The system processes the query.
5. Matching documents are retrieved using the inverted index.
6. Documents are ranked using TF-IDF.
7. The most relevant results are displayed.

## Future Improvements

Possible future extensions include:

- Phrase search
- Boolean queries such as AND, OR and NOT
- Autocomplete using a Trie
- Spell correction using Edit Distance
- Improved search-result snippets
- Performance testing on larger document collections

## Learning Outcomes

This project provides practical understanding of:

- Inverted indexing
- Information retrieval
- Text preprocessing
- Search algorithms
- TF-IDF ranking
- Hash-based data structures
- Top-K retrieval
- Time and memory trade-offs
