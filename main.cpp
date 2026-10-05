#include <bits/stdc++.h>
using namespace std;

struct Document
{
    int id;
    string filename;
    string content;
    vector<string> tokens;
};

struct TrieNode
{
    map<char, TrieNode *> child;
    bool isEnd;

    TrieNode()
    {
        isEnd = false;
    }
};

class SearchEngine
{
private:
    map<int, Document> docs;

    set<string> stopWords;
    set<string> vocabulary;

    unordered_map<string, unordered_map<int, int>> invertedIndex;
    unordered_map<string, unordered_map<int, vector<int>>> positions;
    map<string, int> documentFrequency;

    TrieNode *trieRoot;

public:
    SearchEngine()
    {
        trieRoot = new TrieNode();

        stopWords = {
            "the", "is", "a", "an", "of", "to", "in",
            "and", "for", "on", "with", "but", "it", "as"};
    }

    ~SearchEngine()
    {
        deleteTrie(trieRoot);
    }

    void deleteTrie(TrieNode *node)
    {
        if (node == nullptr)
            return;

        for (auto &p : node->child)
            deleteTrie(p.second);

        delete node;
    }

    string cleanWord(string word)
    {
        string result;

        for (unsigned char c : word)
        {
            if (isalnum(c))
                result += (char)tolower(c);
        }

        return result;
    }

    vector<string> tokenize(string text)
    {
        vector<string> tokens;
        string currentWord;

        for (unsigned char c : text)
        {
            if (isalnum(c))
            {
                currentWord += (char)tolower(c);
            }
            else
            {
                if (!currentWord.empty())
                {
                    if (stopWords.find(currentWord) == stopWords.end())
                        tokens.push_back(currentWord);

                    currentWord.clear();
                }
            }
        }

        if (!currentWord.empty() &&
            stopWords.find(currentWord) == stopWords.end())
        {
            tokens.push_back(currentWord);
        }

        return tokens;
    }

    void insertTrie(string word)
    {
        TrieNode *current = trieRoot;

        for (char c : word)
        {
            if (current->child.find(c) == current->child.end())
                current->child[c] = new TrieNode();

            current = current->child[c];
        }

        current->isEnd = true;
    }

    void loadDocuments(string folderPath)
    {
        int docId = 1;

        while (true)
        {
            string filename = "doc" + to_string(docId) + ".txt";
            string filepath = folderPath + "/" + filename;

            ifstream file(filepath);

            if (!file.is_open())
                break;

            stringstream buffer;
            buffer << file.rdbuf();

            string content = buffer.str();

            Document d;
            d.id = docId;
            d.filename = filename;
            d.content = content;
            d.tokens = tokenize(content);

            docs[docId] = d;

            docId++;
        }

        if (docs.empty())
        {
            cout << "Error: Could not find any documents in '"
                 << folderPath << "' folder!\n";

            cout << "Make sure the files are named:\n";
            cout << "doc1.txt, doc2.txt, doc3.txt, etc.\n";

            return;
        }

        buildIndex();
    }

    void buildIndex()
    {
        for (auto &p : docs)
        {
            int id = p.first;
            Document &d = p.second;

            for (int pos = 0; pos < (int)d.tokens.size(); pos++)
            {
                string word = d.tokens[pos];

                invertedIndex[word][id]++;
                positions[word][id].push_back(pos);
                vocabulary.insert(word);
            }
        }

        for (const string &word : vocabulary)
        {
            documentFrequency[word] =
                (int)invertedIndex[word].size();

            insertTrie(word);
        }

        cout << "Loaded " << docs.size()
             << " documents successfully.\n";

        cout << "Vocabulary size: "
             << vocabulary.size() << " words.\n";
    }

    string getSnippet(int docId, vector<string> queryTokens)
    {
        string content = docs[docId].content;

        if (content.empty())
            return "...";

        string lowerContent;

        for (unsigned char c : content)
            lowerContent += (char)tolower(c);

        size_t bestPos = string::npos;

        for (string word : queryTokens)
        {
            size_t pos = lowerContent.find(word);

            if (pos != string::npos)
            {
                bestPos = pos;
                break;
            }
        }

        if (bestPos == string::npos)
        {
            string snippet =
                content.substr(0, min(80, (int)content.size()));

            replace(snippet.begin(), snippet.end(), '\n', ' ');

            return snippet + "...";
        }

        int start = max(0, (int)bestPos - 30);
        int length =
            min(80, (int)content.size() - start);

        string snippet = content.substr(start, length);

        replace(snippet.begin(), snippet.end(), '\n', ' ');
        replace(snippet.begin(), snippet.end(), '\r', ' ');

        return "..." + snippet + "...";
    }

