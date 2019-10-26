#include "InvertedIndex.h"

std::vector<InvertedIndex*> SearchEngine::splitWord(std::wstring &text)
{
    size_t pi = 0;
    int textId = _textId++;
    std::map<std::wstring, std::vector<int>> m;
    std::vector<InvertedIndex*> iv;
    for (size_t i = 0; i < text.size(); ++i)
    {
        switch(text[i])
        {
        case ' ':
        case ',':
        case '.':
        {
            if (pi != (i-1))
            {
                std::wstring str = text.substr(pi, i-pi);
                auto iter = m.find(str);
                if (iter == m.end())
                {
                    std::vector<int> v;
                    v.push_back(pi);
                    m[str] = std::move(v);
                }
                else
                {
                    std::vector<int> v = std::move(iter->second);
                    v.push_back(pi);
                    m[str] = std::move(v);
                }
            }

            pi = i;

            break;
        }
        default:
            break;
        }
    }

    for (auto v : m)
    {
        InvertedIndex *ii = new InvertedIndex(v.first, textId, std::move(v.second));
        iv.push_back(ii);
    }

    return iv;
}