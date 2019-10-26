#pragma once

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <vector>

class TrieNode
{
public:
    TrieNode() : fail(NULL), _end(false) {}
    #if 1
    TrieNode& operator = (TrieNode &node)
    {
        for (auto iter : next)
        {
            delete iter.second;
        }

        next.clear();

        for (auto iter : node.next)
        {
            TrieNode *n = new TrieNode();
            *n = *(iter.second);
            next[iter.first] = n;
        }

        fail = NULL;
        _ch = node._ch;
        _end = node._end;
        _word = node._word;

        return *this;
    }
    #endif

    TrieNode(wchar_t &ch, bool f = false) : fail(NULL), _ch(ch), _end(f) {}
    ~TrieNode()
    {
        for (auto iter : next)
        {
            delete iter.second;
        }
    }

    bool hasChild(wchar_t &ch) const;
    TrieNode *getChild(wchar_t &ch);
    void addChild(wchar_t &ch, TrieNode *);

    void setEnd(bool b = true) { _end = b; }
    bool end() const { return _end; }

    void setWord(std::wstring &word) { _word = word; }
    std::wstring getWord() const { return _word; }

    std::map<wchar_t, TrieNode*> next;
    TrieNode *fail;
    wchar_t _ch;
    bool _end;
    std::wstring _word;
};

class Automation
{
public:
    Automation() : using_index(0) {}
    ~Automation();
    
    void addWord(std::wstring &word);
    void delWord(std::wstring &word);
    void buildFail(TrieNode *head);
    void print();

    int getOldIndex()
    {
        return (using_index+1) & 0x1;
    }

    int getUsingIndex() { return using_index & 0x1; }

    virtual std::vector<std::wstring> ac(std::wstring &sentence);

protected:
    
    struct UsingHead
    {
        UsingHead() : _count(0)
        {
            if (!head) head = new TrieNode;
        } 

        TrieNode *head;
        std::atomic_int _count;
    };

    UsingHead _h[2];
    int using_index;
    std::mutex _mutex;
    std::condition_variable cv;
};