    void printTopK(
        map<int, double> &docScores,
        int K,
        vector<string> &queryTokens)
    {
        if (docScores.empty())
        {
            cout << "No matching documents found.\n";
            return;
        }

        if (K <= 0)
            K = 5;

        priority_queue<
            pair<double, int>,
            vector<pair<double, int>>,
            greater<pair<double, int>>>
            pq;

        for (auto &p : docScores)
        {
            pq.push({p.second, p.first});

            if ((int)pq.size() > K)
                pq.pop();
        }

        vector<pair<double, int>> results;

        while (!pq.empty())
        {
            results.push_back(pq.top());
            pq.pop();
        }

        reverse(results.begin(), results.end());

        cout << "\nSearch Results:\n\n";

        int rank = 1;

        for (auto &res : results)
        {
            int docId = res.second;
            double score = res.first;

            cout << rank << ". "
                 << docs[docId].filename
                 << "    Score: "
                 << fixed << setprecision(4)
                 << score << "\n";

            cout << "   \""
                 << getSnippet(docId, queryTokens)
                 << "\"\n\n";

            rank++;
        }
    }

    void normalSearch(string query, int K)
    {
        auto start = chrono::high_resolution_clock::now();

        vector<string> queryTokens = tokenize(query);

        if (queryTokens.empty())
        {
            cout << "Empty or invalid query.\n";
            return;
        }

        map<int, double> docScores;

        int totalDocs = docs.size();

        for (string word : queryTokens)
        {
            auto it = invertedIndex.find(word);

            if (it == invertedIndex.end())
                continue;

            int df = documentFrequency[word];

            if (df == 0)
                continue;

            double idf =
                log10((double)totalDocs / df);

            for (auto &docFreq : it->second)
            {
                int docId = docFreq.first;
                int tf = docFreq.second;

                docScores[docId] += tf * idf;
            }
        }

        printTopK(docScores, K, queryTokens);

        auto end = chrono::high_resolution_clock::now();

        double timeTaken =
            chrono::duration_cast<chrono::microseconds>(
                end - start)
                .count() /
            1000.0;

        cout << "Search time: "
             << timeTaken << " ms\n";
    }

    void phraseSearch(string phrase, int K)
    {
        auto start = chrono::high_resolution_clock::now();

        vector<string> queryTokens = tokenize(phrase);

        if (queryTokens.empty())
        {
            cout << "Empty or invalid phrase.\n";
            return;
        }

        if (queryTokens.size() < 2)
        {
            cout << "Phrase contains only one searchable word.\n";
            normalSearch(phrase, K);
            return;
        }

        map<int, double> docScores;

        int totalDocs = docs.size();

        string firstWord = queryTokens[0];

        auto firstIt = positions.find(firstWord);

        if (firstIt == positions.end())
        {
            cout << "No matching documents found.\n";
            return;
        }

        for (auto &docPos : firstIt->second)
        {
            int docId = docPos.first;

            vector<int> startPositions =
                docPos.second;

            int exactMatches = 0;

            for (int pos : startPositions)
            {
                bool match = true;

                for (int i = 1;
                     i < (int)queryTokens.size();
                     i++)
                {
                    string nextWord = queryTokens[i];

                    auto wordIt =
                        positions.find(nextWord);

                    if (wordIt == positions.end())
                    {
                        match = false;
                        break;
                    }

                    auto docIt =
                        wordIt->second.find(docId);

                    if (docIt == wordIt->second.end())
                    {
                        match = false;
                        break;
                    }

                    vector<int> &nextPositions =
                        docIt->second;

                    if (find(
                            nextPositions.begin(),
                            nextPositions.end(),
                            pos + i) == nextPositions.end())
                    {
                        match = false;
                        break;
                    }
                }

                if (match)
                    exactMatches++;
            }

            if (exactMatches > 0)
            {
                double score = 0;

                for (string word : queryTokens)
                {
                    int df = documentFrequency[word];

                    if (df == 0)
                        continue;

                    double idf =
                        log10((double)totalDocs / df);

                    score += exactMatches * idf;
                }

                docScores[docId] = score;
            }
        }

        printTopK(docScores, K, queryTokens);

        auto end = chrono::high_resolution_clock::now();

        double timeTaken =
            chrono::duration_cast<chrono::microseconds>(
                end - start)
                .count() /
            1000.0;

        cout << "Search time: "
             << timeTaken << " ms\n";
    }

