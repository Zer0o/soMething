#pragma once

#include <map>
#include <string>
#include <vector>

class InvertedIndex
{
public:
    InvertedIndex() : _textId(-1) {}
    InvertedIndex(const std::wstring &key, int sid, std::vector<int> indexs)
    : _key(key), _textId(sid)
    {
        _indexs = indexs;
    }
    virtual ~InvertedIndex() {}

protected:
    std::wstring _key;
    int _textId;
    std::vector<int> _indexs;
};

class SearchEngine
{
public:
    SearchEngine() {}
    virtual ~SearchEngine() {}

    int addText(std::wstring &text);
    void delText(int textId);

protected:
    std::vector<InvertedIndex*> splitWord(std::wstring &text);

protected:
    std::map<std::string, InvertedIndex *> _keyMap;
    std::map<int, std::wstring> _textMap;
    int _textId;
};