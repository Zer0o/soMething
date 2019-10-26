#pragma once

#include <string>
#include <vector>

class FSTNode
{
public:
    FSTNode();
    FSTNode(wchar_t ch, int val) : _ch(ch), _val(val) {}
    ~FSTNode();

    wchar_t _ch;
    int _val;
    std::vector<FSTNode*> next;
};

class FST
{
public:
    FST()
    {
        _head = new FSTNode;
    }
    virtual ~FST();

    void add(std::wstring &text, int val);
    int search(std::wstring &text);

protected:
    FSTNode *_head;
};