    void booleanSearch(
        string word1,
        string op,
        string word2,
        int K)
    {
        auto start = chrono::high_resolution_clock::now();

        string w1 = cleanWord(word1);
        string w2 = cleanWord(word2);

        for (char &c : op)
            c = toupper((unsigned char)c);

        if (w1.empty() || w2.empty())
        {
            cout << "Invalid search terms.\n";
            return;
        }

        set<int> docs1;
        set<int> docs2;

        auto it1 = invertedIndex.find(w1);

        if (it1 != invertedIndex.end())
        {
            for (auto &p : it1->second)
                docs1.insert(p.first);
        }

        auto it2 = invertedIndex.find(w2);

        if (it2 != invertedIndex.end())
        {
            for (auto &p : it2->second)
                docs2.insert(p.first);
        }

        vector<int> resultSet;

        if (op == "AND")
        {
            set_intersection(
                docs1.begin(),
                docs1.end(),
                docs2.begin(),
                docs2.end(),
                back_inserter(resultSet));
        }
        else if (op == "OR")
        {
            set_union(
                docs1.begin(),
                docs1.end(),
                docs2.begin(),
                docs2.end(),
                back_inserter(resultSet));
        }
        else if (op == "NOT")
        {
            set_difference(
                docs1.begin(),
                docs1.end(),
                docs2.begin(),
                docs2.end(),
                back_inserter(resultSet));
        }
        else
        {
            cout << "Invalid operator. "
                 << "Use AND, OR, NOT.\n";
            return;
        }

        map<int, double> docScores;

        int totalDocs = docs.size();

        for (int docId : resultSet)
        {
            double score = 0;

            if (op == "NOT")
            {
                int df = documentFrequency[w1];

                if (df > 0)
                {
                    double idf =
                        log10((double)totalDocs / df);

                    score =
                        invertedIndex[w1][docId] * idf;
                }
            }
            else
            {
                if (invertedIndex[w1].count(docId))
                {
                    int df = documentFrequency[w1];

                    if (df > 0)
                    {
                        double idf =
                            log10((double)totalDocs / df);

                        score +=
                            invertedIndex[w1][docId] * idf;
                    }
                }

                if (invertedIndex[w2].count(docId))
                {
                    int df = documentFrequency[w2];

                    if (df > 0)
                    {
                        double idf =
                            log10((double)totalDocs / df);

                        score +=
                            invertedIndex[w2][docId] * idf;
                    }
                }
            }

            docScores[docId] = score;
        }

        vector<string> queryTokens = {w1, w2};

        printTopK(docScores, K, queryTokens);

        auto end = chrono::high_resolution_clock::now();

        double timeTaken =
            chrono::duration_cast<chrono::microseconds>(
                end - start)
                .count() /
            1000.0;

        cout << "Search time: "
             << timeTaken << " ms\n";
    }

    void autocompleteHelper(
        TrieNode *node,
        string currentPrefix,
        vector<string> &results)
    {
        if (results.size() >= 5)
            return;

        if (node->isEnd)
            results.push_back(currentPrefix);

        for (auto &p : node->child)
        {
            if (results.size() >= 5)
                break;

            autocompleteHelper(
                p.second,
                currentPrefix + p.first,
                results);
        }
    }

    void autocomplete(string prefix)
    {
        string cleanPrefix = cleanWord(prefix);

        if (cleanPrefix.empty())
        {
            cout << "Invalid prefix.\n";
            return;
        }

        TrieNode *current = trieRoot;

        for (char c : cleanPrefix)
        {
            auto it = current->child.find(c);

            if (it == current->child.end())
            {
                cout << "No suggestions found.\n";
                return;
            }

            current = it->second;
        }

        vector<string> results;

        autocompleteHelper(
            current,
            cleanPrefix,
            results);

        if (results.empty())
        {
            cout << "No suggestions found.\n";
            return;
        }

        cout << "\nSuggestions:\n";

        for (int i = 0; i < (int)results.size(); i++)
            cout << i + 1 << ". "
                 << results[i] << "\n";
    }

    int calculateEditDistance(
        string w1,
        string w2)
    {
        int m = w1.length();
        int n = w2.length();

        vector<vector<int>> dp(
            m + 1,
            vector<int>(n + 1));

        for (int i = 0; i <= m; i++)
            dp[i][0] = i;

        for (int j = 0; j <= n; j++)
            dp[0][j] = j;

        for (int i = 1; i <= m; i++)
        {
            for (int j = 1; j <= n; j++)
            {
                if (w1[i - 1] == w2[j - 1])
                {
                    dp[i][j] =
                        dp[i - 1][j - 1];
                }
                else
                {
                    dp[i][j] =
                        1 + min({dp[i][j - 1],
                                 dp[i - 1][j],
                                 dp[i - 1][j - 1]});
                }
            }
        }

        return dp[m][n];
    }

    void spellCorrect(string word)
    {
        string cleanWordValue =
            cleanWord(word);

        if (cleanWordValue.empty())
        {
            cout << "Invalid word.\n";
            return;
        }

        if (vocabulary.find(cleanWordValue) != vocabulary.end())
        {
            cout << "Word '"
                 << cleanWordValue
                 << "' is correctly spelled "
                 << "and exists in the vocabulary.\n";

            return;
        }

        string bestWord;
        int minDistance = 3;

        for (const string &vocabWord : vocabulary)
        {
            int dist =
                calculateEditDistance(
                    cleanWordValue,
                    vocabWord);

            if (dist < minDistance)
            {
                minDistance = dist;
                bestWord = vocabWord;
            }
        }

        if (!bestWord.empty())
        {
            cout << "\nDid you mean: "
                 << bestWord
                 << " ?\n";
        }
        else
        {
            cout << "\nNo spell correction found for '"
                 << cleanWordValue
                 << "'.\n";
        }
    }
};

int main()
{
    SearchEngine engine;

    cout << "Initializing Search Engine...\n";

    engine.loadDocuments("documents");

    while (true)
    {
        cout << "\n========================================\n";
        cout << "           C++ SEARCH ENGINE            \n";
        cout << "========================================\n";
        cout << "1. Normal Search\n";
        cout << "2. Phrase Search (Exact Match)\n";
        cout << "3. Boolean Search\n";
        cout << "4. Autocomplete\n";
        cout << "5. Spell Correction\n";
        cout << "6. Exit\n";
        cout << "Enter choice: ";

        int choice;

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(
                numeric_limits<streamsize>::max(),
                '\n');

            cout << "Invalid choice.\n";
            continue;
        }

        if (choice == 6)
        {
            cout << "Exiting...\n";
            break;
        }

        if (choice == 1)
        {
            cin.ignore(
                numeric_limits<streamsize>::max(),
                '\n');

            cout << "Enter query: ";

            string query;
            getline(cin, query);

            cout << "Enter K (number of results): ";

            int k;
            cin >> k;

            engine.normalSearch(query, k);
        }
        else if (choice == 2)
        {
            cin.ignore(
                numeric_limits<streamsize>::max(),
                '\n');

            cout << "Enter exact phrase: ";

            string phrase;
            getline(cin, phrase);

            cout << "Enter K: ";

            int k;
            cin >> k;

            engine.phraseSearch(phrase, k);
        }
        else if (choice == 3)
        {
            cout << "Boolean syntax:\n";
            cout << "word1 AND word2\n";
            cout << "word1 OR word2\n";
            cout << "word1 NOT word2\n";

            cout << "Enter boolean query: ";

            string w1, op, w2;

            cin >> w1 >> op >> w2;

            cout << "Enter K: ";

            int k;
            cin >> k;

            engine.booleanSearch(
                w1,
                op,
                w2,
                k);
        }
        else if (choice == 4)
        {
            cout << "Enter prefix: ";

            string prefix;
            cin >> prefix;

            engine.autocomplete(prefix);
        }
        else if (choice == 5)
        {
            cout << "Enter word: ";

            string word;
            cin >> word;

            engine.spellCorrect(word);
        }
        else
        {
            cout << "Invalid choice.\n";
        }
    }

    return 0;
}